--- Module which provides transport level interface for emulate web engine connection to SDL
--
-- *Dependencies:* `qt`, `network`
--
-- *Globals:* `xmlReporter`, `qt`, `network`
-- @module WebEngine
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local WebEngine = { mt = { __index = {} } }

--- Type which provides transport level interface for emulate connection with mobile for SDL
-- @type Connection

--- Construct instance of Connection type
-- @tparam string host SDL host address
-- @tparam string port SDL port
-- @treturn Connection Constructed instance
function WebEngine.Connection(params)
  local res = {
    url = params.url,
    port = params.port
  }
  res.socket = network.WebSocket()
  setmetatable(res, WebEngine.mt)
  res.qtproxy = qt.dynamic()

  return res
end

--- Check 'self' argument
local function checkSelfArg(s)
  if type(s) ~= "table" or
  getmetatable(s) ~= WebEngine.mt then
    error("Invalid argument 'self': must be connection (use ':', not '.')")
  end
end

--- Connect to SDL through QT transport interface
function WebEngine.mt.__index:Connect()
  xmlReporter.AddMessage("websocket_connection","Connect")
  checkSelfArg(self)
  self.socket:open(self.url, self.port)
end

--- Send pack of messages from mobile to SDL
-- @tparam table data Data to be sent
function WebEngine.mt.__index:Send(data)
  checkSelfArg(self)
  for _, c in ipairs(data) do
    self.socket:binary_write(c)
  end
end

--- Set handler for OnInputData
-- @tparam function func Handler function
function WebEngine.mt.__index:OnInputData(func)
  local this = self
  function self.qtproxy:binaryMessageReceived(data)
    func(this, data)
  end
  qt.connect(self.socket, "binaryMessageReceived(QByteArray)", self.qtproxy, "binaryMessageReceived(QByteArray)")
end

--- Set handler for OnDataSent
-- @tparam function func Handler function
function WebEngine.mt.__index:OnDataSent(func)
  local this = self
  function self.qtproxy:bytesWritten(num)
    func(this, num)
  end
  qt.connect(self.socket, "bytesWritten(qint64)", self.qtproxy, "bytesWritten(qint64)")
end

--- Set handler for OnConnected
-- @tparam function func Handler function
function WebEngine.mt.__index:OnConnected(func)
  checkSelfArg(self)
  if self.qtproxy.connected then
    error("WebEngine connection: connected signal is handled already")
  end
  local this = self
  self.qtproxy.connected = function() func(this) end
  qt.connect(self.socket, "connected()", self.qtproxy, "connected()")
end

--- Set handler for OnDisconnected
-- @tparam function func Handler function
function WebEngine.mt.__index:OnDisconnected(func)
  checkSelfArg(self)
  if self.qtproxy.disconnected then
    error("WebEngine connection: disconnected signal is handled already")
  end
  local this = self
  self.qtproxy.disconnected = function() func(this) end
  qt.connect(self.socket, "disconnected()", self.qtproxy, "disconnected()")
end

--- Close connection
function WebEngine.mt.__index:Close()
  xmlReporter.AddMessage("websocket_connection", "Close")
  checkSelfArg(self)
  self.socket:close();
end

return WebEngine
