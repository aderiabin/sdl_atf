--- Module which provides ATF connection configuration
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local config = { }

--- Remote cofiguration
config.remoteConnection = {}
config.remoteConnection.enabled = false
--- Define host for default remote connection
config.remoteConnection.url = "127.0.0.1"
config.remoteConnection.port = 5555
config.hmiAdapterConfig = {}
config.hmiAdapterConfig.hmiAdapterType = "WebSocket"
--- Define configuration parameters for HMI connection on WebSocket base
config.hmiAdapterConfig.WebSocket = {}
config.hmiAdapterConfig.WebSocket.url = "ws://localhost"
config.hmiAdapterConfig.WebSocket.port = 8087
--- Define configuration parameters for Remote HMI connection
config.hmiAdapterConfig.Remote = {}
--- Define configuration parameters for HMI connection on WebSocket base
-- for Remote HMI connection
config.hmiAdapterConfig.Remote.WebSocketConfig = {}
config.hmiAdapterConfig.Remote.WebSocketConfig.host = "127.0.0.1"
config.hmiAdapterConfig.Remote.WebSocketConfig.port = 8087

--- Define host for default mobile device connection
config.mobileHost = "127.0.0.1"
--- Define port for default mobile device connection
config.mobilePort = 12345

--- Define host for SDL logs
config.sdl_logs_host = "localhost"
--- Define port for SDL logs
config.sdl_logs_port = 6676

return config
