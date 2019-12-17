--- Module which provides ATF base configuration
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local config = { }
--- Flag which defines usage of color for reporting
config.color = true

--- Debug option
config.debug = true

--- Define timeout for Heartbeat in msec
config.heartbeatTimeout = 7000
--- Flag which defines whether ATF checks all validations for particular expectation or just till the first which fails
config.checkAllValidations = false
--- Define default version of Ford protocol
-- 1 - basic
--
-- 2 - RPC, encryption
--
-- 3 - video/audio streaming, heartbeat
-- 4 - SDL 4.0
config.defaultProtocolVersion = 3

--- Logs and Reports
--
--- Flag which defines whether ATF displays time of test step run
config.ShowTimeInConsole = true
--- Flag which defines whether ATF performs validation of Mobile and HMI messages by API
config.ValidateSchema = true
--- Flag which defines whether ATF ignores collecting of reports
config.excludeReport = true
--- Flag which defines whether ATF creates full ATF logs (with json files and service messages)
config.storeFullATFLogs = true
--- Flag which defines whether ATF stores full SDLCore logs
config.storeFullSDLLogs = true
--- Define path to collected ATF and SDL logs
config.reportPath = "./TestingReports"

return config
