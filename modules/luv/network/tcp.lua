local TcpManager = {}

-- TCP client
local tcp_client_mt = { __index = {} }

function tcp_client_mt.__index:connect(targetHost, targetPort, connectionTimeout, sourceHost)
  if self.client and self.onConnectHandler then
    self.client:connect(targetHost, targetPort, self.onConnectHandler)
  end
end

function tcp_client_mt.__index:read(size)
  if self.onDataHandler then
    self.client:read_start(function (err, chunk)
      assert(not err, err)
      if chunk then
        self.onDataHandler(chunk)
      else
        self.client:read_stop()
      end
    end)
  end
end

function tcp_client_mt.__index:read_all()
  local data = {}
  self.client:read_start(function (err, chunk)
    assert(not err, err)
    if chunk then
      table.insert(data, chunk)
    else
      self.client:read_stop()
      return table.concat(data)
    end
  end)
end

function tcp_client_mt.__index:write(pData)
  -- ToDo: Add connection check here
  self.client:write(pData)
end

function tcp_client_mt.__index:close() end

function tcp_client_mt.__index:setOnDataHandler(pHandlerFunc)
  if not type(pHandlerFunc) == "function" then
    print("OnDataHandler of " .. self .. " TCP client was not set because "
      .. pHandlerFunc .. " is not a function")
    return
  end
  self.onDataHandler = pHandlerFunc
end

function tcp_client_mt.__index:setOnConnectHandler(pHandlerFunc)
  if not type(pHandlerFunc) == "function" then
    print("OnConnectHandler of " .. self .. " TCP client was not set because "
      .. pHandlerFunc .. " is not a function")
    return
  end
  self.onDataHandler = pHandlerFunc
end

-- TCP server
local tcp_server_mt = { __index = {} }

function tcp_server_mt.__index:listen(host, port)
  self.server:bind(host, port)

  self.server:listen(self.backlog, function(err)
    -- Accept the client
    local client = TcpManager.TcpClient()
    self.server:accept(client.client)
    table.insert(self.pendingConnections, client)
  end)
end

function tcp_server_mt.__index:get_connection()
  if #self.pendingConnections > 0 then
    local connection = self.pendingConnections[1]
    table.remove(self.pendingConnections, 1)
    return connection
  end
  return nil
end

-- TimersManager
function TcpManager.TcpClient()
  local res = {
    client = uv.new_tcp(),
    onDataHandler = nil,
    onConnectHandler = nil
  }
  setmetatable(res, tcp_client_mt)
  return res
end

function TcpManager.TcpServer()
  local res = {
    server = uv.new_tcp(),
    backlog = 128,
    pendingConnections = {}
  }
  setmetatable(res, tcp_server_mt)
  return res
end

return TcpManager
