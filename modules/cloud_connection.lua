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

  function res:inputData() end

  function res.qtproxy.readyRead()
    while true do
      local data = res.socket:read(81920)
      if data == '' then break end
      res.qtproxy:inputData(data)
    end
  end

  function res.qtproxy.newConnection()
    res.socket = res.server_socket:get_connection()
    qt.connect(res.socket, "readyRead()", res.qtproxy, "readyRead()")
  end

  qt.connect(res.server_socket, "newConnection()", res.qtproxy, "newConnection()")

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
function Cloud.mt.__index:Open()
  xmlReporter.AddMessage("cloud_connection","Open")
  checkSelfArg(self)
  if self.server_socket:listen(self.port) then
    self.qtproxy.connected();
  end
end

-- - Send pack of messages from mobile to SDL
-- @tparam table data Data to be sent
function Cloud.mt.__index:Send(data)
  -- xmlReporter.AddMessage("cloud_connection","Send", data)
  checkSelfArg(self)
  if checkConnection(self) then
    for _, c in ipairs(data) do
      self.socket:write(c)
    end
  end
end

--- Set handler for OnInputData
-- @tparam function func Handler function
function Cloud.mt.__index:OnInputData(func)
  checkSelfArg(self)
  local d = qt.dynamic()
  local this = self
  function d:inputData(data)
    func(this, data)
  end
  qt.connect(self.qtproxy, "inputData(QByteArray)", d, "inputData(QByteArray)")
end

--- Set handler for OnDataSent
-- @tparam function func Handler function
function Cloud.mt.__index:OnDataSent(func)
  local d = qt.dynamic()
  local this = self

  function d:bytesWritten(num)
    func(this, num)
  end
  qt.connect(self.socket, "bytesWritten(qint64)", d, "bytesWritten(qint64)")
end

--- Set handler for OnConnected
-- @tparam function func Handler function
function Cloud.mt.__index:OnConnected(func)
  checkSelfArg(self)
  if self.qtproxy.connected then
    error("Cloud connection: connected signal is handled already")
  end
  local this = self
  self.qtproxy.connected = function() func(this) end
  -- qt.connect(res.server_socket, "newConnection()", self.qtproxy, "connected()")
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
  qt.connect(self.socket, "disconnected()", self.qtproxy, "disconnected()")
end

--- Set handler for OnConnectionAccepted
-- @tparam function func Handler function
function Cloud.mt.__index:OnConnectionAccepted(func)
  checkSelfArg(self)
  if self.qtproxy.connectionAccepted then
    error("Cloud connection: connected accepted signal is handled already")
  end
  local this = self
  self.qtproxy.connectionAccepted = function() func(this) end
  qt.connect(res.server_socket, "newConnection()", self.qtproxy, "connectionAccepted()")
end

--- Close connection
function Cloud.mt.__index:Close()
  xmlReporter.AddMessage("cloud_connection","Close")
  checkSelfArg(self)
  self.socket:close()
  self.server_socket:close()
end

return Cloud
