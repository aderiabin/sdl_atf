--- Module which provides interface for emulate connection with mobile for SDL
--
-- *Dependencies:* `file_connection`, `protocol_handler.protocol_handler`
--
-- *Globals:* `atf_logger`, `xmlReporter`
-- @module mobile_connection
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local ph = require('protocol_handler/protocol_handler')
local file_connection = require("file_connection")
local mobile_session = require("mobile_session")
local events = require('events')
local expectations = require('expectations')
local FAILED = expectations.FAILED


local MobileConnection = {
  mt = { __index = {} }
}

--- Type which provides interface for emulate connection with mobile for SDL
-- @type MobileConnection

--- Construct instance of MobileConnection type
-- @tparam FileConnection connection Lower level connection
-- @treturn MobileConnection Constructed instance
function MobileConnection.MobileConnection(connection)
  local res = { }
  res.connection = connection
  setmetatable(res, MobileConnection.mt)
  return res
end

--- Connect with SDL
function MobileConnection.mt.__index:Connect()
  self.connection:Connect()
end

--- Send pack of messages from mobile to SDL
-- @tparam table data Data to be sent
function MobileConnection.mt.__index:Send(data)
  local messages = { }
  local protocol_handler = ph.ProtocolHandler()
  for _, msg in ipairs(data) do
    atf_logger.LOG("MOBtoSDL", msg)
    local msgs = protocol_handler:Compose(msg)
    for _, m in ipairs(msgs) do
      table.insert(messages, m)
    end
  end
  self.connection:Send(messages)
end

--- Send frame from mobile to SDL
-- @tparam string bytes Bytes to be sent
function MobileConnection.mt.__index:SendFrame(frameMessage)
local protocol_handler = ph.ProtocolHandler()
  local frame = protocol_handler:GetBinaryFrame(frameMessage)
  -- atf_logger.LOG("MOBtoSDL", frameMessage)
  self.connection:Send({frame})
end

function MobileConnection.mt.__index:StartSession(test)
  test.mobileSession = mobile_session.MobileSession(
  test,
  test.mobileConnection,
  config.application1.registerAppInterfaceParams)
  return test.mobileSession:Start()
end

function MobileConnection.mt.__index:StartSecureSession(test)
  local startSecureSessionEvent = events.Event()
  startSecureSessionEvent.matches = function(_, data)
      return data.message == "Secured session started"
    end

  self:StartSession(test)
  :Do(function(exp, _)
      if exp.status == FAILED then return end
      test.mobileSession:StartSecureService(7)
      :Do(function(exp2, _)
        if exp2.status == FAILED then return end
        event_dispatcher:RaiseEvent(test.mobileConnection, {message = "Secured session started"})
      end)
    end)
  local ret = expectations.Expectation("StartedSecureSession", test.mobileConnection)
  ret.event = startSecureSessionEvent
  event_dispatcher:AddEvent(test.mobileConnection, startSecureSessionEvent, ret)
  return ret
end

--- Start streaming file from mobile to SDL
-- @tparam number session Session identificator
-- @tparam number service Sevice number
-- @tparam string filename Name of file to be streamed
-- @tparam number bandwidth Bandwidth in bytes
function MobileConnection.mt.__index:StartStreaming(session, service, filename, bandwidth)
  if getmetatable(self.connection) ~= file_connection.mt then
    error("Data streaming is impossible unless underlying connection is FileConnection")
  end
  xmlReporter.AddMessage("mobile_connection","StartStreaming", {["Session"] = session,
      ["Service"] = service,["FileName"] = filename,["Bandwidth"] = bandwidth })
  self.connection:StartStreaming(session, service, filename, bandwidth)
end

--- Stop streaming file from mobile to SDL
-- @tparam string filename Name of file to be streamed
function MobileConnection.mt.__index:StopStreaming(filename)
  xmlReporter.AddMessage("mobile_connection","StopStreaming", {["FileName"] = filename})
  self.connection:StopStreaming(filename)
end

--- Set handler for OnInputData
-- @tparam function func Handler function
function MobileConnection.mt.__index:OnInputData(func)
  local protocol_handler = ph.ProtocolHandler()
  local f =
  function(_, binary)
    local msgs = protocol_handler:Parse(binary)
    for _, msg in ipairs(msgs) do
      -- After refactoring should be moved in mobile session
      atf_logger.LOG("SDLtoMOB", msg)
      func(self, msg)
    end
  end
  self.connection:OnInputData(f)
end

--- Set handler for OnDataSent
-- @tparam function func Handler function
function MobileConnection.mt.__index:OnDataSent(func)
  self.connection:OnDataSent(func)
end

--- Set handler for OnMessageSent
-- @tparam function func Handler function
function MobileConnection.mt.__index:OnMessageSent(func)
  self.connection:OnMessageSent(func)
end

--- Set handler for OnConnected
-- @tparam function func Handler function
function MobileConnection.mt.__index:OnConnected(func)
  self.connection:OnConnected(function() func(self) end)
end

--- Set handler for OnDisconnected
-- @tparam function func Handler function
function MobileConnection.mt.__index:OnDisconnected(func)
  self.connection:OnDisconnected(function() func(self) end)
end

--- Close connection
function MobileConnection.mt.__index:Close()
  self.connection:Close()
end

return MobileConnection
