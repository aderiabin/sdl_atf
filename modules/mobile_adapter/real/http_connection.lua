--- Module which provides transport level interface for communication using HTTP protocol
--
-- *Dependencies:*
--
-- *Globals:* `network`
-- @module http_connection
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local Http = { mt = { __index = {} } }

--- Type which provides transport level interface for communication using HTTP protocol
-- @type Connection

--- Construct instance of RestHandler type
-- @tparam string host server host address
-- @tparam string port server port
-- @treturn Connection Constructed instance
function Http.Connection(params)
  local res = {
    host = params.host,
    port = params.port
  }
  res.connection = network.HttpClient()
  setmetatable(res, Http.mt)
  return res
end

local function handleError(error, requestString)
  -- ToDo: Perform print message using ATF print capabilities
  print("Request " .. requestString .. " returned error: " .. error)
end

--- Send http request and receive response
-- @tparam table params Parameters for request to be sent
-- @treturn string Result data
function Http.mt.__index:SendGetRequest(data)
  data = data or ""
  local requestString = "http://" .. tostring(self.host) .. ":" .. tostring(self.port) .. "/" .. data
  local response, error  = self.connection:get(requestString)
  if error then
      handleError(error, requestString)
  end
  return response
end

return Http
