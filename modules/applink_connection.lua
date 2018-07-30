---- Provides emulation of applink signals to SDL (used for remote SDL)
--
--
-- *Dependencies:* `remote/remote_mq_utils`
--
-- *Globals:* ATF
-- @module applinkConnection
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local remote_mq_utils = require("remote/remote_mq_utils")

local ApplinkConnection = {
  mt = { __index = {} }
}
---Appink messege type enumeration
ApplinkConnection.SDLMsgType ={
  SDL_MSG_SDL_START = 0x10,
  SDL_MSG_SDL_STOP = 0x11,
  SDL_MSG_SDL_JSON = 0x12,
  SDL_MSG_APPLINK_JSON = 0x13,
  SDL_MSG_BT_DEVICE_CONNECT = 0x14,
  SDL_MSG_BT_DEVICE_CONNECT_ACK = 0x15,
  SDL_MSG_BT_DEVICE_SPP_DISCONNECT = 0x16,
  SDL_MSG_BT_DEVICE_DISCONNECT = 0x17,
  SDL_MSG_BT_DEVICE_DISCONNECT_ACK = 0x18,
  SDL_MSG_BT_DEVICE_CONNECT_END = 0x19,
  SDL_MSG_IPOD_DEVICE_CONNECT = 0x1A,
  SDL_MSG_IPOD_DEVICE_CONNECT_ACK = 0x1B,
  SDL_MSG_IPOD_DEVICE_DISCONNECT = 0x1C,
  SDL_MSG_IPOD_DEVICE_DISCONNECT_ACK = 0x1E,
  SDL_MSG_START_USB_LOGGING = 0x1F,
  SDL_MSG_HEARTBEAT_ACK = 0x20,
  SDL_MSG_IPOD_DEVICE_TRANSPORT_SWITCH = 0x21,
  SDL_MSG_IPOD_DEVICE_TRANSPORT_SWITCH_ACK = 0x22,
  SDL_MSG_LOW_VOLTAGE = 100,
  SDL_MSG_WAKE_UP = 101,
  SDL_MSG_POWER_DOWN = 102,
  TAKE_AOA = 103,
  AOA_TAKEN = 104,
  RELEASE_AOA = 105,
  AOA_RELEASED = 106,
  AOA_INACCESSIBLE = 107,
  DEVICE_NOT_FOUND = 108,
  DEVICE_ALREADY_CONNECTED = 109
}

--- Construct instance of ApplinkConnection type
-- @tparam RemoteConnection connection RemoteConnection instance
-- @treturn ApplinkConnection Constructed instance
function ApplinkConnection.ApplinkConnection(connection)
  local res = { }
  res.connection = remote_mq_utils.RemoteMqUtils(connection, config.controlMqConfig)
  res.connection:OpenWithParams()
  setmetatable(res, ApplinkConnection.mt)
  return res
end

--- Send Applink signal to SDL
-- @tparam string signal Data to send
-- @treturn number Return 0 in case of success
function ApplinkConnection.mt.__index:sendSignal(signal)
  return self.connection:Send(string.char(signal))
end

return ApplinkConnection
