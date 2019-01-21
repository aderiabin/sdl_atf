--- Module which provides ATF configuration for remote connection by websocket protocol
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local config = {}
--- HMI configuration
config.hmiAdapterConfig = {}
config.hmiAdapterConfig.hmiAdapterType = "Remote"
--- Define configuration parameters for Remote HMI connection
config.hmiAdapterConfig.Remote = {}
--- Define configuration parameters for HMI connection on WebSocket base
-- for Remote HMI connection
config.hmiAdapterConfig.Remote.WebSocketConfig = {}
config.hmiAdapterConfig.Remote.WebSocketConfig.host = "127.0.0.1"
config.hmiAdapterConfig.Remote.WebSocketConfig.port = 8087

return config
