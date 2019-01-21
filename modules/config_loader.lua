--- Module which build ATF configuration from configuration pieces
--
-- *Dependencies:* `config`, `config_builder`
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local config = require('config')
local config_builder = require('config_builder')

-- ToDo: Add all necessary checks for ATF configuration
local function validateAll()

end

-- ToDo: Add all calculated fields for ATF configuration

local configurationBuilder = config_builder.ConfigurationBuilder(config)
configurationBuilder:addConfiguration("default_applications_config")

if config.remoteConnection.enabled then
    configurationBuilder:addConfiguration("websocket_remote_config")
else
    configurationBuilder:addConfiguration("websocket_local_config")
end

validateAll()

-- ToDo: Prevent changes of ATF configuration

return configurationBuilder:buildConfiguration()
