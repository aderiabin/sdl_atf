--- Module which load and validate ATF security configuration
--
-- *Dependencies:* `configuration.security_config`, `security.security_constants`
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local config = require('configuration/security_config')
local security_constants = require('security/security_constants')

local function validateSecurityProtocol()
  local isFound = false
  for k in pairs(security_constants.PROTOCOLS)
    if k == config.SecurityProtocol then
      isFound = true
      break
    end
  end
  if not isFound then
      error("SecurityProtocol value " .. config.SecurityProtocol .. "is incorrect. See security/security_constants.lua PROTOCOLS enumeration")
  end
end

local function validateAll()
    validateSecurityProtocol()
end

--

validateAll()

return config