--- Module which provides transport level interface for communication with SDL through real mobile device
--
-- *Dependencies:* `tcp_connection`, `rest_api_handler`
--
-- *Globals:*
-- @module real_device_communication_handler
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local tcp_connection = require('mobile_adapter/emulated/tcp_mobile_adapter')
local rest_api_handler = require('mobile_adapter/real/rest_api_handler')

local RealDeviceCommunicationHandler = { mt = { __index = {} } }

local function isResponseSucces(response, warningMessage)
    if not response.success then
        -- ToDo: Perform print message using ATF print capabilities
        print(warningMessage)
        return false
    end
    return true
end

--- Type which provides transport level interface for communication with SDL through real mobile device
-- @type CommunicationHandler

--- Construct instance of CommunicationHandler type
-- @tparam string host mobile proxy device host address
-- @tparam string port mobile proxy device port
-- @treturn CommunicationHandler Constructed instance
function RealDeviceCommunicationHandler.CommunicationHandler(host, port, transport)
  local res = {
    host = host,
    transport = transport
  }

  local api = {
    GetDeviceInfo = { params = { } },
    ConnectToSDL = { params = { "transport", "sdl_host", "sdl_port" } },
    DisconnectFromSDL = { params = { "id" } },
    GetSDLConnectionStatus = { params = { "id" } }
  }

  res.controlConnection = rest_api_handler.RestHandler(host, port, api)
  setmetatable(res, RealDeviceCommunicationHandler.mt)
  return res
end

--- Send request to connect to SDL through real device transport interface
function RealDeviceCommunicationHandler.mt.__index:ConnectToSDL()
  local response = self.controlConnection:Call("ConnectToSDL", { self.transport })
  local msg = "Try to connect to SDL through real device " .. tostring(self.host)
    .. " and transport " .. tostring(self.transport) .. " has failed"
  if isResponseSucces(response, msg) then
    return tcp_connection.Connection({ host = response.host, port = response.port })
  end
  return nil
end

--- Send request to disconnect from SDL through real device transport interface
function RealDeviceCommunicationHandler.mt.__index:DisconnectFromSDL()
  local response = self.controlConnection:Call("DisconnectFromSDL", { self.transport })
  local msg = "Try to disconnect from SDL through real device " .. tostring(self.host)
    .. " and transport " .. tostring(self.transport) .. " has failed"
  return isResponseSucces(response, msg)
end

--- Send request to get information about proxy device
function RealDeviceCommunicationHandler.mt.__index:GetDeviceInfo()
  local response = self.controlConnection:Call("GetDeviceInfo")
  local msg = "Try to get info about proxy device " .. tostring(self.host) .. " has failed"
  if isResponseSucces(response, msg) then
      response.success = nil
      return response
  end
  return nil
end

return RealDeviceCommunicationHandler
