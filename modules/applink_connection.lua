---- Provides emulation of applink signals to SDL (used for remote SDL)
--
--
-- *Dependencies:* `connecttest`
--
-- @module applinkConnection
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>
local applinkConnection = {}

local con = require('connecttest')

local SDLMsgType ={
	SDL_MSG_SDL_START = 0x10,
	SDL_MSG_SDL_STOP,
	SDL_MSG_SDL_JSON,
	SDL_MSG_APPLINK_JSON,
	SDL_MSG_BT_DEVICE_CONNECT,
	SDL_MSG_BT_DEVICE_CONNECT_ACK,
  SDL_MSG_BT_DEVICE_SPP_DISCONNECT,
	SDL_MSG_BT_DEVICE_DISCONNECT,
	SDL_MSG_BT_DEVICE_DISCONNECT_ACK,
  SDL_MSG_BT_DEVICE_CONNECT_END,
  SDL_MSG_IPOD_DEVICE_CONNECT,
  SDL_MSG_IPOD_DEVICE_CONNECT_ACK,
  SDL_MSG_IPOD_DEVICE_DISCONNECT,
  SDL_MSG_IPOD_DEVICE_DISCONNECT_ACK,
  SDL_MSG_START_USB_LOGGING,
  SDL_MSG_HEARTBEAT_ACK,
  SDL_MSG_IPOD_DEVICE_TRANSPORT_SWITCH,
  SDL_MSG_IPOD_DEVICE_TRANSPORT_SWITCH_ACK,
  SDL_MSG_LOW_VOLTAGE = 100,
  SDL_MSG_WAKE_UP,
  SDL_MSG_POWER_DOWN,
  TAKE_AOA,
  AOA_TAKEN,
  RELEASE_AOA,
  AOA_RELEASED,
  AOA_INACCESSIBLE,
  DEVICE_NOT_FOUND,
  DEVICE_ALREADY_CONNECTED
}

--скорее всего тут неправильно я совсем запутался где нам нужен селф а где нет и как правильно вызвать метод и почему
-- он вообще в тестебейзе а не в коннект тесте и вызывать его надо из коннект теста или тестбейза
function applinkConnection:sendSignal(signal)
	con.Test.remoteUtils.mq.sendSignal(SDLMsgType.signal)
end

return applinkConnection
