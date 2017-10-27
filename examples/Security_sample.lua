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
local expectations = require('expectations')
require('cardinalities')

--[[ Scenario ]]
function Test:Rai_ptu()
  local id = 1
  self["mobileSession" .. id] = mobile_session.MobileSession(self, self.mobileConnection)
  self["mobileSession" .. id]:StartService(7)
  :Do(function()
      local corId = self["mobileSession" .. id]:SendRPC("RegisterAppInterface", config["application" .. id].registerAppInterfaceParams)
      EXPECT_HMINOTIFICATION("BasicCommunication.OnAppRegistered", { application = { appName = config["application" .. id].registerAppInterfaceParams.appName } })
      :Do(function(_, d1)
          hmiAppIds[config["application" .. id].registerAppInterfaceParams.appID] = d1.params.application.appID
          EXPECT_HMINOTIFICATION("SDL.OnStatusUpdate", { status = "UPDATE_NEEDED" }, { status = "UPDATING" }, { status = "UP_TO_DATE" })
          :Times(3)
          EXPECT_HMICALL("BasicCommunication.PolicyUpdate")
          :Do(function(_, d2)
              self.hmiConnection:SendResponse(d2.id, d2.method, "SUCCESS", { })
              ptu_table = jsonFileToTable(d2.params.file)
              ptu(self, ptu_update_func)
            end)
        end)
      self["mobileSession" .. id]:ExpectResponse(corId, { success = true, resultCode = "SUCCESS" })
      :Do(function()
          self["mobileSession" .. id]:ExpectNotification("OnHMIStatus", { hmiLevel = "NONE", audioStreamingState = "NOT_AUDIBLE", systemContext = "MAIN" })
          :Times(AtLeast(1)) -- issue with SDL --> notification is sent twice
          self["mobileSession" .. id]:ExpectNotification("OnPermissionsChange")
        end)
    end)
end

function Test:StartSecureRPCService()
  self.mobileSession1:StartSecureService(7)
  :Do(function(exp,_)
      if exp.status == expectations.FAILED then return end
    end)
end

function Test:AddCommandSecured()
  local iCmdID = 1
  local iPosition = 1
  local cid = self.mobileSession1:SendRPC("AddCommand",
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
