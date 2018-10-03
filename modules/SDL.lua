--- The module which is responsible for managing SDL from ATF
--
-- *Dependencies:* `os`, `sdl_logger`, `config`, `atf.util`
--
-- *Globals:* `sleep()`, `CopyFile()`, `CopyInterface()`, `xmlReporter`, `console`, `ATF`
-- @module SDL
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

require('os')
local sdl_logger = require('sdl_logger')
local config = require('config')
local console = require('console')
local util = require ("atf.util")
local remote_constants = require('modules/remote/remote_constants')
local ATF = require("ATF")
local json = require("modules/json")
require('atf.util')

--[[ Module ]]
local SDL = { }

--- Table of SDL build options
SDL.buildOptions = {}
--- The flag responsible for stopping ATF in case of emergency completion of SDL
SDL.exitOnCrash = true
--- SDL state constant: SDL completed correctly
SDL.STOPPED = 0
--- SDL state constant: SDL works
SDL.RUNNING = 1
--- SDL state constant: SDL crashed
SDL.CRASH = -1
--- SDL state constant: SDL in idle mode (for remote only)
SDL.IDLE = -2

--[[ Local Functions ]]
--- Update SDL logger config in order SDL will be able to write logs through Telnet
local function updateSDLLogProperties()
  if config.storeFullSDLLogs then
    local paramsToUpdate = {
      {
        name = "log4j.rootLogger",
        value = "ALL, TelnetLogging"
      },
      {
        name = "log4j.appender.TelnetLogging.layout.ConversionPattern",
        value = "%%-5p [%%d{yyyy-MM-dd HH:mm:ss,SSS}][%%t][%%c] %%F:%%L %%M: %%m"
      }
    }
    for _, item in pairs(paramsToUpdate) do
      SDL.LOGGER.set(item.name, item.value)
    end
  end
end

local function getPathAndName(pPathToFile)
  local pos = string.find(pPathToFile,"/[^/]*$")
  local path = string.sub(pPathToFile, 1, pos)
  local name = string.sub(pPathToFile, pos + 1)
  return path, name
end

local function getFileContent(pPathToFile)
  if config.remoteConnection.enabled then
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
  if config.remoteConnection.enabled then
    local p, n = getPathAndName(pPathToFile)
    local _ = ATF.remoteUtils.file:UpdateFileContent(p, n, pContent)
  else
    local f = io.open(pPathToFile, "w")
    f:write(pContent)
    f:close()
  end
end

local function deleteFile(pPathToFile)
  if config.remoteConnection.enabled then
    local p, n = getPathAndName(pPathToFile)
    local _ = ATF.remoteUtils.file:DeleteFile(p, n)
  else
    os.execute( "rm -f " .. pPathToFile)
  end
end

local function getParamValue(pContent, pParam)
  local value
  for line in pContent:gmatch("[^\r\n]+") do
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

local function setParamValue(pContent, pParam, pValue)
  local out = ""
  for line in pContent:gmatch("[^\r\n]+") do
    local ptrn = "^%s*".. pParam .. "%s*=.*"
    if string.find(line, ptrn) then
      line = string.gsub(line, ptrn, pParam .. "=" .. tostring(pValue))
    end
    out = out .. line .. "\n"
  end
  return out:sub(1, -2)
end

local function backup(pFilePath)
  local content = getFileContent(pFilePath)
  saveFileContent(pFilePath .. "_origin", content)
end

local function restore(pFilePath)
  local content = getFileContent(pFilePath .. "_origin")
  saveFileContent(pFilePath, content)
  deleteFile(pFilePath .. "_origin")
end

--- Structure of SDL build options what to be set
local function getDefaultBuildOptions()
  local options = { }
  options.remoteControl = { sdlBuildParameter = "REMOTE_CONTROL", defaultValue = "ON" }
  if config.remoteConnection.enabled then
    options.extendedPolicy = { sdlBuildParameter = "EXTENDED_POLICY", defaultValue = "EXTERNAL_PROPRIETARY" }
  else
    options.extendedPolicy = { sdlBuildParameter = "EXTENDED_POLICY", defaultValue = "PROPRIETARY" }
  end
  return options
