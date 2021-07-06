--- Module which provides mobile devices management interface
--
-- *Dependencies:* `mobile_adapter_controller`, `mobile_connection`, `file_connection`
--
-- *Globals:* `config`, `timers`, qt, event_dispatcher
-- @module mobile_device_manager
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local mobile_adapter_controller = require('mobile_adapter/emulated/mobile_adapter_controller')
local emulated_device_transport_adapter = require('mobile_adapter/emulated/tcp_mobile_adapter')
local real_device_transport_adapter = require('mobile_adapter/real/real_device_tcp_transport_adapter')
local real_device_communication_handler = require('modules/mobile_adapter/real/real_device_communication_handler')
local mobile_connection = require('mobile_connection')
local file_connection = require('file_connection')

-- ToDo: Add LDoc for current module
local MobileDeviceManager = { }

-- local DeviceInfo = {
-- 	id = nil,
-- 	host = nil,
-- 	port = nil,
-- 	isRealDevice = nil,
-- 	macAddress = nil,
-- 	osType = nil,
-- 	transportType = nil
-- }

local availableDevicesList = { }

-- ToDo: Move this function into ATF util
-- local function splitString(str, inSplitPattern, outResults )
-- 	if not outResults then
-- 	  outResults = { }
-- 	end
-- 	local theStart = 1
-- 	local theSplitStart, theSplitEnd = string.find(str, inSplitPattern, theStart)
-- 	while theSplitStart do
-- 	  table.insert( outResults, string.sub(str, theStart, theSplitStart - 1))
-- 	  theStart = theSplitEnd + 1
-- 	  theSplitStart, theSplitEnd = string.find(str, inSplitPattern, theStart)
-- 	end
-- 	table.insert( outResults, string.sub(str, theStart ) )
-- 	return outResults
-- end

-- ToDo: Move this function into platform specific utils module
local function getDeviceMAC(host, port)
  local cmd = "echo -n " .. host .. ":" .. port .. " | sha256sum | awk '{printf $1}'"
  local handle = io.popen(cmd)
  local result = handle:read("*a")
  handle:close()
  return result
end

-- ToDo: Move this function into platform specific utils module
	local function addFakeNetworkInterface(name, host)
		os.execute("ifconfig lo:" .. name .." " .. host)
	end

-- ToDo: Move this function into platform specific utils module
local function removeFakeNetworkInterface(name)
	os.execute("ifconfig lo:" .. name .." down")
end

local function buildDeviceId(deviceHost, deviceTransportType)
	local hostName = string.gsub(deviceHost, "%.", "")
	return hostName .. deviceTransportType
end

local function buildFileName(deviceName)
	return "mobile_" .. deviceName .. ".out"
end

local function isDeviceSuitable(availableDevice, deviceInfo)
	for k, v in pairs(deviceInfo) do
		if availableDevice[k] ~= v then
			return false
		end
  end
  return true
end

local function getSuitableDevicesList(deviceInfo)
	local suitableDevicesList = {}
	for idx, availableDevice in ipairs(availableDevicesList) do
		if isDeviceSuitable(availableDevice, deviceInfo) then
			table.insert(suitableDevicesList, { idx = idx, deviceInfo = availableDevice })
		end
	end
	return suitableDevicesList
end

local function isDeviceInfoCorrect(deviceInfo)
	return next(deviceInfo) ~= nil
		and deviceInfo.id ~= nil
		and deviceInfo.macAddress ~= nil
		and deviceInfo.osType ~= nil
		and deviceInfo.transportType ~= nil
		-- ToDo: Add correctness check according enums for osType and transportType
end


local function addDeviceIntoList(deviceInfo)
	local foundDevices = getSuitableDevicesList({ id = deviceInfo.id })
	if #foundDevices ~= 0 then
		error("MobileDeviceManager: Try to add duplicate device into list")
	else
		if isDeviceInfoCorrect(deviceInfo) then
			table.insert(availableDevicesList, deviceInfo)
		else
			if deviceInfo.id == config.defaultMobileDeviceInfo.id then
				error("MobileDeviceManager: Default mobile device is incorrect, please check ATF configuration")
			else
				-- ToDo: Perform print message using ATF print capabilities
				print("Device info received from host " .. deviceInfo.host .. ":" .. deviceInfo.port .. " is incorrect")
			end
		end
	end
end

local function removeDeviceFromList(deviceInfo)
	local foundDevices = getSuitableDevicesList({ id = deviceInfo.id })
	if #foundDevices == 1 then
		table.remove(availableDevicesList, foundDevices[1].idx)
	elseif #foundDevices == 0 then
		error("MobileDeviceManager: List does not have device to remove")
	else
		error("MobileDeviceManager: List has duplicate device")
	end
end

local function cleanAdditionalEmulatedMobileDevices()
	local emulatedDevicesList = getSuitableDevicesList({ isRealDevice = false })
	for _, value in ipairs(emulatedDevicesList) do
		if value.deviceInfo.id ~= config.defaultMobileDeviceInfo.id then
			MobileDeviceManager.RemoveEmulatedDevice(value.deviceInfo)
		end
	end
end

-- ToDo: Move this function into platform specific utils module
-- local function getHostsFromWifiNetwork()
-- 	local wifiInterfaceHandler = io.popen("iw dev | awk '$1==\"Interface\"{print $2}' | tr -d '\n'")
-- 	local interfaceName = wifiInterfaceHandler:read('*all')
-- 	wifiInterfaceHandler:close()

-- 	local mobileDevicesHandler = io.popen("arp -i " .. interfaceName .. " | grep C | awk '{print $1}'")
-- 	local output = mobileDevicesHandler:read('*all')
-- 	mobileDevicesHandler:close()

