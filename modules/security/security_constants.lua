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
SecurityConstants.SECURITY_STATUS = {
  ERROR = -1,
  SUCCESS = 0,
  NO_DATA = 1,
  NO_ENCRYPTION = 2,
}

SecurityConstants.BIO_TYPES = {
  SOURCE = 0,
  FILTER = 1
}

SecurityConstants.PROTOCOLS = {
	AUTO = 0,
	SSL = 1,
	TLS = 2,
	DTLS = 3,
}

return SecurityConstants