end

--- Set SDL build option as values of SDL module property
-- @tparam string optionName Build option to set value
-- @tparam string sdlBuildParam SDL build parameter to read value
-- @tparam string defaultValue Default value of set option
local function setSdlBuildOption(optionName, sdlBuildParam, defaultValue)
  local value, paramType = SDL.BuildOptions.get(sdlBuildParam)
  if value == nil then
    value = defaultValue
    local msg = "SDL build option " ..
      sdlBuildParam .. " is unavailable.\nAssume that SDL was built with " ..
      sdlBuildParam .. " = " .. defaultValue
    print(console.setattr(msg, "cyan", 1))
  else
    if paramType == "UNINITIALIZED" then
      value = nil
      local msg = "SDL build option " ..
        sdlBuildParam .. " is unsupported."
      print(console.setattr(msg, "cyan", 1))
    end
  end
  SDL.buildOptions[optionName] = value
end

--- Set all SDL build options for SDL module of ATF
-- @tparam table self Reference to SDL module
local function setAllSdlBuildOptions()
  for option, data in pairs(getDefaultBuildOptions()) do
    setSdlBuildOption(option, data.sdlBuildParameter, data.defaultValue)
  end
end

function SDL.addSlashToPath(pPath)
  if pPath == nil or string.len(pPath) == 0 then return end
  if pPath:sub(-1) ~= '/' then
    pPath = pPath .. "/"
  end
  return pPath
end

SDL.BuildOptions = {}

function SDL.BuildOptions.file()
  return config.pathToSDL .. "build_config.txt"
end

function SDL.BuildOptions.get(pParam)
  local content = getFileContent(SDL.BuildOptions.file())
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

function SDL.BuildOptions.set()
  -- Useless for now
end

SDL.INI = {}

function SDL.INI.file()
  return config.pathToSDLConfig .. "smartDeviceLink.ini"
end

function SDL.INI.get(pParam)
  local content = getFileContent(SDL.INI.file())
  return getParamValue(content, pParam)
end

function SDL.INI.set(pParam, pValue)
  local content = getFileContent(SDL.INI.file())
  content = setParamValue(content, pParam, pValue)
  saveFileContent(SDL.INI.file(), content)
end

function SDL.INI.backup()
  backup(SDL.INI.file())
end

function SDL.INI.restore()
  restore(SDL.INI.file())
end

SDL.LOGGER = {}

function SDL.LOGGER.file()
  local filePath
  if config.remoteConnection.enabled then
    filePath = SDL.INI.get("LoggerConfigFile")
  else
    filePath = config.pathToSDLConfig .. "log4cxx.properties"
  end
  return filePath
end

function SDL.LOGGER.get(pParam)
  local content = getFileContent(SDL.LOGGER.file())
  return getParamValue(content, pParam)
end

function SDL.LOGGER.set(pParam, pValue)
  local content = getFileContent(SDL.LOGGER.file())
  content = setParamValue(content, pParam, pValue)
  saveFileContent(SDL.LOGGER.file(), content)
end

function SDL.LOGGER.backup()
  backup(SDL.LOGGER.file())
end

function SDL.LOGGER.restore()
  restore(SDL.LOGGER.file())
end

SDL.PreloadedPT = {}

function SDL.PreloadedPT.file()
  return config.pathToSDLConfig .. SDL.INI.get("PreloadedPT")
end

function SDL.PreloadedPT.get()
  local content = getFileContent(SDL.PreloadedPT.file())
  return json.decode(content)
end

function SDL.PreloadedPT.set(pPPT)
  local content = json.encode(pPPT)
  saveFileContent(SDL.PreloadedPT.file(), content)
end

function SDL.PreloadedPT.backup()
  backup(SDL.PreloadedPT.file())
end

function SDL.PreloadedPT.restore()
  restore(SDL.PreloadedPT.file())
end

