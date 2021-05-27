local tools = require('luv.network.websocket.tools')

local handshake = {}

function handshake.upgrade_request(req)
  local lines = {
    string.format('GET %s HTTP/1.1', req.uri or ''),
    string.format('Host: %s', req.host),
    'Upgrade: websocket',
    'Connection: Upgrade',
    string.format('Sec-WebSocket-Key: %s', req.key),
    string.format('Sec-WebSocket-Protocol: %s', table.concat(req.protocols,', ')),
    'Sec-WebSocket-Version: 13'
  }
  if req.origin then
      table.insert(lines, string.format('Origin: %s', req.origin))
  end
  if req.port and req.port ~= 80 then
    lines[2] = string.format('Host: %s:%d', req.host, req.port)
  end
  table.insert(lines, '\r\n')
  return table.concat(lines, '\r\n')
end

function handshake.http_headers(request)
  local headers = {}
  if not request:match('.*HTTP/1%.1') then
    return headers
  end
  request = request:match('[^\r\n]+\r\n(.*)')
  local empty_line
  for line in request:gmatch('[^\r\n]*\r\n') do
    local name,val = line:match('([^%s]+)%s*:%s*([^\r\n]+)')
    if name and val then
      name = name:lower()
      if not name:match('sec%-websocket') then
        val = val:lower()
      end
      if not headers[name] then
        headers[name] = val
      else
        headers[name] = headers[name] .. ',' .. val
      end
    elseif line == '\r\n' then
      empty_line = true
    else
      assert(false, line..'(' .. #line .. ')')
    end
  end
  return headers, request:match('\r\n\r\n(.*)')
end

local guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

function handshake.sec_websocket_accept(sec_websocket_key)
  local a = sec_websocket_key .. guid
  local sha1_hash = tools.sha1(a)
  assert((#sha1_hash % 2) == 0)
  return tools.base64.encode(sha1_hash)
end

return handshake
