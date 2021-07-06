--- Module which provides ATF configuration for predefined mobile devices
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local config = { }

config.MobileDevices = {
  { host = "127.0.0.1", port = 65432 }
}

config.defaultMobileDeviceInfo = { }
config.defaultMobileDeviceInfo.id = "localhost"
config.defaultMobileDeviceInfo.host = "127.0.0.1"
config.defaultMobileDeviceInfo.port = 12346
config.defaultMobileDeviceInfo.isRealDevice = false
-- config.defaultMobileDeviceInfo.macAddress - is dependent from hardware and will be calculated by configuration loader
config.defaultMobileDeviceInfo.osType = "Ubuntu"
config.defaultMobileDeviceInfo.transportTypes = { "TCP", "WS", "WSS" }

return config
