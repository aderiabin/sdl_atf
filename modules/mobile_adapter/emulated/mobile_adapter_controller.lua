--- Module which provides Mobile Adapter control functionality on base of mobile connection emulation type
--
-- *Dependencies:* `mobile_adapter/emulated/tcp_mobile_adapter`, `mobile_adapter/websocket_mobile_adapter`
--
-- *Globals:* none
-- @module MobileAdapterController
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local mobileAdapters = {
  TCP = require('mobile_adapter/emulated/tcp_mobile_adapter'),
  WS = require('mobile_adapter/emulated/websocket_mobile_adapter'),
  WSS = require('mobile_adapter/emulated/websocket_mobile_adapter'),
  WIFI = require('mobile_adapter/real/real_device_tcp_transport_adapter'),
  BT = require('mobile_adapter/real/real_device_tcp_transport_adapter'),
  USB = require('mobile_adapter/real/real_device_tcp_transport_adapter')
}

local MobileAdapterController = {}

--- Provide Mobile adapter instance on base of adapter type
-- @tparam string adapterType Instance of ADAPTER_TYPE enumeration
-- @tparam table adapterParams Specific parameters for mobile adapter type
-- @treturn table mobile adapter instance
function MobileAdapterController.getAdapter(adapterType, adapterParams)
  if adapterType and mobileAdapters[adapterType] then
    return mobileAdapters[adapterType].Connection(adapterParams)
  end
  error("Incorrect mobile adapter type '".. tostring(adapterType)
    .. "' was passed to MobileAdapterController.getAdapter method")
end

function MobileAdapterController.getDefaultAdapterParameters()
  local mobileAdapterType = config.defaultMobileAdapterType
  print("Default mobile device transport: " .. tostring(mobileAdapterType))
  local mobileAdapterParameters
  if mobileAdapterType == "TCP" then
    mobileAdapterParameters = {
      host = config.remoteConnection.enabled and config.remoteConnection.url or config.mobileHost,
      port = config.mobilePort
    }
  elseif mobileAdapterType == "WS" then
    mobileAdapterParameters = {
      url = config.wsMobileURL,
      port = config.wsMobilePort
    }
  elseif mobileAdapterType == "WSS" then
    mobileAdapterParameters = {
      url = config.wssMobileURL,
      port = config.wssMobilePort,
      sslProtocol = config.wssSecurityProtocol,
      sslCypherListString = config.wssCypherListString,
      sslCaCertPath = config.wssCertificateCAPath,
      sslCertPath = config.wssCertificateClientPath,
      sslKeyPath = config.wssPrivateKeyPath
    }
  elseif mobileAdapterType == "WIFI" then
    mobileAdapterParameters = {
      --ToDo: Add parameters
    }
  elseif mobileAdapterType == "BT" then
    mobileAdapterParameters = {
      --ToDo: Add parameters
    }
  elseif mobileAdapterType == "USB" then
    mobileAdapterParameters = {
      --ToDo: Add parameters
    }
  else error("Unknown default mobile adapter type: " .. tostring(mobileAdapterType))
  end
  return mobileAdapterParameters
end

return MobileAdapterController
