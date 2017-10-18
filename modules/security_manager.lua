local openssl = require('luaopenssl')

local SecurityManager = {}

--- SSL
local ssl_mt = { __index = {} }

-- function ossl_mt.__index:PerformHandshake()
--   -- prepare event to expect (example control_service:StartService)
--   openssl.doHandshake(self.connection)
--   -- expect event and check the result (example control_service:StartService)
-- end



--- BIO
local BIO = {
	types = {
		SOURCE_BIO = 0,
		FILTER_BIO = 1
	}
}

--- SecurityManager

SecurityManager.mobileSessions = {}

function SecurityManager.init()
	openssl.initSslLibrary()
	print("OpenSSL library is initialised")
	-- self.sslContext = openssl.prepareSSLContext()
end

function SecurityManager:createSsl()
	return openssl.newSsl(self.sslContext)
end

function SecurityManager.createBio(bioType)
	return openssl.newBio(bioType)
end

function SecurityManager.decrypt(encryptedData, sessionId, serviceType)
	return encryptedData
end

function SecurityManager.encrypt(data, sessionId)
	return data
end

function SecurityManager:SSL(mobileSession)
	self.mobileSessions[mobileSession.sessionId] = {
		session = mobileSession,
		encryptedServices = {}
	}

	local res = {}
	-- res.isEncriptedSession = false
	-- res.ctx = self:createSslContext()
	-- res.ssl = self.sslContext:newSSL()
	-- res.bioIn = self:createBio(BIO.types.SOURCE_BIO)
	-- res.bioOut = self:createBio(BIO.types.SOURCE_BIO)
	setmetatable(res, ssl_mt)
  return res
end

SecurityManager.init()
return SecurityManager

