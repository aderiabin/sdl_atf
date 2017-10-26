local commonFunctions = require('user_modules/shared_testcases/commonFunctions')
local colors = require('user_modules/consts').color

local debuger = {}
debuger.tags = {}

function debuger.clearTags(tag)
	debuger.tags = {}
end

function debuger.setTags(tagTable)
	debuger.tags = tagTable
end

function DEBUG_MESSAGE(message, data, debugTag)
	if config.debug and (next(debuger.tags) == nil or debuger.tags[debugTag]) then
		if data then
			if type(data) == "table" then
				local binaryData = data.binaryData
				data.binaryData = nil
				message = message .. " =>\n" .. commonFunctions:convertTableToString(data, 1)
				data.binaryData = binaryData
			else
				if type(data) == nil then data = "nil"
				elseif type(data) == "boolean" then
					 data = data and "true" or "false"
				end
				message = message .. " => " .. data
			end
		end
		commonFunctions:userPrint(colors.blue, message)
	end
end



-- local req = {}

-- _oldsetmetatable = setmetatable
-- setmetatable = function(tbl, meta)
-- 		if type(meta) == "table" and type(meta.__index) == "table" then
-- 			for key, value in pairs(meta.__index) do
-- 				if type(value) == "function" then
-- 					local funcInfo = debug.getinfo(value, "S")
-- 					meta.__index[key] = function(...)
-- 						print("Debug: call " .. tostring(key) .. " from " .. funcInfo.short_src .. " line: " .. funcInfo.linedefined)
-- 						return value(...)
-- 					end
-- 				end
-- 			end
-- 		end
-- 		_oldsetmetatable(tbl, meta)
-- 	end

-- _oldrequire = require

-- require = function(str)
-- 		local M = _oldrequire(str)
-- 		if not req[str] and type(M) == "table"  then
-- 			req[str] = true
-- 			for key, value in pairs(M) do
-- 				if type(value) == "function" then
-- 					local funcInfo = debug.getinfo(value, "S")
-- 					if not funcInfo.short_src == "[C]" then
-- 						M[key] = function(...)
-- 							print("Debug: call " .. tostring(key) .. " from " .. funcInfo.short_src .. " line: " .. funcInfo.linedefined)
-- 							return value(...)
-- 						end
-- 					end
-- 				end
-- 			end
-- 		end
-- 		return M
-- 	end

return debuger