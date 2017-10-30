---------------------------------------------------------------------------------------------------
-- User story:
-- Use case:
-- Item:
--
-- Requirement summary:
--
-- Description:
-- In case:
---------------------------------------------------------------------------------------------------
--[[ Required Shared libraries ]]
local Test = require('connecttest')
require('cardinalities')

--[[ Scenario ]]
function Test:StartSecureSession()
  self.mobileConnection:StartSecureSession(self)
end

function Test:Activate_app()
  local pHMIAppId = hmiAppIds[config["application" .. pAppId].registerAppInterfaceParams.appID]
  local mobSession = self.mobileSession
  local requestId = self.hmiConnection:SendRequest("SDL.ActivateApp", { appID = pHMIAppId })
  EXPECT_HMIRESPONSE(requestId)
  if mobSession:IsSecuredSession() then
    mobSession:ExpectEncryptedNotification("OnHMIStatus", { hmiLevel = "FULL", audioStreamingState = "AUDIBLE", systemContext = "MAIN" })
  else
    mobSession:ExpectNotification("OnHMIStatus", { hmiLevel = "FULL", audioStreamingState = "AUDIBLE", systemContext = "MAIN" })
  end
end

function Test:AddCommandSecured()
  local iCmdID = 1
  local iPosition = 1
  local cid = self.mobileSession:SendRPC("AddCommand",
  {
    cmdID = iCmdID,
    menuParams =
    {
      position = iPosition,
      menuName ="Command"..tostring(iCmdID)
    }
  })
  EXPECT_HMICALL("UI.AddCommand",
  {
    cmdID = iCmdID,
    menuParams =
    {
      position = iPosition,
      menuName ="Command"..tostring(iCmdID)
    }
  })
  :Do(function(_,data)
    self.hmiConnection:SendResponse(data.id, data.method, "SUCCESS", {})
  end)
  self.mobileSession1:ExpectEncryptedResponse(cid, {  success = true, resultCode = "SUCCESS"  })
  self.mobileSession1:ExpectEncryptedNotification("OnHashChange")
end
