local openssl = require('openssl')

local SM = {}
local mt = { __index = { } }

-- Manage digital certificates
-- Manage security protocols
-- Manage cyphers
-- Handle Secure Sockets Layer (SSL) context
-- Provide interface for encryption/decryption
-- Be able to perform a handshake
function mt.__index:PerformHandshake()
  -- prepare event to expect (example control_service:StartService)
  openssl.doHandshake(self.connection)
  -- expect event and check the result (example control_service:StartService)
end

function SM.SecurityManager(connection)
	local res = {}
	--- isEncriptedSession
	res.isEncriptedSession = false
	res.connection = connection
	res.SSL = openssl.getSsl()
	res.SSL_CTX = openssl.getSslContext()
	setmetatable(res, mt)
  return res
end

return SM