-- 	local mobileDevicesIpString = output:gsub("^%s*(.-)%s*$", "%1")
-- 	-- ToDo: Filter list of IP addresses of devices that listen mobilePort
-- 	return splitString(mobileDevicesIpString, "\n")
-- end

local function getDeviceInfoFromHost(host, port)
	local handler = real_device_communication_handler.CommunicationHandler(host, port, nil)
	return handler:GetDeviceInfo()
end

local function getListOfAvailableDevices()
	for _, device in ipairs(config.MobileDevices) do
		local deviceInfoFromHost = getDeviceInfoFromHost(device.host, device.port)
		if deviceInfoFromHost and deviceInfoFromHost.transports then
			for _, transportType in ipairs(deviceInfoFromHost.transports) do
				local deviceInfo = {
					id = buildDeviceId(device.host, transportType),
					host = device.host,
					port = device.port,
					isRealDevice = true,
					macAddress = deviceInfoFromHost.mac,
					osType = deviceInfoFromHost.os,
					transportType = transportType
				}
				addDeviceIntoList(deviceInfo)
			end
		end
	end
	-- local listOfDevicesIp = getHostsFromWifiNetwork()
	-- for _, ip in ipairs(listOfDevicesIp) do
	-- 	local deviceInfoFromHost = getDeviceInfoFromHost(ip, config.defaultMobileDeviceInfo.port)
	-- 	if deviceInfoFromHost and deviceInfoFromHost.transports then
	-- 		for _, transportType in ipairs(deviceInfoFromHost.transports) do
	-- 			local deviceInfo = {
	-- 				id = buildDeviceId(ip, transportType),
	-- 				host = ip,
	-- 				port = config.defaultMobileDeviceInfo.port,
	-- 				isRealDevice = true,
	-- 				macAddress = deviceInfoFromHost.mac,
	-- 				osType = deviceInfoFromHost.os,
	-- 				transportType = transportType
	-- 			}
	-- 			addDeviceIntoList(deviceInfo)
	-- 		end
	-- 	end
	-- end
end

local function addDefaultDeviceInfoToListOfAvailableDevices()
  local defaultDeviceInfo = config.defaultMobileDeviceInfo
  for _, transportType in ipairs(defaultDeviceInfo.transportTypes) do
    local deviceInfo = {
      id = buildDeviceId(defaultDeviceInfo.host, transportType),
      host = defaultDeviceInfo.host,
      port = defaultDeviceInfo.port,
      isRealDevice = defaultDeviceInfo.isRealDevice,
      macAddress = getDeviceMAC(defaultDeviceInfo.host, defaultDeviceInfo.port),
      osType = defaultDeviceInfo.osType,
      transportType = transportType
    }
    addDeviceIntoList(deviceInfo)
  end
end

local function buildListOfAvailableDevices()
	getListOfAvailableDevices()
  addDefaultDeviceInfoToListOfAvailableDevices()
end

function MobileDeviceManager.FindDevices(deviceInfo)
	local resultList = {}
	local devicesList = getSuitableDevicesList(deviceInfo)
	for _, value in ipairs(devicesList) do
		table.insert(resultList, value.deviceInfo)
	end
	return resultList
end

function MobileDeviceManager.AddEmulatedDevice(host)
	local deviceInfo = {
		id = buildDeviceId(host, config.defaultMobileDeviceInfo.transportType),
		host = host,
		port = config.defaultMobileDeviceInfo.port,
		isRealDevice = false,
		macAddress = getDeviceMAC(host, config.defaultMobileDeviceInfo.port),
		osType = config.defaultMobileDeviceInfo.osType,
		transportType = config.defaultMobileDeviceInfo.transportType
	}

	addDeviceIntoList(deviceInfo)
	addFakeNetworkInterface(deviceInfo.id, deviceInfo.host)
end

function MobileDeviceManager.RemoveEmulatedDevice(deviceInfo)
	if not deviceInfo.isRealDevice
			and  deviceInfo.id ~= config.defaultMobileDeviceInfo.id then
		removeFakeNetworkInterface(deviceInfo.id)
		removeDeviceFromList(deviceInfo)
	else
		error("MobileDeviceManager: Try to remove not emulated device or default emulated device from list")
	end
end

function MobileDeviceManager.GetDefaultDeviceInfo()
  local defaultdeviceId = buildDeviceId(config.defaultMobileDeviceInfo.host, config.defaultMobileAdapterType)
  return MobileDeviceManager.FindDevices({ id = defaultdeviceId })[1]
end

function MobileDeviceManager.CreateMobileConnection(deviceInfo, connectionParams)
	local transportAdapter = mobile_adapter_controller.getAdapter(deviceInfo.transportType, connectionParams)
	local fileConnection = file_connection.FileConnection(buildFileName(deviceInfo.id), transportAdapter)
	local mobileConnection = mobile_connection.MobileConnection(fileConnection, deviceInfo)
	event_dispatcher:AddConnection(mobileConnection)
	removeDeviceFromList(deviceInfo)
	return mobileConnection
end

function MobileDeviceManager.RemoveMobileConnection(mobileConnection)
	if mobileConnection.isConnected then
		mobileConnection:Close()
	end
	-- event_dispatcher:RemoveConnection(mobileConnection)
	-- ToDo: event_dispatcher does not support RemoveConnection
	mobileConnection.connection = nil
	-- ToDo: Move remove file functionality into file_connection module
	os.execute("rm -f " .. buildFileName(mobileConnection.deviceInfo.id))
	addDeviceIntoList(mobileConnection.deviceInfo)
	mobileConnection.deviceInfo = nil
end

buildListOfAvailableDevices()

local mt = { __gc = cleanAdditionalEmulatedMobileDevices }
setmetatable(MobileDeviceManager, mt)

return MobileDeviceManager
