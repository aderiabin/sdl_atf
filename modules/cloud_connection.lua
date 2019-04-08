--- Module which provides transport level interface for emulate connection with cloud for SDL
--
-- *Dependencies:* `qt`, `network`
--
-- *Globals:* `xmlReporter`, `qt`, `network`
-- @module cloud_connection
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local Cloud = { mt = { __index = {} } }

--- Type which provides transport level interface for emulate connection with cloud for SDL
-- @type Connection

--- Construct instance of Connection type
-- @tparam string port Cloud port
-- @treturn Connection Constructed instance
function Cloud.Connection(port)
  local res =
  {
    port = port
  }
  res.server_socket = network.WebSocketServer()
  setmetatable(res, Cloud.mt)
  res.qtproxy = qt.dynamic()
  return res
end

--- Check 'self' argument
local function checkSelfArg(s)
  if type(s) ~= "table" or
  getmetatable(s) ~= Cloud.mt then
    error("Invalid argument 'self': must be connection (use ':', not '.')")
  end
end

local function checkConnection(connection)
    if type(connection) ~= "table" or
    connection.socket == nil then
      print("WebSocket connection is not established yet")
      return false
    end
    return true
  end

--- Start to listen of connections from SDL through QT transport interface
function Cloud.mt.__index:Listen()
  xmlReporter.AddMessage("cloud_connection","Open")
  checkSelfArg(self)
  if self.server_socket:listen(self.port) then
    return true
  end
  print("Cloud server does not started successfully")
  return false
end

--- Connect function handler for Cloud server
function Cloud.mt.__index:Connect()
  error("Cloud connection: Cloud is server. It can not initiate connection. Use Listen() instead to start to listen specified port")
end

-- - Send pack of messages from mobile to SDL
-- @tparam table data Data to be sent
function Cloud.mt.__index:Send(data)
  -- xmlReporter.AddMessage("cloud_connection","Send", data)
  checkSelfArg(self)
  if checkConnection(self) then
    for _, c in ipairs(data) do
      self.socket:binary_write(c)
    end
  end
end

--- Set handler for OnInputData
-- @tparam function func Handler function
function Cloud.mt.__index:OnInputData(func)
  local this = self

  function self.qtproxy:binaryMessageReceived(data)
    func(this, data)
  end
end

--- Set handler for OnDataSent
-- @tparam function func Handler function
function Cloud.mt.__index:OnDataSent(func)
  local this = self

  function self.qtproxy:bytesWritten(num)
    func(this, num)
  end
end

--- Set handler for OnConnected
-- @tparam function func Handler function
function Cloud.mt.__index:OnConnected(func)
  checkSelfArg(self)
  if self.qtproxy.connected then
    error("Cloud connection: connected signal is handled already")
  end
  local this = self
  self.qtproxy.connected = function()
    if self.socket then
      error("Cloud connection: ATF restriction - cloud server allows only one active connection.")
    end
    self.socket = self.server_socket:get_connection()
    qt.connect(self.socket, "bytesWritten(qint64)", self.qtproxy, "bytesWritten(qint64)")
    qt.connect(self.socket, "binaryMessageReceived(QByteArray)", self.qtproxy, "binaryMessageReceived(QByteArray)")
    qt.connect(self.socket, "disconnected()", self.qtproxy, "disconnected()")
    func(this)
  end
  qt.connect(self.server_socket, "newConnection()", self.qtproxy, "connected()")
end

--- Set handler for OnDisconnected
-- @tparam function func Handler function
function Cloud.mt.__index:OnDisconnected(func)
  checkSelfArg(self)
  if self.qtproxy.disconnected then
    error("Cloud connection: disconnected signal is handled already")
  end
  local this = self
  self.qtproxy.disconnected = function()
    this.socket = nil
    func(this)
  end
end

--- Close connection on server
function Cloud.mt.__index:Close()
  xmlReporter.AddMessage("cloud_connection","Close")
  checkSelfArg(self)
  self.socket:close()
  self.socket = nil
end

--- Stop listening of specified port
function Cloud.mt.__index:Stop()
  xmlReporter.AddMessage("cloud_connection","Stop")
  checkSelfArg(self)
  self:CloseConnection()
  self.server_socket:close()
end

return Cloud
