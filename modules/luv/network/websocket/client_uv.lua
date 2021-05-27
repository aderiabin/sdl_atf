local uv = require('luv')
local tools = require('luv.network.websocket.tools')
local handshake = require('luv.network.websocket.handshake')
local frame = require('luv.network.websocket.frame')

local WsClient = {}

local ws_client_mt = { __index = {} }

function ws_client_mt.__index:open(url)
  self.protocol, self.host, self.port, self.uri = tools.parse_url(url)
  -- print("url: " .. tostring(url))
  -- print("protocol: " .. tostring(self.protocol))
  -- print("host: " .. tostring(self.host))
  -- print("port: " .. tostring(self.port))
  -- print("uri: " .. tostring(self.uri))

  self.ws_protocols_tbl = { "echo" }

  self.key = tools.generate_key()
  print("key: " .. self.key)

  self.tcp:connect(self.host, self.port, self.on_connect)
end

function ws_client_mt.__index:close(code,reason,timeout)
  if self.isConnected then
    local encoded = frame.encode_close(code or 1000,reason)
    encoded = frame.encode(encoded,frame.opcodes.CLOSE,true)
    self.tcp:write(encoded)
    timeout = timeout or 100
    local timer = uv.new_timer()
    timer:start(timeout, 0, function ()
      timer:close()
      self.tcp:close()
      print("Connection has been closed")
      self.isConnected = false
    end)
    return
  end
  print("Connection is not established, nothing to close")
end

local function sendMessage(ws, message, opcode)
  print("Send: " .. message)
  if not ws.isConnected then
    print("Message can not be sent. Connection is not established")
    return
  end
  local encoded = frame.encode(message, opcode, true)
  ws.tcp:write(encoded)
end

function ws_client_mt.__index:sendTextMessage(message)
  sendMessage(self, message, frame.opcodes.TEXT)
end

function ws_client_mt.__index:sendBinaryMessage(message)
  sendMessage(self, message, frame.opcodes.BINARY)
end

function ws_client_mt.__index:setOnTextMessageReceived(callback)
  self.on_text_message_received = callback
end

function ws_client_mt.__index:setOnBinaryMessageReceived(callback)
  self.on_binary_message_received = callback
end

local function processReceivedData(ws, err, chunk)
  assert(not err, err)
  if chunk then
    local encoded = chunk
    if ws.receive.last then
      encoded = ws.receive.last .. chunk
      ws.receive.last = nil
    end

    repeat
      local decoded,fin,opcode,rest = frame.decode(encoded)
      if decoded then
        if not ws.receive.first_opcode then
          ws.receive.first_opcode = opcode
        end
        table.insert(ws.receive.frames, decoded)
        encoded = rest
        if fin == true then
          local message = table.concat(ws.receive.frames)
          if ws.receive.first_opcode == frame.opcodes.TEXT then
            ws.on_text_message_received(message)
          elseif ws.receive.first_opcode == frame.opcodes.BINARY then
            ws.on_binary_message_received(message)
          elseif ws.receive.first_opcode == frame.opcodes.CLOSE then
            if ws.isConnected then
              local code,reason = frame.decode_close(message)
              local encoded_close = frame.encode_close(code or 1000,reason)
              encoded_close = frame.encode(encoded_close,frame.opcodes.CLOSE,true)
              ws.tcp:write(encoded_close)
              -- on_close function
            else
              print("Connection is not established, nothing to close")
            end
          end
          ws.receive.frames = {}
          ws.receive.first_opcode = nil
        end
      end
    until not decoded

    if #encoded > 0 then
      ws.receive.last = encoded
    end
  else
    print("Really No messages")
    -- ws.tcp:close()
    ws.isConnected = false
  end
end

local function setupWsConnection(self)
  local response = ""
  self.tcp:read_start(function(err, chunk)
    assert(not err, err)
    if chunk then
      print("Received: " .. chunk)
      response = response .. chunk
      if response:sub(#response-3) == "\r\n\r\n" then
        local headers = handshake.http_headers(response)
        local expected_accept = handshake.sec_websocket_accept(self.key)
        if headers['sec-websocket-accept'] ~= expected_accept then
          print('Error: sec-websocket-accept is not received')
          self.tcp:read_stop()
          return
        end
        self.tcp:read_start(function(d_err, d_chunk)
          processReceivedData(self, d_err, d_chunk)
        end)
        print("WS protocol has been initialized")
        self.isConnected = true
      end
    else
      print("No messages")
      self.tcp:close()
      self.isConnected = false
    end
  end)

  local req = handshake.upgrade_request
  {
    key = self.key,
    host = self.host,
    port = self.port,
    protocols = self.ws_protocols_tbl,
    uri = self.uri
  }
  print ("request: " .. tostring(req))

  self.tcp:write(req)
end

function WsClient.new()
  local res = {}
  res.tcp = uv.new_tcp()
  res.isConnected = false
  res.receive = {
    frames = {},
    first_opcode = nil,
    last = nil
  }
  res.on_text_message_received = function(data)
    print("Text message received: " .. data)
  end
  res.on_binary_message_received = function(data)
    print("Binary message received: " .. data)
  end
  res.on_connect = function(err)
    assert(not err, err)
    print("Connected to socket " .. tostring(res.host) .. ":" .. tostring(res.port))
    setupWsConnection(res)
  end
  setmetatable(res, ws_client_mt)
  return res
end

return WsClient
