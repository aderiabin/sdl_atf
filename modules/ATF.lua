local remote_connection = require("remote/remote_connection")
local remote_file_utils = require("remote/remote_file_utils")
local remote_application_utils = require("remote/remote_application_utils")
local applink_connection = require("applink_connection")

ATF = {}

if config.remoteConnection.enabled then
  ATF.remoteConnection = remote_connection.RemoteConnection(config.remoteConnection.url, config.remoteConnection.port)
  ATF.remoteConnection:Connect()
  ATF.remoteUtils = {}
  ATF.remoteUtils.file = remote_file_utils.RemoteFileUtils(Test.remoteConnection)
  ATF.remoteUtils.app = remote_application_utils.RemoteAppUtils(Test.remoteConnection)
  ATF.applinkConnection = applink_connection.ApplinkConnection(Test.remoteConnection)
end

return ATF
