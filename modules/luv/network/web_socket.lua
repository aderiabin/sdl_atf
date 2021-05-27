local ws = require('luv.network.websocket.client_uv')

local WebSocketManager = {}

-- TCP client
local ws_mt = { __index = {} }

function ws_mt.__index:open() end

function ws_mt.__index:close() end

function ws_mt.__index:binary_write() end

function ws_mt.__index:write() end

function ws_mt.__index:read() end

function ws_mt.__index:setOnTextDataReceivedHandler(pHandlerFunc)
  if not type(pHandlerFunc) == "function" then
    print("OnTextDataReceivedHandler of " .. self .. " TCP client was not set because "
      .. pHandlerFunc .. " is not a function")
    return
  end
  self.onTextDataReceivedHandler = pHandlerFunc
end

function ws_mt.__index:setOnBinaryDataReceivedHandler(pHandlerFunc)
  if not type(pHandlerFunc) == "function" then
    print("OnBinaryDataReceivedHandler of " .. self .. " TCP client was not set because "
      .. pHandlerFunc .. " is not a function")
    return
  end
  self.onBinaryDataReceivedHandler = pHandlerFunc
end

function ws_mt.__index:setOnTextDataSentHandler(pHandlerFunc)
  if not type(pHandlerFunc) == "function" then
    print("OnTextDataSentHandler of " .. self .. " TCP client was not set because "
      .. pHandlerFunc .. " is not a function")
    return
  end
  self.onTextDataSentHandler = pHandlerFunc
end

function ws_mt.__index:setOnBinaryDataSentHandler(pHandlerFunc)
  if not type(pHandlerFunc) == "function" then
    print("OnBinaryDataSentHandler of " .. self .. " TCP client was not set because "
      .. pHandlerFunc .. " is not a function")
    return
  end
  self.onBinaryDataSentHandler = pHandlerFunc
end

function ws_mt.__index:setOnConnectedHandler(pHandlerFunc)
  if not type(pHandlerFunc) == "function" then
    print("OnConnectHandler of " .. self .. " TCP client was not set because "
      .. pHandlerFunc .. " is not a function")
    return
  end
  self.onConnectedHandler = pHandlerFunc
end

function ws_mt.__index:setOnDisconnectedHandler(pHandlerFunc)
  if not type(pHandlerFunc) == "function" then
    print("OnDisconnectHandler of " .. self .. " TCP client was not set because "
      .. pHandlerFunc .. " is not a function")
    return
  end
  self.onDisconnectedHandler = pHandlerFunc
end

-- TimersManager
function WebSocketManager.WebSocket()
  local res = {
    onTextDataReceivedHandler = nil,
    onBinaryDataReceivedHandler = nil,
    onTextDataSentHandler = nil,
    onBinaryDataSentHandler = nil,
    onConnectedHandler = nil,
    onDisconnectedHandler = nil
  }
  setmetatable(res, ws_mt)
  return res
end

return WebSocketManager
