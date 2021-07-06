--- Module which provides transport adapter level interface for communication with SDL through real mobile device
--
-- *Dependencies:* `real_device_communication_handler`
--
-- *Globals:*
-- @module real_device_tcp_transport_adapter
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local real_device_communication_handler = require('mobile_adapter/real/real_device_communication_handler')

local RealDeviceTcpTransportAdapter = { mt = { __index = {} } }

--- Type which provides transport adapter level interface for communication with SDL through real mobile device
-- @type Connection

--- Construct instance of Connection type
-- @tparam string host mobile proxy device host address
-- @tparam string port mobile proxy device port
-- @tparam MobileDeviceConstants.TRANSPORT_TYPE transport mobile proxy device transport for SDL connection
-- @treturn Connection Constructed instance
function RealDeviceTcpTransportAdapter.Connection(params)
  local res = {
    url = params.url,
    port = params.port,
    transport = params.transport
  }
  res.controlConnection = real_device_communication_handler.CommunicationHandler(res.url, res.port, res.transport)
  res.dataConnection = nil
  setmetatable(res, RealDeviceTcpTransportAdapter.mt)
  return res
end

--- Connect with SDL through real device transport interface
function RealDeviceTcpTransportAdapter.mt.__index:Connect()
  self.dataConnection = self.controlConnection:ConnectToSDL()
  self.dataConnection:Connect()
end

--- Send pack of messages from mobile to SDL
-- @tparam table data Data to be sent
function RealDeviceTcpTransportAdapter.mt.__index:Send(data)
  self.dataConnection:Send(data)
end

--- Set handler for OnInputData
-- @tparam function func Handler function
function RealDeviceTcpTransportAdapter.mt.__index:OnInputData(func)
  self.dataConnection:OnInputData(func)
end

--- Set handler for OnDataSent
-- @tparam function func Handler function
function RealDeviceTcpTransportAdapter.mt.__index:OnDataSent(func)
  self.dataConnection:OnDataSent(func)
end

--- Set handler for OnConnected
-- @tparam function func Handler function
function RealDeviceTcpTransportAdapter.mt.__index:OnConnected(func)
  self.dataConnection:OnConnected(func)
end

--- Set handler for OnDisconnected
-- @tparam function func Handler function
function RealDeviceTcpTransportAdapter.mt.__index:OnDisconnected(func)
  self.dataConnection:OnDisconnected(func)
end

--- Close connection
function RealDeviceTcpTransportAdapter.mt.__index:Close()
  self.controlConnection:DisconnectFromSDL()
end

return RealDeviceTcpTransportAdapter
