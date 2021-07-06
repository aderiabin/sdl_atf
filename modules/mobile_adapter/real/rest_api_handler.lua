--- Module which provides transport level interface for communication using REST over HTTP protocol
--
-- *Dependencies:* `http_connection`, `json`
--
-- *Globals:*
-- @module rest_api_handler
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local json = require("json")
local http_connection = require('mobile_adapter/real/http_connection')

local RestApiHandler = { mt = { __index = {} } }

--- Type which provides transport level interface for communication using REST over HTTP protocol
-- @type RestHandler

--- Construct instance of RestHandler type
-- @tparam string host server host address
-- @tparam string port server port
-- @tparam table api list of REST API functions of server
-- @treturn RestHandler Constructed instance
function RestApiHandler.RestHandler(host, port, api)
  local res = {
      api = api
  }
  res.connection = http_connection.Connection({ host = host, port = port })
  setmetatable(res, RestApiHandler.mt)
  return res
end

local function buildAPIString(api, apiName, params)
  local paramsString = ""
  if type(params) == "table" then
    local isFirstParam = true
    for paramIndex, paramValue in ipairs(params) do
      if isFirstParam then
        paramsString = paramsString .. "?"
        isFirstParam = false
      else
        paramsString = paramsString .. "&"
      end
      paramsString = paramsString .. api[apiName].params[paramIndex] .. "=" .. paramValue
    end
  end
  return apiName .. paramsString
end

local function parseResponseData(responseData)
  if responseData then
    return json.decode(responseData)
  end
  return { }
end

--- Send http request and receive response
-- @tparam table params Parameters for request to be sent
-- @treturn table Parsed response JSON
function RestApiHandler.mt.__index:Call(apiName, params)
  local responseData = self.connection:SendGetRequest(buildAPIString(self.api, apiName, params))
  return parseResponseData(responseData)
end

return RestApiHandler
