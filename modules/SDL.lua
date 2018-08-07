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
local remote_constants = require('modules/remote/remote_constants')
local SDL = { }

require('atf.util')
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

--- Structure of SDL build options what to be set
local usedBuildOptions = {
  remoteControl =  {
    sdlBuildParameter = "REMOTE_CONTROL",
    defaultValue = "ON"
  },
  extendedPolicy =  {
    sdlBuildParameter = "EXTENDED_POLICY",
    defaultValue = "PROPRIETARY"
  }
}
--- Function to handle the path parameter from ini file to add an ending / if it missed.

local function setPathToSDL()
  local pathToSDL = config.pathToSDL
  if pathToSDL:sub(-1) ~= '/' then
    pathToSDL = pathToSDL .. "/"
    config.pathToSDL = pathToSDL
  end
end

--- Read specified parameter from CMakeCache.txt file
-- @tparam string paramName Parameter to read value
-- @treturn string The main result. Value read of parameter.
-- Can be nil in case parameter was not found.
-- @treturn string Type of read parameter
local function readParameterFromCMakeCacheFile(paramName)
  -- local paramValue, paramType

  -- if config.remoteConnection.enabled then
  --   local result, isexist = ATF.remoteUtils.file:IsFileExists(config.pathToSDL,"CMakeCache.txt")
  --   if result then
  --     if isexist then
  --       local content_res, content = ATF.remoteUtils.file:GetFileContent(config.pathToSDL,"CMakeCache.txt")
  --       if content_res then

  --         local function getLine(s) -- Function needed for Lines parsing at Remote ATF file contents
  --           if s:sub(-1) ~= "\n" then
  --             s = s .. "\n"
  --           end
  --           return s:gmatch("(.-)\n")
  --         end

  --         for line in getLine(content) do
  --           paramType, paramValue = string.match(line, "^%s*" .. paramName .. ":(.+)=(%S*)")
  --           if paramValue then
  --             return paramValue, paramType
  --           end
  --         end

  --       else
  --         error("remote utils unable to get file content of " .. config.pathToSDL .. "CMakeCache.txt")
  --       end
  --     end
  --   else
  --     error("remote utils unable to check for file existens of ".. config.pathToSDL .. "CMakeCache.txt")
  --   end
  -- else
  --   local pathToFile = config.pathToSDL .. "CMakeCache.txt"
  --   if is_file_exists(pathToFile) then
  --     for line in io.lines(pathToFile) do
  --       paramType, paramValue = string.match(line, "^%s*" .. paramName .. ":(.+)=(%S*)")
  --       if paramValue then
  --         return paramValue, paramType
  --       end
  --     end
  --   end
  -- end
  return nil
end

--- Set SDL build option as values of SDL module property
-- @tparam table self Reference to SDL module
-- @tparam string optionName Build option to set value
-- @tparam string sdlBuildParam SDL build parameter to read value
-- @tparam string defaultValue Default value of set option
local function setSdlBuildOption(self, optionName, sdlBuildParam, defaultValue)
  local value, paramType = readParameterFromCMakeCacheFile(sdlBuildParam)
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
  self.buildOptions[optionName] = value
end

--- Set all SDL build options for SDL module of ATF
-- @tparam table self Reference to SDL module
local function setAllSdlBuildOptions(self)
  for option, data in pairs(usedBuildOptions) do
    setSdlBuildOption(self, option, data.sdlBuildParameter, data.defaultValue)
  end
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
      sdl_logger.init_log(get_script_file_name())
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
function SDL:DeleteFile()
  if os.execute ('test -e sdl.pid') then
    os.execute('rm -f sdl.pid')
  end
end

--- Update SDL log4cxx.properties in order SDL will be able to write logs through Telnet
local function updateSDLLogProperties()
  -- if config.storeFullSDLLogs then
  --   local content = nil
  --   local pathToFile = ""

  --   if config.remoteConnection.enabled then
  --     local content_res
  --     content_res, content = ATF.remoteUtils.file:GetFileContent(config.pathToSDL, "log4cxx.properties")
  --     if not content_res then
  --       error("Remote utils: unable to get content of " .. config.pathToSDL .. "log4cxx.properties")
  --     end
  --   else
  --     pathToFile = config.pathToSDL .. "log4cxx.properties"
  --     local f = io.open(pathToFile, "r")
  --     content = f:read("*all")
  --     f:close()
  --   end
  --   local paramsToUpdate = {
  --     {
  --       name = "log4j.rootLogger",
  --       value = "ALL, SmartDeviceLinkCoreLogFile, TelnetLogging"
  --     },
  --     {
  --       name = "log4j.appender.TelnetLogging.layout.ConversionPattern",
  --       value = "%%-5p [%%d{yyyy-MM-dd HH-mm:ss,SSS}][%%t][%%c] %%F:%%L %%M: %%m"
  --     }
  --   }

  --   for _, item in pairs(paramsToUpdate) do
  --     content = string.gsub(content, item.name .. "=.-\n", item.name .. "=" .. item.value .. "\n")
  --   end
  --   if config.remoteConnection.enabled then
  --     ATF.remoteUtils.file:UpdateFileContent(config.pathToSDL, "log4cxx.properties", content)
  --   else
  --     local f = io.open(pathToFile, "w")
  --     f:write(content)
  --     f:close()
  --   end
  -- end
end

setPathToSDL()

setAllSdlBuildOptions(SDL)

updateSDLLogProperties()

return SDL
