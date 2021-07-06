--- Module which provides constants for mobile device related modules
--
-- *Dependencies:* none
--
-- *Globals:* none
-- @module mobile_device.mobile_device_constants
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local MobileDeviceConstants = {}

MobileDeviceConstants.OS_TYPE = {
    UBUNTU = "Ubuntu",
    ANDROID_4_0 = "Android 4.0",
    IOS_4 = "iOS 4"
}

MobileDeviceConstants.TRANSPORT_TYPE = {
    TCP = "TCP",
    WIFI = "WiFi",
    BLUETOOTH = "Bluetooth",
    USB = "USB"
}

return MobileDeviceConstants
