local config = require("config")
local ATF = require("ATF")

local isRemote = config.remoteConnection.enabled

--[[ Local Funcions ]]

local function getPathAndName(pPathToFile)
  local pos = string.find(pPathToFile,"/[^/]*$")
  local path = string.sub(pPathToFile, 1, pos)
  local name = string.sub(pPathToFile, pos + 1)
  return path, name
end

local function getFileContent(pPathToFile)
  if isRemote then
    local p, n = getPathAndName(pPathToFile)
    local _, isExist = ATF.remoteUtils.file:IsFileExists(p, n)
    if isExist then
      local _, path = ATF.remoteUtils.file:GetFileContent(p, n)
      pPathToFile = path
    else
      return nil
    end
  end
  local f = io.open(pPathToFile, "r")
  local content = f:read("*all")
  f:close()
  return content
end

local function saveFileContent(pPathToFile, pContent)
  if isRemote then
    local p, n = getPathAndName(pPathToFile)
    local _ = ATF.remoteUtils.file:UpdateFileContent(p, n, pContent)
  else
    local f = io.open(pPathToFile, "w")
    f:write(pContent)
    f:close()
  end
end

--[[ Module ]]
local SDLUtils = {}

function SDLUtils.addSlashToPath(pPath)
  if pPath == nil or string.len(pPath) == 0 then return end
  if pPath:sub(-1) ~= '/' then
    pPath = pPath .. "/"
  end
  return pPath
end

function SDLUtils.getPath(pPath)
   if pPath:sub(1,1) ~= "/" then
     pPath = config.pathToSDL .. pPath
  end
  return SDLUtils.addSlashToPath(pPath)
end

SDLUtils.BuildOptions = {}
SDLUtils.BuildOptions.fileName = "build_config.txt"

function SDLUtils.BuildOptions.get(pParam)
  local path = SDLUtils.addSlashToPath(config.pathToSDL)
  local filePapth = path .. SDLUtils.BuildOptions.fileName
  local content = getFileContent(filePapth)
  if content then
    for line in content:gmatch("[^\r\n]+") do
      local pType, pValue = string.match(line, "^%s*" .. pParam .. ":(.+)=(%S*)")
      if pValue then
        return pValue, pType
      end
    end
  end
  return nil
end

function SDLUtils.BuildOptions.set()
  -- Useless for now
end



SDLUtils.INI = {}
SDLUtils.INI.fileName = "smartDeviceLink.ini"

function SDLUtils.INI.get(pParam)
  local iniPath = SDLUtils.addSlashToPath(config.pathToSDLConfig)
  local iniFilePath = iniPath .. SDLUtils.INI.fileName
  local content = getFileContent(iniFilePath)
  local value
  for line in content:gmatch("[^\r\n]+") do
    if string.match(line, "^%s*" .. pParam .. "%s*=%s*") ~= nil then
      if string.find(line, "%s*=%s*$") ~= nil then
        value = ""
        break
      end
      local b, e = string.find(line, "%s*=%s*.")
      if b ~= nil then
        value = string.sub(line, e, string.len(line))
        break
      end
    end
  end
  return value
end

function SDLUtils.INI.set(pParam, pValue)
end



SDLUtils.LOG = {}

function SDLUtils.LOG.get(pParam)
end

function SDLUtils.LOG.set(pParam, pValue)
  local logFilePath
  if isRemote then
    logFilePath = SDLUtils.INI.get("LoggerConfigFile")
  else
    logFilePath = config.pathToSDLConfig .. "log4cxx.properties"
  end
  local content = getFileContent(logFilePath)
  content = string.gsub(content, pParam .. "=.-\n", pParam .. "=" .. pValue .. "\n")
  saveFileContent(logFilePath, content)
end

SDLUtils.PPT = {}

function SDLUtils.PPT.get()
end

function SDLUtils.PPT.set(pPPT)
end

function SDLUtils.PPT.update(pUpdateFunc)
end

return SDLUtils
