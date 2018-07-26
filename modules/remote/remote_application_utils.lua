--- Module which provides utils interface for application management on remote host
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @module RemoteAppUtils
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local RemoteAppUtils = {
	mt = { __index = {} }
  }

--- Module which provides utils interface for application management on remote host
-- @type RemoteAppUtils

--- Application status enumeration
RemoteAppUtils.APP_STATUS = {
  CRASHED = -1,
  NOT_RUNNING = 0,
  RUNNING = 1
}

--- Construct instance of RemoteAppUtils type
-- @tparam RemoteConnection connection RemoteConnection instance
-- @treturn RemoteAppUtils Constructed instance
function RemoteAppUtils.RemoteAppUtils(connection)
  local res = { }
  res.connection = connection:GetConnection()
  setmetatable(res, RemoteAppUtils.mt)
  return res
end

--- Check connection
-- @treturn boolean Return true in case connection is active
function RemoteAppUtils.mt.__index:Connected()
  return self.connection:connected()
end

--- Start application on remote host
-- @tparam string remotePathToApp Path to application file on remote host
-- @tparam string fileName Name of application executive file
-- @treturn boolean Return true in case of success
function RemoteAppUtils.mt.__index:StartApp(remotePathToApp, appName)
  -- self.connection:clear()
end

--- Stop application on remote host
-- @tparam string remotePathToApp Path to application file on remote host
-- @tparam string fileName Name of application executive file
-- @treturn boolean Return true in case of success
function RemoteAppUtils.mt.__index:StopApp(appName)
  -- self.connection:clear()
end

--- Check status of application on remote host
-- @tparam string remotePathToApp Path to application file on remote host
-- @tparam string fileName Name of application executive file
-- @treturn number Return status of application
function RemoteAppUtils.mt.__index:CheckAppStatus(appName)
  -- self.connection:clear()
end

return RemoteAppUtils
