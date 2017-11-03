--- Module which is responsible for protocol level message handling and provides ProtocolHandler type
--
-- *Dependencies:* `json`, `protocol_handler.ford_protocol_constants`, `bit32`
--
-- *Globals:* `bit32`
-- @module protocol_handler.protocol_handler
-- @copyright [Ford Motor Company](https://smartdevicelink.com/partners/ford/) and [SmartDeviceLink Consortium](https://smartdevicelink.com/consortium/)
-- @license <https://github.com/smartdevicelink/sdl_core/blob/master/LICENSE>

local ProtocolHandler = {}
local json = require("json")
local constants = require('protocol_handler/ford_protocol_constants')
local securityManager = require('security/security_manager')
local securityConstants = require('security/security_constants')
local mt = { __index = { } }

--- Type which represents protocol level message handling
-- @type ProtocolHandler

--- Construct instance of ProtocolHandler type
-- @treturn ProtocolHandler Constructed instance
function ProtocolHandler.ProtocolHandler()
  local ret =
  {
    buffer = "",
    frames = { }
  }
  setmetatable(ret, mt)
  return ret
end

--- Build byte representation of int32
-- @tparam userdata val Original int32 value
-- @treturn string Byte representation of int32
local function int32ToBytes(val)
  local res = string.char(
    bit32.rshift(bit32.band(val, 0xff000000), 24),
    bit32.rshift(bit32.band(val, 0xff0000), 16),
    bit32.rshift(bit32.band(val, 0xff00), 8),
    bit32.band(val, 0xff)
  )
  return res
end

--- Build int32 value from its byte representation
-- @tparam string val Byte representation of int32
-- @tparam number offset Offset
-- @treturn userdata Built int32 value
local function bytesToInt32(val, offset)
  local res = bit32.lshift(string.byte(val, offset), 24) +
  bit32.lshift(string.byte(val, offset + 1), 16) +
  bit32.lshift(string.byte(val, offset + 2), 8) +
  string.byte(val, offset + 3)
  return res
end

--- Build byte representation of RPC payload
-- @tparam number rpcType RPC type
-- @tparam number rpcFunctionId Function Id
-- @tparam number rpcCorrelationId RPC correlation ID
-- @tparam string payload Data
-- @treturn string Built byte representation of RPC payload
local function rpcPayload(rpcType, rpcFunctionId, rpcCorrelationId, payload)
  payload = payload or ""
  local res = string.char(
    bit32.lshift(rpcType, 4) + bit32.band(bit32.rshift(rpcFunctionId, 24), 0x0f),
    bit32.rshift(bit32.band(rpcFunctionId, 0xff0000), 16),
    bit32.rshift(bit32.band(rpcFunctionId, 0xff00), 8),
    bit32.band(rpcFunctionId, 0xff)) ..
  int32ToBytes(rpcCorrelationId) ..
  int32ToBytes(#payload) ..
  payload
  return res
end

--- Build byte representation of message header
-- @tparam number version Version number of the ford protocol
-- @tparam number encryption Encription flag
-- @tparam number frameType Frame type
-- @tparam number serviceType Service type
-- @tparam number frameInfo Frame info
-- @tparam number sessionId Session Id
-- @tparam string payload Data
-- @tparam number messageId Message Id
-- @treturn string Built byte representation of header
-- @see `Applink Protocol`
local function createProtocolHeader(version, encryption, frameType, serviceType, frameInfo, sessionId, payload, messageId)
  local res = string.char(
    bit32.bor(
      bit32.lshift(version, 4),
      (encryption and 0x08 or 0),
      bit32.band(frameType, 0x07)),
    serviceType,
    frameInfo,
    sessionId) ..
  (payload and int32ToBytes(#payload) or string.char(0, 0, 0, 0)) .. -- size
  int32ToBytes(messageId)
  return res
end

local function parseProtocolHeader(buffer)
  local PROTOCOL_HEADER_SIZE = 12
    local size = bytesToInt32(buffer, 5)
    if #buffer < size + PROTOCOL_HEADER_SIZE then
      return nil
    end
    local msg = {}
    local firstByte = string.byte(buffer, 1)
    msg.version = bit32.rshift(bit32.band(firstByte, 0xf0), 4)
    msg.frameType = bit32.band(firstByte, 0x07)
    msg.encryption = bit32.band(firstByte, 0x08) == 0x08
    msg.serviceType = string.byte(buffer, 2)
    msg.frameInfo = string.byte(buffer, 3)
    msg.sessionId = string.byte(buffer, 4)
    msg.size = size
    msg.messageId = bytesToInt32(buffer, 9)
    msg.binaryData = string.sub(buffer, PROTOCOL_HEADER_SIZE + 1, PROTOCOL_HEADER_SIZE + msg.size)
    return msg
end

local function parseBinaryHeader(message, validateJson)
  local BINARY_HEADER_SIZE = 12
  if message.serviceType == constants.SERVICE_TYPE.CONTROL
    and (bit32.rshift(string.byte(message.binaryData, 1), 4) ~= constants.BINARY_RPC_TYPE.NOTIFICATION
      or bit32.band(bytesToInt32(message.binaryData, 1), 0x0fffffff) ~= constants.BINARY_RPC_FUNCTION_ID.HANDSHAKE
      or bytesToInt32(message.binaryData, 9) ~= 0) then  -- it is not Handshake data
    return
  end
  message.rpcType = bit32.rshift(string.byte(message.binaryData, 1), 4)
  message.rpcFunctionId = bit32.band(bytesToInt32(message.binaryData, 1), 0x0fffffff)
  message.rpcCorrelationId = bytesToInt32(message.binaryData, 5)
  message.rpcJsonSize = bytesToInt32(message.binaryData, 9)
  if message.rpcJsonSize > 0 then
    if not validateJson then
      message.payload = json.decode(string.sub(message.binaryData, BINARY_HEADER_SIZE + 1, BINARY_HEADER_SIZE + message.rpcJsonSize))
    end
  end
  if message.size > message.rpcJsonSize + BINARY_HEADER_SIZE then
    message.binaryData = string.sub(message.binaryData, BINARY_HEADER_SIZE + message.rpcJsonSize + 1)
  else
    message.binaryData = ""
  end
end

local function encryptPayload(data, message)
  if message.encryption and data then
    local encryptionStatus, encryptedData = securityManager:encrypt(data, message.sessionId, message.serviceType)
    if encryptionStatus == securityConstants.SECURITY_STATUS.ERROR then
      error("Protocol handler: Encryption error")
    end
    return encryptedData
  end
  return data
end

local function decryptPayload(data, message)
  if data then
    if message.encryption then
      print("Received encrypted message. Start to decrypt.")
      return securityManager:decrypt(data, message.sessionId, message.serviceType)
    else
      return securityConstants.SECURITY_STATUS.NO_ENCRYPTION, data
    end
  else
    return securityConstants.SECURITY_STATUS.NO_DATA, nil
  end
end

local function printMsgData(msg)
  print("-------------------- " .. msg.sessionId .. ":" .. msg.messageId .. " --------------------")
  local encryption
  if msg.encryption then
    encryption = "true"
  else
    encryption = "false"
  end
  print("Message encryption: " .. encryption)
  print("Message binary data size: " .. msg.size)
  print("Size of current binary data: " .. #msg.binaryData)

  local frameType = ""
  if msg.frameType == constants.FRAME_TYPE.CONTROL_FRAME then
    frameType = "CONTROL_FRAME"
  elseif msg.frameType == constants.FRAME_TYPE.FIRST_FRAME then
    frameType = "FIRST_FRAME"
  elseif msg.frameType == constants.FRAME_TYPE.SINGLE_FRAME then
    frameType = "SINGLE_FRAME"
  elseif msg.frameType == constants.FRAME_TYPE.CONSECUTIVE_FRAME then
    frameType = "CONSECUTIVE_FRAME"
  end
  print("Frame type: " .. frameType)

  local frameInfo = ""
  if msg.frameType == constants.FRAME_TYPE.CONSECUTIVE_FRAME then
    if msg.frameInfo == constants.FRAME_INFO.LAST_FRAME then
      frameInfo = "LAST_FRAME"
    else
      frameInfo = msg.frameInfo
    end
  elseif msg.frameType == constants.FRAME_TYPE.CONTROL_FRAME then
    if msg.frameInfo == constants.FRAME_INFO.HEARTBEAT then
      frameInfo = "HEARTBEAT"
    elseif msg.frameInfo == constants.FRAME_INFO.START_SERVICE then
      frameInfo = "START_SERVICE"
    elseif msg.frameInfo == constants.FRAME_INFO.START_SERVICE_ACK then
      frameInfo = "START_SERVICE_ACK"
    elseif msg.frameInfo == constants.FRAME_INFO.START_SERVICE_NACK then
      frameInfo = "START_SERVICE_NACK"
    elseif msg.frameInfo == constants.FRAME_INFO.END_SERVICE then
      frameInfo = "END_SERVICE"
    elseif msg.frameInfo == constants.FRAME_INFO.END_SERVICE_ACK then
      frameInfo = "END_SERVICE_ACK"
    elseif msg.frameInfo == constants.FRAME_INFO.END_SERVICE_NACK then
      frameInfo = "END_SERVICE_NACK"
    elseif msg.frameInfo == constants.FRAME_INFO.SERVICE_DATA_ACK then
      frameInfo = "SERVICE_DATA_ACK"
    elseif msg.frameInfo == constants.FRAME_INFO.HEARTBEAT_ACK then
      frameInfo = "HEARTBEAT_ACK"
    end
  end
  print("Frame info: " .. frameInfo)

  local serviceType = ""
  if msg.serviceType == constants.SERVICE_TYPE.CONTROL then
    serviceType = "CONTROL"
  elseif msg.serviceType == constants.SERVICE_TYPE.PCM then
    serviceType = "PCM"
  elseif msg.serviceType == constants.SERVICE_TYPE.VIDEO then
    serviceType = "VIDEO"
  elseif msg.serviceType == constants.SERVICE_TYPE.RPC then
    serviceType = "RPC"
  elseif msg.serviceType == constants.SERVICE_TYPE.BULK_DATA then
    serviceType = "BULK_DATA"
  end
  print("Service type: " .. serviceType)
  print ("-------------")
  if msg.frameType ~= constants.FRAME_TYPE.CONTROL_FRAME
          and msg.serviceType == constants.SERVICE_TYPE.CONTROL
          and msg.rpcType == constants.BINARY_RPC_TYPE.NOTIFICATION
          and msg.rpcFunctionId == constants.BINARY_RPC_FUNCTION_ID.HANDSHAKE then
          print("It is Handshake message")
        else
          print("It is not Handshake message")
        end
  print("---------------------------------------------")

end

--- Parse binary message from SDL to table with json validation
-- @tparam string binary Message
-- @tparam boolean validateJson True if JSON validation is required
-- @treturn table Parsed message
function mt.__index:Parse(binary, validateJson)
  self.buffer = self.buffer .. binary
  local res = { }
  while #self.buffer >= 12 do
    local msg = parseProtocolHeader(self.buffer)
    if not msg then break end
    self.buffer = string.sub(self.buffer, msg.size + 13)

    local decryptedData
    msg.decryptionStatus, decryptedData = decryptPayload(msg.binaryData, msg)
    if msg.decryptionStatus == securityConstants.SECURITY_STATUS.SUCCESS then
      msg.binaryData = decryptedData
    end

    if #msg.binaryData == 0
       or msg.decryptionStatus == securityConstants.SECURITY_STATUS.ERROR then
      table.insert(res, msg)
    else
      if msg.frameType == constants.FRAME_TYPE.CONTROL_FRAME then
        table.insert(res, msg)
      elseif msg.frameType == constants.FRAME_TYPE.FIRST_FRAME then
        self.frames[msg.messageId] = ""
      elseif msg.frameType == constants.FRAME_TYPE.SINGLE_FRAME then
        if msg.serviceType == constants.SERVICE_TYPE.RPC
           or msg.serviceType == constants.SERVICE_TYPE.BULK_DATA
           or msg.serviceType == constants.SERVICE_TYPE.CONTROL then
          parseBinaryHeader(msg, validateJson)
        end
        table.insert(res, msg)
      elseif msg.frameType == constants.FRAME_TYPE.CONSECUTIVE_FRAME then
        self.frames[msg.messageId] = self.frames[msg.messageId] .. msg.binaryData
        if msg.frameInfo == constants.FRAME_INFO.LAST_FRAME then
          msg.binaryData = self.frames[msg.messageId]
          self.frames[msg.messageId] = nil
          if msg.serviceType == constants.SERVICE_TYPE.RPC
           or msg.serviceType == constants.SERVICE_TYPE.BULK_DATA
           or msg.serviceType == constants.SERVICE_TYPE.CONTROL then
            parseBinaryHeader(msg, validateJson)
          end
          table.insert(res, msg)
        end
      end
    end
  end
  return res
end

--- Compose table with binary message and header for SDL
-- @tparam table message Table representation of message
-- @treturn table Table with binary message and header
function mt.__index:Compose(message)
  local kMax_protocol_payload_size = 1400 --1488
  local kFirstframe_frameInfo = 0
  local payload = nil
  local is_multi_frame = false
  local res = {}
  local multiframe_payloads = {}

  if message.frameType ~= constants.FRAME_TYPE.CONTROL_FRAME
    and (((message.serviceType == constants.SERVICE_TYPE.RPC
          or message.serviceType == constants.SERVICE_TYPE.BULK_DATA)
        and message.payload)
      or (message.serviceType == constants.SERVICE_TYPE.CONTROL
        and message.rpcType == constants.BINARY_RPC_TYPE.NOTIFICATION
        and message.rpcFunctionId == constants.BINARY_RPC_FUNCTION_ID.HANDSHAKE
        and (not message.payload))) then
    payload = rpcPayload(message.rpcType,
      message.rpcFunctionId,
      message.rpcCorrelationId,
      message.payload)
  end

  if message.binaryData then
    if payload then
      payload = payload .. message.binaryData
    else
      payload = message.binaryData
    end
  end

  local payload_size = 0
  if payload then payload_size = #payload end

  if payload and #payload > kMax_protocol_payload_size then
    is_multi_frame = true
    while #payload > 0 do
      local payload_part = string.sub(payload, 1, kMax_protocol_payload_size)
      payload = string.sub(payload, kMax_protocol_payload_size + 1)
      table.insert(multiframe_payloads, payload_part)
    end
  end

  if is_multi_frame then
    -- 1st frame
    local firstFrame_payload = int32ToBytes(payload_size) .. int32ToBytes(#multiframe_payloads)
    -- firstFrame_payload = encryptPayload(firstFrame_payload, message)
    local header = createProtocolHeader(message.version,
      message.encryption,
      constants.FRAME_TYPE.FIRST_FRAME,
      message.serviceType,
      kFirstframe_frameInfo,
      message.sessionId,
      firstFrame_payload,
      message.messageId)
    local frame = header .. firstFrame_payload
    table.insert(res, frame)

    for frame_number = 1, #multiframe_payloads do
      local frame_info
      if frame_number == #multiframe_payloads then --last frame
        frame_info = 0
      else
        -- frame info range should be [1 - 255].
        -- frame info can't be 0, 0 mean last frame
        frame_info = ((frame_number - 1) % 255) + 1
      end
      local frame_payload = encryptPayload(multiframe_payloads[frame_number], message)
      header = createProtocolHeader(message.version, message.encryption, constants.FRAME_TYPE.CONSECUTIVE_FRAME, message.serviceType,
        frame_info, message.sessionId, frame_payload, message.messageId)
      frame = header .. frame_payload
      table.insert(res, frame)
    end
  else
    payload = encryptPayload(payload, message)
    local header = createProtocolHeader(message.version, message.encryption, message.frameType, message.serviceType,
      message.frameInfo, message.sessionId, payload or "", message.messageId)
    if payload then
      table.insert(res, header .. payload)
    else
      table.insert(res, header)
    end
  end
  return res
end

return ProtocolHandler
