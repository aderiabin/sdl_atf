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

function CB.ConfigurationBuilder(baseConfigTable)
    local res =  {
        config = baseConfigTable,
        configurationsList = {}
    }
    setmetatable(res, CB.mt)
    return res
end

local function addConfigurationFields(baseConfigTable, configTable)
    for k, v in pairs(configTable) do
        if baseConfigTable[k] then
            print("Configuration option " .. k .. "was overwritten")
        end
        baseConfigTable[k] = v
    end
end

local function loadConfiguration(baseConfigTable, configName)
    local configLoaderFolderPath = "configuration/loader/"
    local configFolderPath = "configuration/"
    local configLoaderSuffix = "_loader"

    local path = configLoaderFolderPath .. configName .. configLoaderSuffix
    local isSuccess, newConfig = pcall(require, path)

    if not isSuccess then
        path = configFolderPath .. configName
        isSuccess, newConfig = pcall(require, path)
        if not isSuccess then
            error("ConfigLoader: Configuration " .. configName .. " was not found in "
                .. configFolderPath .. " and " .. configLoaderFolderPath)
        end
    end

    addConfigurationFields(baseConfigTable, newConfig)
end

function CB.mt.__index:addConfiguration(configurationName)
    table.insert(self.configurationsList, configurationName)
end

function CB.mt.__index:buildConfiguration()
    for _, configuration in ipairs(self.configurationsList) do
        loadConfiguration(self.config, configuration)
    end
    return self.config
end

return CB
