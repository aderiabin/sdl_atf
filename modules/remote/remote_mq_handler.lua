--- Module which provides transport level interface for emulate connection with HMI for SDL
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @module RemoteMqHandler
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local RemoteMqHandler = {
  mt = { __index = {} }
}

--- Type which provides transport level interface for emulate connection with HMI for SDL
-- @type RemoteMqHandler

--- Construct instance of RemoteMqHandler type
-- @tparam connection RemoteConnection instance
-- @tparam mqConfig Parameters to create/open MQ
-- @treturn RemoteMqHandler Constructed instance
function RemoteMqHandler.RemoteMqHandler(connection, mqConfig)
  local res = {
    name = mqConfig.name,
    max_messages_number = mqConfig.max_messages_number,
    max_message_size = mqConfig.max_message_size,
    flags = mqConfig.flags,
    mode = mqConfig.mode
  }
  res.connection = connection:GetConnection()
  setmetatable(res, RemoteMqHandler.mt)
  return res
end

--- Check connection
-- @treturn boolean Return true in case connection is active
function RemoteMqHandler.mt.__index:Connected()
  return self.connection:connected()
end

--- Open Mq with default params
-- @treturn number Return 0 in case of success
function RemoteMqHandler.mt.__index:Open()
  max_messages_number = nil
  max_message_size = nil
  flags = nil
  mode = nil
  return self.connection:open(self.name)
end

--- Open Mq with params
-- @treturn number Return 0 in case of success
function RemoteMqHandler.mt.__index:OpenWithParams()
  return self.connection:open_with_params(self.name,
                                          self.max_messages_number,
                                          self.max_message_size,
                                          self.flags,
                                          self.mode)
end

--- Send data to Mq
-- @treturn number Return 0 in case of success
function RemoteMqHandler.mt.__index:Send(data)
  return self.connection:send(self.name, data)
end

--- Receive data from Mq
-- @treturn number Return 0 in case of success
-- @treturn string Received data
function RemoteMqHandler.mt.__index:Receive()
  return self.connection:receive(self.name)
end

--- Close Mq
-- @treturn number Return 0 in case of success
function RemoteMqHandler.mt.__index:Close()
  self.connection:close(self.name)
end

--- Unlink Mq
-- @treturn number Return 0 in case of success
function RemoteMqHandler.mt.__index:Unlink()
  self.connection:unlink(self.name)
end

--- Clear Mq
-- @treturn number Return 0 in case of success
function RemoteMqHandler.mt.__index:Clear()
  self.connection:clear()
end

return RemoteMqHandler
