--- Module which provides utils interface for file management
--
-- *Dependencies:*
--
-- *Globals:* `config`
-- @module FileUtils
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local remote_file_utils = require("remote/remote_file_utils")

--- Type which provides utils interface for local file management
-- @type LocalFileUtils
local LocalFileUtils = {}

--- Get local file content
-- @tparam string filePath Path to file
-- @treturn string Content of file
function LocalFileUtils.GetFileContent(filePath)
    local file = io.open(filePath, "r")
    local content = nil
    if file then
        content = file:read("*all")
        file:close()
    end
    return content
end

--- Create new local file with content
-- @tparam string filePath Path to file
-- @tparam string fileContent Content of file
function LocalFileUtils.CreateFile(filePath, fileContent)
    local file = io.open(filePath, "w")
    if file then
        file:write(fileContent)
        file:close()
    end
end

--- Delete local file
-- @tparam string filePath Path to file
function LocalFileUtils.DeleteFile(filePath)
    os.execute( " rm -f " .. filePath)
end

--- Update local file with content
-- @tparam string filePath Path to file
-- @tparam string fileContent Content of file
function LocalFileUtils.UpdateFileContent(filePath, fileContent)
    local file = io.open(filePath, "w")
    if file then
        file:write(fileContent)
        file:close()
    end
end

--- Check local file existance
-- @tparam string filePath Path to file
-- @treturn boolean Return true in case of file exists
function LocalFileUtils.IsFileExists(filePath)
    local file = io.open(pFile, "r")
    if file == nil then
      return false
    else
      file:close()
      return true
    end
end

--- Backup local file
-- @tparam string filePath Path to file
function LocalFileUtils.BackupFile(filePath)
    os.execute(" cp " .. filePath .. " " .. filePath .. "_origin")
end

--- Restore backuped local file
-- @tparam string filePath Path to file
function LocalFileUtils.RestoreFile(filePath)
    os.execute(" cp " .. filePath .. "_origin " .. filePath)
    os.execute( " rm -f " .. filePath .. "_origin")
end

--- Check folder existance
-- @tparam string pathToFolder Path to folder
-- @treturn boolean Return true in case of folder exists
function LocalFileUtils.IsFolderExists(pathToFolder)
	pathToFolder = pathToFolder .. "/"
    local ok, err, code = os.rename(file, file)
    if not ok then
        if code == 13 then
            -- Permission denied, but it exists
            return true
        end
        return false
    end
    return true
end

--- Create folder
-- @tparam string pathToFolder Path to folder
function LocalFileUtils.CreateFolder(pathToFolder)
    os.execute( "mkdir " .. pathToFolder)
end

--- Delete folder
-- @tparam string pathToFolder Path to folder
function LocalFileUtils.DeleteFolder(pathToFolder)
    os.execute( "rm -rf " .. pathToFolder)
end


local FileUtils = {
    mt = { __index = {} }
}

local function isLocal(isRemoteconnectionEnabled, isForceLocal)
    return isForceLocal or not isRemoteconnectionEnabled
end

local function getPathAndName(pPathToFile)
    local pos = string.find(pPathToFile, "/[^/]*$")
    local path = string.sub(pPathToFile, 1, pos)
    local name = string.sub(pPathToFile, pos + 1)
    return path, name
end

--- Type which provides utils interface for file management
-- @type FileManager

--- Construct instance of RemoteHMIAdapter type
-- @tparam params parameters for Remote adapter
-- @treturn RemoteHMIAdapter Constructed instance
function FileUtils.FileManager(remoteConnection)
    local res = { }
    if remoteConnection then
        res.remoteFileUtils = remote_file_utils.RemoteFileUtils(remoteConnection)
    end
    setmetatable(res, FileUtils.mt)
    return res
  end

--- Check file existance
-- @tparam string filePath Path to file
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
-- @treturn boolean Return true in case of file exists
function FileUtils.mt.__index:IsFileExists(filePath, isForceLocal)
    if isLocal(config.remoteConnection.enabled, isForceLocal) then
        return LocalFileUtils.IsFileExists(filePath)
    end

    local p, n = getPathAndName(pFile)
    local _, isExists = self.remoteFileUtils:IsFileExists(p, n)
    return isExists
