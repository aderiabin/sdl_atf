--- Module which provides HMI Adapter control functionality on base of ATF configuration
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @module HmiAdapterController
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local HmiAdapterController = {}

local hmiAdapter
if config.hmiAdapterConfig.hmiAdapterType == "Remote" then
  hmiAdapter = require('hmi_adapter/remote_hmi_adapter')
else if config.hmiAdapterConfig.hmiAdapterType == "WebSocket" then
  hmiAdapter = require('hmi_adapter/websocket_hmi_adapter')
end

function HmiAdapterController.getHmiAdapter() {
	return hmiAdapter.Connection(config.hmiAdapterConfig[config.hmiAdapterConfig.hmiAdapterType])
}

return HmiAdapterController
