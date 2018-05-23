--- Module which provides transport level interface for emulate connection with HMI for SDL
--
-- *Dependencies:* `libSDLRemoteTestAdapter.so`
--
-- *Globals:* `atf_logger`, `qt`
-- @module remote_hmi_adapter_connection.lua
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local json = require("json")
local remote_adapter = require("SDLRemoteTestAdapter")

local RemoteHMIAdapter = {
  mt = { __index = {} }
}

--- Type which provides transport level interface for emulate connection with HMI for SDL
-- @type RemoteHMIAdapter

--- Construct instance of WebSocketConnection type
-- @tparam string url URL for websocket
-- @tparam number port Port for Websocket
-- @treturn WebSocketConnection Constructed instance
function RemoteHMIAdapter.RemoteHMIAdapterConnection(url, port)
  local res =
  {
    url = url,
    port = port
  }
  setmetatable(res, connection.mt)
  res.qtproxy = qt.dynamic()
  if (res.connection) then
    return res
  else 
    return nil
  end  
end


--- Connect with SDL
function RemoteHMIAdapter.mt.__index:Connect()
  self.connection = remote_adapter:new(self.url,self.port)
end

function RemoteHMIAdapter.mt.__index:StartReadingLoop()
   
  function self.qtproxy.BlockingRead() 
    local data = self.connection.read()
    qtproxy:DataAvailable(data)
  end
  self.reading_timer = timers.Timer()
  qt.connect(self.reading_timer, "timeout()", self.qtproxy, "BlockingRead()")
  self.reading_timer:start(10)
end

--- Check 'self' argument
local function checkSelfArg(s)
  if type(s) ~= "table" or
  getmetatable(s) ~= connection.mt then
    error("Invalid argument 'self': must be connection (use ':', not '.')")
  end
end

--- Send message from HMI to SDL
-- @tparam string text Message
function RemoteHMIAdapter.mt.__index:Send(text)
  atf_logger.LOG("HMItoSDL", text)
  self.connection:send(text)
end

--- Set handler for OnInputData
-- @tparam function func Handler function
function RemoteHMIAdapter.mt.__index:OnInputData(func)
  local d = self.qtproxy
  local this = self
  function d:textMessageReceived(text)
    atf_logger.LOG("SDLtoHMI", text)
    local data = json.decode(text)
    --print("ws input:", text)
    func(this, data)
  end
  qt.connect(self.qtproxy, "readDone(QString)", d, "textMessageReceived(QString)")
end

--- Set handler for OnDataSent
-- @tparam function func Handler function
function RemoteHMIAdapter.mt.__index:OnDataSent(func)
  local d = self.qtproxy
  local this = self
  function d:bytesWritten(num)
    func(this, num)
  end
  qt.connect(self.socket, "bytesWritten(qint64)", d, "bytesWritten(qint64)")
end

--- Set handler for OnConnected
-- @tparam function func Handler function
function RemoteHMIAdapter.mt.__index:OnConnected(func)
  if self.qtproxy.connected then
    error("RemoteHMIAdapterConnection connection: connected signal is handled already")
  end
  local this = self
  self.qtproxy.connected = function() func(this) end
  qt.connect(self.socket, "connected()", self.qtproxy, "connected()")
end

--- Set handler for OnDisconnected
-- @tparam function func Handler function
function RemoteHMIAdapter.mt.__index:OnDisconnected(func)
  if self.qtproxy.disconnected then
    error("RemoteHMIAdapterConnection connection: disconnected signal is handled already")
  end
  local this = self
  self.qtproxy.disconnected = function() func(this) end
  -- TODO : Handle disconnection
end

--- Close connection
function RemoteHMIAdapter.mt.__index:Close()
  checkSelfArg(self)
  self.reading_timer:stop()
  self.connection:close();

end

return RemoteHMIAdapter
