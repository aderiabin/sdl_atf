local util = require ("atf.util")
config = require('config_loader')

local script_name = "main.lua"

xmlReporter = xmlReporter.init(tostring(script_name))
atf_logger = atf_logger.init_log(tostring(script_name))

local ed = require("event_dispatcher")
local mob = require('app.mob')
local utils = require("app.utils")

os.setlocale("C")

event_dispatcher = ed.EventDispatcher()
config.defaultProtocolVersion = 2

-- Init
local function initItemsSubscriptionsState()
  local subscriptionState = utils.cloneTable(config.initialData)
  for item in pairs(subscriptionState) do
    subscriptionState[item] = false
  end
  return subscriptionState
end

local function initItemsData()
  return utils.cloneTable(config.initialData)
end

local function initItemsList()
  local items = {}
  for item in pairs(config.initialData) do
    table.insert(items, item)
  end
  return items
end

local App = {
  db = nil,
  connection = nil,
  session = nil,
  hmiLevel = nil,
  permissions = {},
  items = initItemsList(),
  data = initItemsData(),
  subscriptions = initItemsSubscriptionsState()
}

-- Utils
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

-- Data base
local function writeDataToDb()
  local keys = {}
  local values = {}
  for k, v in pairs(App.data) do
    table.insert(keys, k)
    if type(v) == "string" then
      table.insert(values, "'" .. tostring(v) .. "'")
    else
      table.insert(values, tostring(v))
    end
  end

  local strKeys = table.concat(keys, ", ")
  local strValues = table.concat(values, ", ")

  local command = "mysql -h " .. config.mySQLHost
    .. " -u" .. config.mySQLUser .. " -p" .. config.mySQLPassword
    .. " -D " .. config.mySQLDatabase
    .. " -e \"INSERT INTO data (" .. strKeys .. ") VALUES (" .. strValues .. ");\""
    utils.dprint("SQL command: " .. utils.toString(command))
  os.execute(command)
end

-- Actions
local function registerApp()
  App.session:StartService(7)
  :Do(function()
      utils.dprint("RPC service on session ".. utils.toString(App.session.SessionId.get()) .." is opened")
      local appParams = config["application1"].registerAppInterfaceParams
      local corId = App.session:SendRPC("RegisterAppInterface", appParams)
      App.session:ExpectResponse(corId)
      :Do(function(_, data)
          local payload = data.payload
          if payload.success == true
              and payload.resultCode == "SUCCESS" then
            utils.dprint("App is registered")
          else
            utils.dprint("App is not registered")
          end
        end)
    end)
end

local function subscribeOnVehicleData(pVehicleParameters)
  local requestParameters = {}

  for _, parameter in ipairs(pVehicleParameters) do
    if App.subscriptions[parameter] == false then
      requestParameters[parameter] = true
    end
  end

  if next(requestParameters) ~= nil then
    local paramsForPrint = {}
    for param in pairs(requestParameters) do
      table.insert(paramsForPrint, param)
    end
    utils.dprint("Subscribe on next vehicle data: " .. table.concat(paramsForPrint, ", "))

    local corId = App.session:SendRPC("SubscribeVehicleData", requestParameters)
    App.session:ExpectResponse(corId)
    :Do(function(_, data)
        local payload = data.payload
        if payload.success == true
            and payload.resultCode == "SUCCESS" then
          utils.dprint("Subscription on vehicle data is successful")
        else
          utils.dprint("Subscription on vehicle data is not successful")
        end

        for parameter in pairs(requestParameters) do
          utils.dprint(" - " .. utils.toString(parameter) .. " subscription result: "
            ..  utils.toString(payload[parameter].resultCode))
          if payload[parameter].resultCode == "SUCCESS" then
            App.subscriptions[parameter] = true
          end
        end

        if (payload.info) then
          utils.dprint(" - info: " .. utils.toString(payload.info))
        end
      end)
  end
end

-- Notification handlers
local function onHMIStatusHandler(_, data)
  utils.dprint("Notification OnHMIStatus " .. utils.toString(data.payload.hmiLevel) .. " is received")
  App.hmiLevel = data.payload.hmiLevel

  local rpcs = {
    "SubscribeVehicleData",
    "OnVehicleData"
  }

  local parameters = App.items
  local isPrintSubscriptionAllowance = false
  for _, parameter in ipairs(parameters) do
    if App.subscriptions[parameter] == false then
      isPrintSubscriptionAllowance = true
    end
  end

  if isPrintSubscriptionAllowance then
    for _, rpcName in ipairs(rpcs) do
      utils.dprint(" - " .. utils.toString(rpcName) .. " is " .. (isRpcAllowed(rpcName) and "allowed" or "disallowed"))
      utils.dprint(" - " .. utils.toString(rpcName) .. " with parameters (" .. table.concat(parameters, ", ") ..  ") is "
        .. (isRpcWithParametersAllowed({ name = rpcName, parameters = parameters }) and "allowed" or "disallowed"))
    end
  end

  if data.payload.hmiLevel == "NONE" then
    subscribeOnVehicleData(parameters)
  end
end

local function onPermissionsChangeHandler(_, data)
  utils.dprint("Notification OnPermissionsChange is received")

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
    utils.dprint(" - " .. utils.toString(rpcName) .. " permissions is " .. utils.toString(App.permissions[rpcName]))
  end
end

local function onVehicleDataHandler(_, data)
  utils.dprint("Notification OnVehicleData is received")
  for paramName, paramValue in pairs(data.payload) do
    utils.dprint(" - " .. utils.toString(paramName) .. ": " .. utils.toString(paramValue))
    App.data[paramName] = paramValue
  end
  -- write data to DB
  writeDataToDb()
end

local function onConnect()
  utils.dprint("Connection with SDL is established")
  App.session = mob.createSession(App.connection)
  registerApp()
  App.session:ExpectNotification("OnHMIStatus"):Do(onHMIStatusHandler)
  App.session:ExpectNotification("OnPermissionsChange"):Do(onPermissionsChangeHandler)
  App.session:ExpectNotification("OnVehicleData"):Do(onVehicleDataHandler)
  -- App.connection:Close()
end

local function onDisconnect()
  utils.dprint("Connection with SDL is canceled")
  xmlReporter:finalize()
  quit()
end

-- create mobile connection
App.connection = mob.createMobileConnection(config.mobileHost, config.mobilePort, onConnect, onDisconnect)
-- connect to SDL
App.connection:Connect()
