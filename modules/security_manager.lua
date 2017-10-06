local openssl = require('ssl')

local SecurityManager = {}

--- SSL
local ssl_mt = { __index = {} }

function ssl_mt.__index:PerformHandshake()
  -- prepare event to expect (example control_service:StartService)
  openssl.doHandshake(self.connection)
  -- expect event and check the result (example control_service:StartService)
end

--- BIO
local BIO = {
	types = {
		SOURCE_BIO = 0,
		FILTER_BIO = 1
	}
}

--- SecurityManager

function SecurityManager:init()
	openssl.initSslLibrary()
	self.sslContext = openssl.prepareSSLContext()
end

function SecurityManager:createSsl()
	return openssl.newSsl(self.sslContext)
end

function SecurityManager:createBio(bioType)
	return openssl.newBio(bioType)
end

function SecurityManager:SSL()
	local res = {}
	res.isEncriptedSession = false
	res.ssl = self:createSsl()
	res.bioIn = self:createBio(BIO.types.SOURCE_BIO)
	res.bioOut = self:createBio(BIO.types.SOURCE_BIO)
	setmetatable(res, ssl_mt)
  return res
end

return SecurityManager