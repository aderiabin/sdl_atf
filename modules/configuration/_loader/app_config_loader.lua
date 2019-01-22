--- Module which load and validate ATF configuration for predefined mobile applications
--
-- *Dependencies:* `configuration/default_applications_config`
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local config = require('configuration/default_applications_config')

-- ToDo: Add all necessary checks for ATF configuration
local function validateAll()
    for appConfig, appConfigParam in pairs(config) do
        if not appConfigParam.registerAppInterfaceParams or type(appConfigParam.registerAppInterfaceParams) ~= "table" then
            error("Application configuration " .. appConfig .. "does not have field 'registerAppInterfaceParams'")
        end

        if not appConfigParam.registerAppInterfaceParams.deviceInfo or type(appConfigParam.registerAppInterfaceParams.deviceInfo) ~= "table" then
            error("Application configuration " .. appConfig .. "does not have field 'registerAppInterfaceParams.deviceInfo'")
        end

        if not appConfigParam.registerAppInterfaceParams.deviceInfo.os then
            error("Application configuration " .. appConfig .. "does not have field 'registerAppInterfaceParams.deviceInfo.os'")
        end

        if appConfigParam.registerAppInterfaceParams.deviceInfo.os ~= "Android"
            and appConfigParam.registerAppInterfaceParams.deviceInfo.os ~= "iOS" then
            error("Application configuration " .. appConfig .. "has incorrect value of field 'registerAppInterfaceParams.deviceInfo.os'")
        end
    end

end

-- ToDo: Add all calculated fields for ATF configuration
local function CalculateFullAppId(appId)
    return "000" .. appId
end

config.application1.registerAppInterfaceParams.fullAppID = CalculateFullAppId(config.application1.registerAppInterfaceParams.appID)
config.application2.registerAppInterfaceParams.fullAppID = CalculateFullAppId(config.application2.registerAppInterfaceParams.appID)
config.application3.registerAppInterfaceParams.fullAppID = CalculateFullAppId(config.application3.registerAppInterfaceParams.appID)
config.application4.registerAppInterfaceParams.fullAppID = CalculateFullAppId(config.application4.registerAppInterfaceParams.appID)
config.application5.registerAppInterfaceParams.fullAppID = CalculateFullAppId(config.application5.registerAppInterfaceParams.appID)

for i = 1, 5 do
    config["application" .. i].registerAppInterfaceParams.syncMsgVersion = {}
    config["application" .. i].registerAppInterfaceParams.syncMsgVersion.majorVersion = 5
    config["application" .. i].registerAppInterfaceParams.syncMsgVersion.majorVersion = 0
end

--

validateAll()

return config
