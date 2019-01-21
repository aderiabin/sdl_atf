--- Module which provides ATF configuration for local connection by websocket protocol
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local config = {}
--- HMI configuration
config.hmiAdapterConfig = {}
config.hmiAdapterConfig.hmiAdapterType = "Local"
--- Define configuration parameters for HMI connection on WebSocket base
config.hmiAdapterConfig.WebSocket = {}
config.hmiAdapterConfig.WebSocket.url = "ws://localhost"
config.hmiAdapterConfig.WebSocket.port = 8087

return config
