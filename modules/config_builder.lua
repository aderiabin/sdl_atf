--- Module which build ATF configuration from configuration pieces
--
-- *Dependencies:*
--
-- *Globals:* none
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local CB = {
    mt = { __index = {} }
}

function CB.ConfigurationBuilder(baseConfigTable, environment)
    local res =  {
        config = baseConfigTable,
        environment = environment,
        configurationsList = {}
    }
    setmetatable(res, CB.mt)
    return res
end

local function addConfigurationFields(baseConfigTable, configTable)
    for k, v in pairs(configTable) do
        baseConfigTable[k] = v
    end
end

local function loadConfiguration(self, configName)
    local envFolderName = self.environment
    if not envFolderName then envFolderName = "" end

    local configFolderPath = "configuration/" .. envFolderName .. "/"
    local configLoaderFolderPath = configFolderPath .."_loader/"
    local configLoaderSuffix = "_loader"

    local path = configLoaderFolderPath .. configName .. configLoaderSuffix
    local isSuccess, newConfig = pcall(require, path)

    if not isSuccess then
        path = configFolderPath .. configName
        isSuccess, newConfig = pcall(require, path)
        if not isSuccess then
            error("ConfigurationBuilder: Configuration " .. configName .. " was not found in "
                .. configFolderPath .. " and " .. configLoaderFolderPath)
        end
    end

    addConfigurationFields(self.config, newConfig)
end

function CB.mt.__index:addConfiguration(configurationName)
    table.insert(self.configurationsList, configurationName)
end

function CB.mt.__index:buildConfiguration()
    for _, configuration in ipairs(self.configurationsList) do
        loadConfiguration(self, configuration)
    end
end

return CB
