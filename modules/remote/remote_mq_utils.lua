--- Module which provides transport level interface for emulate connection with HMI for SDL
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @module RemoteMqUtils
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local constants = require("remote/remote_constants")

local RemoteMqUtils = {
  mt = { __index = {} }
}

local function HandleResult(result, data)
  local isResultValid = false
  for _, v in pairs(constants.ERROR_CODE) do
    if result == v then
      isResultValid = true
      break;
    end
  end

  if not isResultValid then
    error("Module RemoteMqUtils received invalid result value: " .. result)
  end

  if result == constants.ERROR_CODE.SUCCESS then
    if data == nil then
      return true
    else
      return true, data
    end
  end
  if data == nil then
    return false
  else
    return false, nil
  end
end

--- Type which provides transport level interface for emulate connection with HMI for SDL
-- @type RemoteMqUtils

--- Construct instance of RemoteMqUtils type
-- @tparam RemoteConnection connection RemoteConnection instance
-- @tparam table mqConfig Parameters to create/open MQ
-- @treturn RemoteMqUtils Constructed instance
function RemoteMqUtils.RemoteMqUtils(connection, mqConfig)
  local res = {
    name = mqConfig.name,
    max_messages_number = mqConfig.max_messages_number,
    max_message_size = mqConfig.max_message_size,
    flags = mqConfig.flags,
    mode = mqConfig.mode
  }
  res.connection = connection:GetConnection()
  setmetatable(res, RemoteMqUtils.mt)
  return res
end

--- Check connection
-- @treturn boolean Return true in case connection is active
function RemoteMqUtils.mt.__index:Connected()
  return self.connection:connected()
end

--- Open Mq with default params
-- @treturn boolean Return true in case of success
function RemoteMqUtils.mt.__index:Open()
  max_messages_number = nil
  max_message_size = nil
  flags = nil
  mode = nil
  return HandleResult(self.connection:open(self.name))
end

--- Open Mq with params
-- @treturn boolean Return true in case of success
function RemoteMqUtils.mt.__index:OpenWithParams()
  return HandleResult(
      self.connection:open_with_params(self.name,
                                       self.max_messages_number,
                                       self.max_message_size,
                                       self.flags,
                                       self.mode))
end

--- Send data to Mq
-- @tparam string data Data to send
-- @treturn boolean Return true in case of success
function RemoteMqUtils.mt.__index:Send(data)
  return HandleResult(self.connection:send(self.name, data))
end

--- Receive data from Mq
-- @treturn boolean Return true in case of success
-- @treturn string Received data
function RemoteMqUtils.mt.__index:Receive()
  return HandleResult(self.connection:receive(self.name))
end

--- Close Mq
-- @treturn boolean Return true in case of success
function RemoteMqUtils.mt.__index:Close()
  return HandleResult(self.connection:close(self.name))
end

--- Unlink Mq
-- @treturn boolean Return true in case of success
function RemoteMqUtils.mt.__index:Unlink()
  return HandleResult(self.connection:unlink(self.name))
end

--- Clear Mq
-- @treturn boolean Return true in case of success
function RemoteMqUtils.mt.__index:Clear()
  HandleResult(self.connection:clear())
end

return RemoteMqUtils
