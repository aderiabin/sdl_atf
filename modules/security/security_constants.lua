--- Module which provides constants for security
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @module security.security_constants
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local SecurityConstants = {}
--- Frame type enumeration
SecurityConstants.DECRIPTION_STATUS = {
  ERROR = -1,
  SUCCESS = 0,
  NO_DATA = 1,
  NOT_DECRYPTED = 2,
}

return SecurityConstants