--- Module which provides ATF connection configuration
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local config = { }

--- Define host for default mobile device connection
config.mobileHost = "127.0.0.1"
--- Define port for default mobile device connection
config.mobilePort = 12345

--- Define mySQL parameters
config.mySQLHost = "172.30.157.141"
config.mySQLUser = "root"
config.mySQLPassword = "1234"
config.mySQLDatabase = "ces"

return config
