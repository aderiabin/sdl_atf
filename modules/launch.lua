--- Script which runs test scripts
--
-- *Dependencies:* `atf.util`
--
-- *Globals:* none
-- @script launch
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

-------------------------------------------------------------
package.cpath = "./lib/lib?.so;./lib/?.so;./lib?.so;" .. package.cpath
package.path = "./modules/?.lua;./modules/atf/stdlib/?.lua;" .. package.path
timestamp = require ("luatime").timestamp
argv = {...}

function arguments()
  print("Command line arguments:")
  for i, v in ipairs(argv) do
    print("[" .. i .. "] = " .. v)
  end
  return argv
end

function quit(code)
  os.exit(code)
end

--------------------------debug------------------------------
function pt(tbl, name)
  print(name .. ": ")
  if not tbl or type(tbl) ~= "table" then 
    print("is not a table")
    return
  end
  for k, v in pairs(tbl) do
    print("[" .. tostring(k) .. "] = " .. tostring(v))
  end
end

local function test()
  local uv = require('luv')

  local ticks = 0
  local aliveTimer = uv.new_timer()
  aliveTimer:start(1000, 1000, function ()
    if ticks < 20 then
      print("UV timer is ticked")
      ticks = ticks + 1
    else
      quit(4)
    end
  end)

  uv.run()
end

-------------------------------------------------------------
local uv = require('luv')
local util = require ("atf.util")

util.commandLine.declare_opt("-c", "--config", util.commandLine.consts.RequiredArgument, "Config folder")
util.commandLine.declare_long_opt("--mobile-connection", util.commandLine.consts.RequiredArgument, "Mobile connection IP")
util.commandLine.declare_long_opt("--mobile-connection-port", util.commandLine.consts.RequiredArgument, "Mobile connection port")
util.commandLine.declare_long_opt("--hmi-connection", util.commandLine.consts.RequiredArgument, "HMI connection IP")
util.commandLine.declare_long_opt("--hmi-connection-port", util.commandLine.consts.RequiredArgument, "HMI connection port")
util.commandLine.declare_long_opt("--perflog-connection", util.commandLine.consts.RequiredArgument, "PerfLog connection IP")
util.commandLine.declare_long_opt("--perflog-connection-port", util.commandLine.consts.RequiredArgument, "Perflog connection port")
util.commandLine.declare_long_opt("--report-path", util.commandLine.consts.RequiredArgument, "Path for a report collecting.")
util.commandLine.declare_long_opt("--report-mark", util.commandLine.consts.RequiredArgument, "Specify label of string for marking test report.")
util.commandLine.declare_long_opt("--storeFullSDLLogs", util.commandLine.consts.NoArgument, "Store Full SDL Logs enable")
util.commandLine.declare_long_opt("--heartbeat", util.commandLine.consts.RequiredArgument, "Hearbeat timeout value")
util.commandLine.declare_long_opt("--sdl-core", util.commandLine.consts.RequiredArgument, "Path to folder with SDL binary")
util.commandLine.declare_long_opt("--report-mark", util.commandLine.consts.RequiredArgument, "Marker of testing report")
util.commandLine.declare_long_opt("--security-protocol", util.commandLine.consts.RequiredArgument, "Security protocol type")
util.commandLine.declare_long_opt("--sdl-interfaces", util.commandLine.consts.RequiredArgument, "Path to folder with APIs")

-- Register SIGINT signal handler
local sigint_signal = uv.new_signal()
sigint_signal:start("sigint", function(signal)
  print("\nGot " .. signal .. ", stopping test execution")
  quit(1)
end)

local script_files = util.commandLine.parse_cmdl()
if (#script_files > 0) then
  for _,scpt in ipairs(script_files) do
    test()
    -- util.runner.print_startscript(scpt)
    -- util.runner.script_execute(scpt)
  end
end

print("---004---")
uv.run()
