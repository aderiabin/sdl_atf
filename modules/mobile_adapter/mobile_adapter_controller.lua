--- Module which provides Mobile Adapter control functionality on base of mobile connection emulation type
--
-- *Dependencies:* `mobile_adapter/tcp_mobile_adapter`
--
-- *Globals:* none
-- @module MobileAdapterController
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local mobileAdapters = {
  TCP = require('mobile_adapter/tcp_mobile_adapter')
}

local MobileAdapterController = {}

MobileAdapterController.ADAPTER_TYPE = {
  NORMAL = "TCP"
}

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

return MobileAdapterController
