local events = require("events")
local expectations = require('expectations')
local tcp = require("tcp_connection")
local file_connection = require("file_connection")
local mobile = require("mobile_connection")
local mobile_session = require("mobile_session")

local Mob = {}

--- Raise event on mobile connection
local function MobRaiseEvent(self, pEvent, pEventName)
  if pEventName == nil then pEventName = "noname" end
  reporter.AddMessage(debug.getinfo(1, "n").name, pEventName)
  event_dispatcher:RaiseEvent(self, pEvent)
end

--- Create expectation for event on mobile connection
local function MobExpectEvent(self, pEvent, pEventName)
  if pEventName == nil then pEventName = "noname" end
  local ret = expectations.Expectation(pEventName, self)
  ret.event = pEvent
  event_dispatcher:AddEvent(self, pEvent, ret)
  return ret
end

local function addConnectionHandlers(pMobConn, pConnectHandler, pDisconnectHandler)
  local expC = pMobConn:ExpectEvent(events.connectedEvent, "Connected")
  :Pin()
  :Times(AnyNumber())
  if type(pConnectHandler) == "function" then
    expC:Do(pConnectHandler)
  else
    print("ERROR: ConnectHandler is not a function")
  end

  local expD = pMobConn:ExpectEvent(events.disconnectedEvent, "Disconnected")
  :Pin()
  :Times(AnyNumber())
  if type(pDisconnectHandler) == "function" then
    expD:Do(pDisconnectHandler)
  else
    print("ERROR: DisconnectHandler is not a function")
  end
end

--[[ @createConnection: Create mobile connection
--! @parameters:
--! pMobConnHost - mobile connection host
--! pMobConnPort - mobile connection port
--! @return: MobileConnection
--]]
function Mob.createMobileConnection(pMobConnHost, pMobConnPort, pConnectHandler, pDisconnectHandler)
  local filename = "mobile.out"
  local tcpConnection = tcp.Connection(pMobConnHost, pMobConnPort)
  local fileConnection = file_connection.FileConnection(filename, tcpConnection)
  local connection = mobile.MobileConnection(fileConnection)
  connection.RaiseEvent = MobRaiseEvent
  connection.ExpectEvent = MobExpectEvent
  connection.host = pMobConnHost
  connection.port = pMobConnPort
  event_dispatcher:AddConnection(connection)
  addConnectionHandlers(connection, pConnectHandler, pDisconnectHandler)
  return connection
end

----------------------------------------------------------------------------

--- Provide default configuration for mobile session creation
local function getDefaultMobSessionConfig()
  local mobSesionConfig = {
    activateHeartbeat = false,
    sendHeartbeatToSDL = false,
    answerHeartbeatFromSDL = false,
    ignoreSDLHeartBeatACK = false
  }

  if config.defaultProtocolVersion > 2 then
    mobSesionConfig.activateHeartbeat = true
    mobSesionConfig.sendHeartbeatToSDL = true
    mobSesionConfig.answerHeartbeatFromSDL = true
    mobSesionConfig.ignoreSDLHeartBeatACK = true
  end
  return mobSesionConfig
end

function Mob.createSession(pMobConn)
  local mobSesionConfig = getDefaultMobSessionConfig()
  local dummyTest = {
    expectations_list = expectations.ExpectationsList()
  }
  local session = mobile_session.MobileSession(dummyTest, pMobConn)
  for k, v in pairs(mobSesionConfig) do
    session[k] = v
  end
  print("Mobile session is created")
  return session
end

return Mob
