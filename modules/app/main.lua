local util = require ("atf.util")
config = require('config_loader')

local script_name = "main.lua"

xmlReporter = xmlReporter.init(tostring(script_name))
atf_logger = atf_logger.init_log(tostring(script_name))

local ed = require("event_dispatcher")
local mob = require('app.mob')
local utils = require("app.utils")

os.setlocale("C")

local App = {
  status = "Launch",
  connection = nil,
  session = nil,
  hmiLevel = nil,
  permissions = {},
  subscriptions = {
    vehicleData = {
      speed = false,
      fuelLevel = false,
      rpm = false
    }
  },
  data = {
    vehicleData = {}
  }
}

event_dispatcher = ed.EventDispatcher()
config.defaultProtocolVersion = 2

local function isRpcAllowed(pRpcName)
  if App.hmiLevel
      and App.permissions[pRpcName]
      and utils.isTableContains(App.permissions[pRpcName].hmiPermissions.allowed, App.hmiLevel)
      and not utils.isTableContains(App.permissions[pRpcName].hmiPermissions.userDisallowed, App.hmiLevel) then
    return true
  end
  return false
end

local function isRpcWithParametersAllowed(pRpcData)
  local function isParameterAllowed(pRpcName, pParameterName)
    local parameterPermissions = App.permissions[pRpcName].parameterPermissions
    if utils.isTableContains(parameterPermissions.allowed, pParameterName)
        and not utils.isTableContains(parameterPermissions.userDisallowed, pParameterName) then
      return true
    end
    return false
  end

  if isRpcAllowed(pRpcData.name) then
    if pRpcData.parameters then
      for _, parameterName in ipairs(pRpcData.parameters) do
        if not isParameterAllowed(pRpcData.name, parameterName) then
          return false
        end
      end
    end
    return true
  end
  return false
end

local function registerApp()
  App.session:StartService(7)
  :Do(function()
      print("RPC service on session ".. utils.toString(App.session.SessionId.get()) .." is opened")
      local appParams = config["application1"].registerAppInterfaceParams
      local corId = App.session:SendRPC("RegisterAppInterface", appParams)
      App.session:ExpectResponse(corId)
      :Do(function(_, data)
          local payload = data.payload
          if payload.success == true
              and payload.resultCode == "SUCCESS" then
            print("App is registered")
          else
            print("App is not registered")
          end
        end)
    end)
end

local function subscribeOnVehicleData(pVehicleParameters)
  local requestParameters = {}

  for _, parameter in ipairs(pVehicleParameters) do
    if App.subscriptions.vehicleData[parameter] == false then
      requestParameters[parameter] = true
    end
  end

  if next(requestParameters) ~= nil then
    local paramsForPrint = {}
    for param in pairs(requestParameters) do
      table.insert(paramsForPrint, param)
    end
    print("Subscribe on next vehicle data: " .. table.concat(paramsForPrint, ", "))

    local corId = App.session:SendRPC("SubscribeVehicleData", requestParameters)
    App.session:ExpectResponse(corId)
    :Do(function(_, data)
        local payload = data.payload
        if payload.success == true
            and payload.resultCode == "SUCCESS" then
          print("Subscription on vehicle data is successful")
        else
          print("Subscription on vehicle data is not successful")
        end

        for parameter in pairs(requestParameters) do
          print(" - " .. utils.toString(parameter) .. " subscription result: "
            ..  utils.toString(payload[parameter].resultCode))
          if payload[parameter].resultCode == "SUCCESS" then
            App.subscriptions.vehicleData[parameter] = true
          end
        end

        if (payload.info) then
          print(" - info: " .. utils.toString(payload.info))
        end
      end)
  end
end

local function onHMIStatusHandler(_, data)
  print("Notification OnHMIStatus " .. utils.toString(data.payload.hmiLevel) .. " is received")
  App.hmiLevel = data.payload.hmiLevel

  local rpcs = {
    "SubscribeVehicleData",
    "OnVehicleData"
  }

  local parameters = {
    "rpm" , "speed", "fuelLevel"
  }

  local isPrintSubscriptionAllowance = false
  for _, parameter in ipairs(parameters) do
    if App.subscriptions.vehicleData[parameter] == false then
      isPrintSubscriptionAllowance = true
    end
  end

  if isPrintSubscriptionAllowance then
    for _, rpcName in ipairs(rpcs) do
      print(" - " .. utils.toString(rpcName) .. " is " .. (isRpcAllowed(rpcName) and "allowed" or "disallowed"))
      print(" - " .. utils.toString(rpcName) .. " with parameters (" .. table.concat(parameters, ", ") ..  ") is "
        .. (isRpcWithParametersAllowed({ name = rpcName, parameters = parameters }) and "allowed" or "disallowed"))
    end
  end

  if data.payload.hmiLevel == "FULL" then
    subscribeOnVehicleData(parameters)
  end
end

local function onPermissionsChangeHandler(_, data)
  print("Notification OnPermissionsChange is received")

  for _, item in ipairs(data.payload.permissionItem) do
    local rpcName = item.rpcName
    item.rpcName = nil
    App.permissions[rpcName] = item
  end

  local rpcs = {
    "SubscribeVehicleData",
    "OnVehicleData"
  }

  for _, rpcName in ipairs(rpcs) do
    print(" - " .. utils.toString(rpcName) .. " permissions is " .. utils.toString(App.permissions[rpcName]))
  end
end

local function onVehicleDataHandler(_, data)
  print("Notification OnVehicleData is received")
  for paramName, paramValue in pairs(data.payload) do
    print(" - " .. utils.toString(paramName) .. ": " .. utils.toString(paramValue))
    App.data.vehicleData[paramName] = paramValue
  end
end

local function onConnect()
  print("Connection with SDL is established")
  App.session = mob.createSession(App.connection)
  registerApp()
  App.session:ExpectNotification("OnHMIStatus"):Do(onHMIStatusHandler)
  App.session:ExpectNotification("OnPermissionsChange"):Do(onPermissionsChangeHandler)
  App.session:ExpectNotification("OnVehicleData"):Do(onVehicleDataHandler)
end

local function onDisconnect()
  print("Connection with SDL is canceled")
  xmlReporter:finalize()
  -- quit()
end

-- create mobile connection
App.connection = mob.createMobileConnection(config.mobileHost, config.mobilePort, onConnect, onDisconnect)

function start()
  if App.status ~= "connected" then
    -- connect to SDL
    App.status = "connected"
    App.connection:Connect()
  end
end

function stop()
  if App.status == "connected" then
    -- disconnect from SDL
    App.status = "disconnected"
    App.connection:Close()
  end
end
