--- Module which provides utils interface for file management on remote host
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @module RemoteFileUtils
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local RemoteFileUtils = {
	mt = { __index = {} }
  }

--- Module which provides utils interface for file management on remote host
-- @type RemoteFileUtils

--- Construct instance of RemoteFileUtils type
-- @tparam RemoteConnection connection RemoteConnection instance
-- @treturn RemoteFileUtils Constructed instance
function RemoteFileUtils.RemoteFileUtils(connection)
  local res = { }
  res.connection = connection:GetConnection()
  setmetatable(res, RemoteFileUtils.mt)
  return res
end

--- Check connection
-- @treturn boolean Return true in case connection is active
function RemoteFileUtils.mt.__index:Connected()
  return self.connection:connected()
end

--- Check file existance on remote host
-- @tparam string remotePathToFile Path to file on remote host
-- @tparam string fileName Name of file
-- @treturn boolean Return true in case of file exists
function RemoteFileUtils.mt.__index:IsFileExists(remotePathToFile, fileName)
  -- self.connection:clear()
end

--- Create new file with content on remote host
-- @tparam string remotePathToFile Path to file on remote host
-- @tparam string fileName Name of file
-- @tparam string fileContent Content of file
-- @treturn boolean Return true in case of success
function RemoteFileUtils.mt.__index:CreateFile(remotePathToFile, fileName, fileContent)
	if self:IsFileExists(remotePathToFile, fileName) then
		return false
	end
	return self:UpdateFileContent(remotePathToFile, fileName, fileContent)
end

--- Delete file on remote host
-- @tparam string remotePathToFile Path to file on remote host
-- @tparam string fileName Name of file
-- @treturn boolean Return true in case of success
function RemoteFileUtils.mt.__index:DeleteFile(remotePathToFile, fileName)
  -- self.connection:clear()
end

--- Backup file on remote host
-- @tparam string remotePathToFile Path to file on remote host
-- @tparam string fileName Name of file
-- @treturn boolean Return true in case of success
function RemoteFileUtils.mt.__index:BackupFile(remotePathToFile, fileName)
  -- self.connection:clear()
end

--- Restore backuped file on remote host
-- @tparam string remotePathToFile Path to file on remote host
-- @tparam string fileName Name of file
-- @treturn boolean Return true in case of success
function RemoteFileUtils.mt.__index:RestoreFile(remotePathToFile, fileName)
  -- self.connection:clear()
end

--- Update file with content on remote host
-- @tparam string remotePathToFile Path to file on remote host
-- @tparam string fileName Name of file
-- @tparam string fileContent Content of file
-- @treturn boolean Return true in case of success
function RemoteFileUtils.mt.__index:UpdateFileContent(remotePathToFile, fileName, fileContent)
  -- self.connection:clear()
end

--- Get file content from remote host
-- @tparam string remotePathToFile Path to file on remote host
-- @tparam string fileName Name of file
-- @treturn boolean Return true in case of success
-- @treturn string Content of file
function RemoteFileUtils.mt.__index:GetFileContent(remotePathToFile, fileName)
  -- self.connection:clear()
end

--- Check folder existance on remote host
-- @tparam string remotePathToFolder Path to folder on remote host
-- @tparam string folderName Name of folder
-- @treturn boolean Return true in case of folder exists
function RemoteFileUtils.mt.__index:IsFolderExists(remotePathToFolder, folderName)
  -- self.connection:clear()
end

--- Create folder on remote host
-- @tparam string remotePathToFolder Path to folder on remote host
-- @tparam string folderName Name of folder
-- @treturn boolean Return true in case of success
function RemoteFileUtils.mt.__index:CreateFolder(remotePathToFolder, folderName)
  -- self.connection:clear()
end

--- Delete folder on remote host
-- @tparam string remotePathToFolder Path to folder on remote host
-- @tparam string folderName Name of folder
-- @treturn boolean Return true in case of success
function RemoteFileUtils.mt.__index:DeleteFolder(remotePathToFolder, folderName)
  -- self.connection:clear()
end

return RemoteFileUtils