end

--- Create new file with content
-- @tparam string filePath Path to file
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
-- @tparam string fileContent Content of file
function FileUtils.mt.__index:CreateFile(filePath, fileContent, isForceLocal)
    if isLocal(config.remoteConnection.enabled, isForceLocal) then
        LocalFileUtils.CreateFile(filePath, fileContent)
        return
    end

    local p, n = getPathAndName(filePath)
    self.remoteFileUtils:CreateFile(p, n)
end

--- Delete file
-- @tparam string filePath Path to file
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
function FileUtils.mt.__index:DeleteFile(filePath, isForceLocal)
    if isLocal(config.remoteConnection.enabled, isForceLocal) then
        LocalFileUtils.DeleteFile(filePath)
        return
    end

    local p, n = getPathAndName(filePath)
    self.remoteFileUtils:DeleteFile(p, n)
end

--- Backup file
-- @tparam string filePath Path to file
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
function FileUtils.mt.__index:BackupFile(filePath, isForceLocal)
    if isLocal(config.remoteConnection.enabled, isForceLocal) then
        LocalFileUtils.BackupFile(filePath)
        return
    end

    local p, n = getPathAndName(filePath)
    self.remoteFileUtils:BackupFile(p, n)
end

--- Restore backuped file
-- @tparam string filePath Path to file
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
function FileUtils.mt.__index:RestoreFile(filePath, isForceLocal)
    if self:IsFileExists(filePath, isForceLocal) then
        if isLocal(config.remoteConnection.enabled, isForceLocal) then
            LocalFileUtils.RestoreFile(filePath)
            return
        end

        local p, n = getPathAndName(filePath)
        self.remoteFileUtils:RestoreFile(p, n)
    end
end

--- Update file with content
-- @tparam string filePath Path to file
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
-- @tparam string fileContent Content of file
function FileUtils.mt.__index:UpdateFileContent(filePath, fileContent, isForceLocal)
    if isLocal(config.remoteConnection.enabled, isForceLocal) then
        LocalFileUtils.UpdateFileContent(filePath, fileContent)
        return
    end

    local p, n = getPathAndName(filePath)
    self.remoteFileUtils:UpdateFileContent(p, n, fileContent)
end

--- Get file content
-- @tparam string filePath Path to file
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
-- @treturn string Content of file
function FileUtils.mt.__index:GetFileContent(filePath, isForceLocal)
    if isLocal(config.remoteConnection.enabled, isForceLocal) then
        return LocalFileUtils.GetFileContent(filePath)
    end

    local p, n = getPathAndName(filePath)
    local _, isExists = self.remoteFileUtils:IsFileExists(p, n)
    if isExists then
        local _, path = self.remoteFileUtils:GetFile(p, n)
        filePath = path
    else
        return nil
    end
    local content = LocalFileUtils.GetFileContent(filePath)
    LocalFileUtils.DeleteFile(filePath)
    return content
end

--- Check folder existance
-- @tparam string pathToFolder Path to folder
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
-- @treturn boolean Return true in case of folder exists
function FileUtils.mt.__index:IsFolderExists(pathToFolder, isForceLocal)
    if isLocal(config.remoteConnection.enabled, isForceLocal) then
        LocalFileUtils.IsFolderExists(pathToFolder)
        return
    end

    self.remoteFileUtils:IsFolderExists(pathToFolder)
end

--- Create folder
-- @tparam string pathToFolder Path to folder
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
function FileUtils.mt.__index:CreateFolder(pathToFolder, isForceLocal)
    if isLocal(config.remoteConnection.enabled, isForceLocal) then
        LocalFileUtils.CreateFolder(pathToFolder)
        return
    end

    self.remoteFileUtils:CreateFolder(pathToFolder)
end

--- Delete folder
-- @tparam string pathToFolder Path to folder
-- @tparam boolean isForceLocal Force to perform action on ATF host file system
function FileUtils.mt.__index:DeleteFolder(pathToFolder, isForceLocal)
    if isLocal(config.remoteConnection.enabled, isForceLocal) then
        LocalFileUtils.DeleteFolder(pathToFolder)
        return
    end

    self.remoteFileUtils:DeleteFolder(pathToFolder)
end

return FileUtils
