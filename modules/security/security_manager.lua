local openssl = require('luaopenssl')
local securityConstants = require('security/security_constants')

local SecurityManager = {}

local function updateSecurityOfSession(security)
	if next(security.encryptedServices) == nil then
		security.session.isSecuredSession = false
		security.ctx = nil
		security.ssl = nil
		security.bioIn = nil
		security.bioOut = nil
	else
		security.session.isSecuredSession = true
	end
end

local function getSecurityProtocolConst(strProtocol)
	local protocolConst = securityConstants.PROTOCOLS[strProtocol]
	return protocolConst and true or securityConstants.PROTOCOLS.AUTO
end

--- Security
local security_mt = { __index = {} }

function security_mt.__index:isHandshakeFinished()
	if self.ssl then
		return self.ssl:isHandshakeFinished()
	end
	return false
end

--- Prepare openssl to perform handshake on base of securitySettings
function security_mt.__index:prepareToHandshake()
	local SERVER = 1
	-- create SSL_CTX
	self.ctx = SecurityManager.createSslContext(self)
	-- print("Server SSL_CTX is initialized")
	-- create BIOs
	self.bioIn = SecurityManager.createBio(securityConstants.BIO_TYPES.SOURCE, self)
	-- print("Input BIO is created")
	self.bioOut = SecurityManager.createBio(securityConstants.BIO_TYPES.SOURCE, self)
	-- print("Output BIO is created")
	-- create SSL
	self.ssl = self.ctx:newSsl()
	-- print("SSL is created")
	-- set info callback
	self.ssl:setInfoCallback(SERVER)
	-- print("SSL InfoCallback is set")
	-- populate SSL with BIOs
	self.ssl:setBios(self.bioIn, self.bioOut)
	-- print("SSL BIOs is attached")
	-- Prepare SSL to perform handshake
	self.ssl:prepareToHandshake(SERVER)
	-- print("SSL is prepared to handshake")
end

function security_mt.__index:performHandshake(inHandshakeData)
	local outHandshakeData = nil
	if not self:isHandshakeFinished()
		 and inHandshakeData and inHandshakeData:len() > 0 then
		self.bioIn:write(inHandshakeData);
		self.ssl:performHandshake()
		local pending = self.bioOut:checkData()
		if pending > 0 then
   		outHandshakeData = self.bioOut:read(pending)
   	end
	end
	return outHandshakeData
end

function security_mt.__index:encrypt(data)
	local encryptedData
	if not (self:isHandshakeFinished() and data) then
		return securityConstants.SECURITY_STATUS.ERROR, nil
	end

	self.ssl:encrypt(data)
	local pending = self.bioOut:checkData()
	if pending == 0 then
		return securityConstants.SECURITY_STATUS.ERROR, nil
	end

	encryptedData = self.bioOut:read(pending)
	if not encryptedData then
		return securityConstants.SECURITY_STATUS.ERROR, nil
	end

	return securityConstants.SECURITY_STATUS.SUCCESS, encryptedData
end

function security_mt.__index:decrypt(encryptedData)
	local data
	if not (self:isHandshakeFinished()
		 and encryptedData and encryptedData:len() > 0) then
		return securityConstants.SECURITY_STATUS.ERROR, nil
	end

	self.bioIn:write(encryptedData)
	local pending = self.ssl:checkData()
	if pending == 0 then
		return securityConstants.SECURITY_STATUS.ERROR, nil
	end

	data = self.ssl:decrypt(pending)
	if not data then
		return securityConstants.SECURITY_STATUS.ERROR, nil
	end

	return securityConstants.SECURITY_STATUS.SUCCESS, data
end

function security_mt.__index:registerSessionSecurity()
	if not SecurityManager.mobileSecurities[self.session.sessionId.get()] then
		SecurityManager.mobileSecurities[self.session.sessionId.get()] = self
		print("Session " .. self.session.sessionId.get() .. " was registered in Security manager")
		print("Registered sessions:")
		for k, _ in pairs(SecurityManager.mobileSecurities) do
			print(k)
		end
	end
end

function security_mt.__index:registerSecureService(service)
	self.encryptedServices[service] = true
	updateSecurityOfSession(self)
end

function security_mt.__index:unregisterSecureService(service)
	self.encryptedServices[service] = nil
	updateSecurityOfSession(self)
end

function security_mt.__index:unregisterAllSecureServices()
	self.encryptedServices = {}
	updateSecurityOfSession(self)
end

function security_mt.__index:checkSecureService(service)
	return self.encryptedServices[service]
end

--- SecurityManager
SecurityManager.mobileSecurities = {}

function SecurityManager.init()
	openssl.initSslLibrary()
	print("OpenSSL library is initialised")
end

function SecurityManager.createSslContext(sessionSecurity)
	local sslCtx = openssl.newSslContext(getSecurityProtocolConst(sessionSecurity.settings.securityProtocol))
	if (not (sslCtx and sslCtx:initSslContext(
								sessionSecurity.settings.cipherListString,
								sessionSecurity.settings.serverCertPath,
								sessionSecurity.settings.serverKeyPath))) then
		error("Error: Can not create and init SSL context for mobile session " .. sessionSecurity.session.sessionId.get())
	end
	return sslCtx
end

function SecurityManager.createBio(bioType, sessionSecurity)
	local bio = openssl.newBio(bioType)
	if not bio then
		error("Error: Can not create BIO for mobile session " .. sessionSecurity.session.sessionId.get())
	end
	return bio
end

function SecurityManager:decrypt(encryptedData, sessionId, serviceType)
	local security = self.mobileSecurities[sessionId]
	if not security then
		print("Error [decrypt]: Session " .. sessionId .. " is not registered in ATF Security manager")
		return securityConstants.SECURITY_STATUS.ERROR, encryptedData
	end
	if not security:checkSecureService(serviceType) then
		print("Warning: Received encrypted message with not secure service: " .. serviceType)
	end
	return security:decrypt(encryptedData)
end

function SecurityManager:encrypt(data, sessionId, serviceType)
	local security = self.mobileSecurities[sessionId]
	if not security then
		error("Error [encrypt]: Session " .. sessionId .. " is not registered in ATF Security manager")
	end
	if not security:checkSecureService(serviceType) then
		error("Error: Try to send encrypted message with not secure service" .. serviceType)
	end
	return security:encrypt(data)
end

function SecurityManager:Security(mobileSession, securitySettings)
		local res = {}
	res.settings = securitySettings
	res.session = mobileSession
	res.encryptedServices = {}
	-- res.isEncriptedSession = false
	-- res.ctx
	-- res.ssl
	-- res.bioIn
	-- res.bioOut
	setmetatable(res, security_mt)
  return res
end

SecurityManager.init()
return SecurityManager

