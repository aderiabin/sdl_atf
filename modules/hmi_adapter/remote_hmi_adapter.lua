--- Module which provides transport level interface for emulate connection with HMI for SDL
--
-- *Dependencies:* `RemoteHMIAdapter`
--
-- *Globals:* `xmlReporter`, `qt`, `timers`, `atf_logger`
-- @module RemoteHMIAdapter
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local json = require("json")
local remote_adapter = require("RemoteHMIAdapter")
local RemoteHMIAdapter = {
  mt = { __index = {} }
}

--- Type which provides transport level interface for emulate connection with HMI for SDL
-- @type RemoteHMIAdapter

--- Construct instance of WebSocketConnection type
-- @tparam params parameters for Remote adapter
-- @treturn RemoteHMIAdapter Constructed instance
function RemoteHMIAdapter.Connection(params)
  local res =
  {
    url = params.url,
    port = params.port,
    inMqConfig = params.inMqConfig,
    outMqConfig = params.outMqConfig,
  }
  res.connection = remote_adapter:new(res.url, res.port, res.inMqConfig, res.outMqConfig)
  setmetatable(res, RemoteHMIAdapter.mt)
  res.qtproxy = qt.dynamic()
  return res
end

--- Check 'self' argument
local function checkSelfArg(s)
  if type(s) ~= "table" or
  getmetatable(s) ~= RemoteHMIAdapter.mt then
    error("Invalid argument 'self': must be connection (use ':', not '.')")
  end
end

--- Connect with SDL
function RemoteHMIAdapter.mt.__index:Connect()
  xmlReporter.AddMessage("remote_hmi_adapter_connection","Connect")
  checkSelfArg(self)
  self.connection:connect()
  -- -- ToDo (aderiabin): Add unsuccess connect check
end

--- Send message from HMI to SDL
-- @tparam string text Message
function RemoteHMIAdapter.mt.__index:Send(text)
  atf_logger.LOG("HMItoSDL", text)
  self.connection:write(text)
end

--- Set handler for OnInputData
-- @tparam function func Handler function
function RemoteHMIAdapter.mt.__index:OnInputData(func)
  local d = self.qtproxy
  local this = self
  function d:textMessageReceived(text)
    atf_logger.LOG("SDLtoHMI", text)
    local data = json.decode(text)
    func(this, data)
  end
  qt.connect(self.connection.__self, "textMessageReceived(QString)", d, "textMessageReceived(QString)")
end

--- Set handler for OnDataSent
-- @tparam function func Handler function
function RemoteHMIAdapter.mt.__index:OnDataSent(func)
  local d = self.qtproxy
  local this = self
  function d:bytesWritten(num)
    func(this, num)
  end
  qt.connect(self.connection.__self, "bytesWritten(qint64)", d, "bytesWritten(qint64)")
end

--- Set handler for OnConnected
-- @tparam function func Handler function
function RemoteHMIAdapter.mt.__index:OnConnected(func)
  if self.qtproxy.connected then
    error("RemoteHMIAdapterConnection connection: connected signal is handled already")
  end
  local this = self
  self.qtproxy.connected = function() func(this) end
  qt.connect(self.connection.__self, "connected()", self.qtproxy, "connected()")
end

--- Set handler for OnDisconnected
-- @tparam function func Handler function
function RemoteHMIAdapter.mt.__index:OnDisconnected(func)
  if self.qtproxy.disconnected then
    error("RemoteHMIAdapterConnection connection: disconnected signal is handled already")
  end
  local this = self
  self.qtproxy.disconnected = function() func(this) end
  qt.connect(self.connection.__self, "disconnected()", self.qtproxy, "disconnected()")
end

--- Close connection
function RemoteHMIAdapter.mt.__index:Close()
  self.connection = nil
end

return RemoteHMIAdapter