--- A global function for organizing execution delays (using the OS)
-- @tparam number n The delay in ms
function sleep(n)
  os.execute("sleep " .. tonumber(n))
end

--- Launch SDL from ATF
-- @tparam string pathToSDL Path to SDL
-- @tparam string smartDeviceLinkCore The name of the SDL to run
-- @tparam boolean ExitOnCrash Flag whether Stop ATF in case SDL shutdown
-- @treturn boolean The main result. Indicates whether the launch of SDL was successful
-- @treturn string Additional information on the main SDL startup result
function SDL:StartSDL(pathToSDL, smartDeviceLinkCore, ExitOnCrash)
  local result
  if ExitOnCrash ~= nil then
    self.exitOnCrash = ExitOnCrash
  end
  local status = self:CheckStatusSDL()

  if (status == self.RUNNING) then
    local msg = "SDL had already started out of ATF"
    xmlReporter.AddMessage("StartSDL", {["message"] = msg})
    print(console.setattr(msg, "cyan", 1))
    return false, msg
  end
  if config.remoteConnection.enabled then
     result = ATF.remoteUtils.app:StartApp(pathToSDL, smartDeviceLinkCore)
  else
     result = os.execute ('./tools/StartSDL.sh ' .. pathToSDL .. ' ' .. smartDeviceLinkCore)
  end

  local msg
  if result then
    msg = "SDL started"
    if config.storeFullSDLLogs == true then
      sdl_logger.init_log(util.runner.get_script_file_name())
    end
  else
    msg = "SDL had already started not from ATF or unexpectedly crashed"
    print(console.setattr(msg, "cyan", 1))
  end
  xmlReporter.AddMessage("StartSDL", {["message"] = msg})
  return result, msg
end

--- Stop SDL from ATF (SIGINT is used)
-- @treturn nil The main result. Always nil.
-- @treturn string Additional information on the main result of stopping SDL
function SDL:StopSDL()
  self.autoStarted = false
  local status = self:CheckStatusSDL()
  if status == self.RUNNING or status == self.IDLE then
    if config.remoteConnection.enabled then
      ATF.remoteUtils.app:StopApp(config.SDL)
    else
      os.execute('./tools/StopSDL.sh')
    end
  else
    local msg = "SDL had already stopped"
    xmlReporter.AddMessage("StopSDL", {["message"] = msg})
    print(console.setattr(msg, "cyan", 1))
  end
  if config.storeFullSDLLogs == true then
    sdl_logger.close()
  end
  sleep(1)
end

--- SDL status check
-- @treturn number SDL state
--
-- SDL.STOPPED = 0 Completed the work correctly
--
-- SDL.RUNNING = 1 Running
--
-- SDL.CRASH = -1 Crash
--
-- SDL.IDLE = -2 Idle (for remote only)
function SDL:CheckStatusSDL()
  if config.remoteConnection.enabled then
    local result, data = ATF.remoteUtils.app:CheckAppStatus(config.SDL)
    if result then
      if data == remote_constants.APPLICATION_STATUS.IDLE then
        return self.IDLE
      elseif data == remote_constants.APPLICATION_STATUS.NOT_RUNNING then
        return self.STOPPED
      elseif data == remote_constants.APPLICATION_STATUS.RUNNING then
        return self.RUNNING
      end
    else
      error("Remote utils: unable to get Appstatus of SDL")
    end
  else
    local testFile = os.execute ('test -e sdl.pid')
    if testFile then
      local testCatFile = os.execute ('test -e /proc/$(cat sdl.pid)')
      if not testCatFile then
        return self.CRASH
      end
      return self.RUNNING
    end
    return self.STOPPED
  end
end

--- Deleting an SDL process indicator file
function SDL.DeleteFile()
  if os.execute ('test -e sdl.pid') then
    os.execute('rm -f sdl.pid')
  end
end

config.pathToSDL = SDL.addSlashToPath(config.pathToSDL)
config.pathToSDLConfig = SDL.addSlashToPath(config.pathToSDLConfig)

setAllSdlBuildOptions()

updateSDLLogProperties()

return SDL
