local tcp = require('luv.network.tcp')
local ws = require('luv.network.web_socket')

local Network = {
  TcpClient = tcp.TcpClient,
  TcpServer = tcp.TcpServer,
  WebSocket = ws.WebSocket
}

return Network
