local TimersManager = {}

-- Timer
local timer_mt = { __index = {} }

function timer_mt.__index:start(pMsecs)
  if pMsecs then
    self.setInterval(pMsecs)
  end
  local repeatInterval = self.isSingleShot and self.interval or 0
  self.timer:start(self.interval, repeatInterval, self.timeoutHandler)
end

function timer_mt.__index:stop()
  self.timer:stop()
end

function timer_mt.__index:reset()
  self.stop()
  self.start()
end

function timer_mt.__index:setInterval(pMsec)
  if not type(pMsec) == "number" then
    print("Interval of " .. self .. " timer was not set because "
      .. pMsec .. " is not a number")
    return
  end
  self.interval = pMsec
end

function timer_mt.__index:setSingleShot(pIsSingleShot)
  if not type(pIsSingleShot) == "boolean" then
    print("IsSingleShot of " .. self .. " timer was not set because "
      .. pIsSingleShot .. " is not a boolean")
    return
  end
  self.isSingleShot = pIsSingleShot
end

function timer_mt.__index:setTimeoutHandler(pHandlerFunc)
  if not type(pHandlerFunc) == "function" then
    print("Timeout of " .. self .. " timer was not set because "
      .. pHandlerFunc .. " is not a function")
    return
  end
  self.timeoutHandler = pHandlerFunc
end

function timer_mt.__index:close()
  self.timer:close()
end

-- TimersManager
function TimersManager.Timer()
  local res = {
    timer = uv.new_timer(),
    isSingleShot = false,
    interval = 0,
    timeoutHandler = nil
  }
  if res.timer == fail then -- ToDo: investigate fail tuple handling
    print("Fail to create Timer")
    return nil
  end
  setmetatable(res, timer_mt)
  return res
end

return TimersManager
