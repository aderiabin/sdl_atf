local tools = require('luv.network.websocket.tools')
local bit = require'luv.network.websocket.bit'

local function bits(...)
  local n = 0
  for _,bitn in pairs{...} do
    n = n + 2^bitn
  end
  return n
end

local bit_0_3 = bits(0,1,2,3)
local bit_0_6 = bits(0,1,2,3,4,5,6)
local bit_7 = bits(7)

local frame = {}

frame.opcodes = {
  CONTINUATION = 0,
  TEXT = 1,
  BINARY = 2,
  CLOSE = 8,
  PING = 9,
  PONG = 10
}

-- TODO: improve performance
local function xor_mask(encoded,mask,payload)
  local transformed,transformed_arr = {},{}
  -- xor chunk-wise to prevent stack overflow.
  -- sbyte and schar multiple in/out values
  -- which require stack
  for p=1,payload,2000 do
    local last = math.min(p+1999,payload)
    local original = {string.byte(encoded,p,last)}
    for i=1,#original do
      local j = (i-1) % 4 + 1
      transformed[i] = bit.bxor(original[i],mask[j])
    end
    local xored = string.char(unpack(transformed,1,#original))
    table.insert(transformed_arr,xored)
  end
  return table.concat(transformed_arr)
end

local function encode_header_small(header, payload)
  return string.char(header, payload)
end

local function encode_header_medium(header, payload, len)
  return string.char(header, payload,
    bit.band(bit.rshift(len, 8), 0xFF), bit.band(len, 0xFF))
end

local function encode_header_big(header, payload, high, low)
  return string.char(header, payload)
    .. tools.write_int32(high) .. tools.write_int32(low)
end

function frame.encode(data,opcode,masked,fin)
  local header = opcode or 1-- TEXT is default opcode
  if fin == nil or fin == true then
    header = bit.bor(header,bit_7)
  end
  local payload = 0
  if masked then
    payload = bit.bor(payload,bit_7)
  end
  local len = #data
  local chunks = {}
  if len < 126 then
    payload = bit.bor(payload,len)
    table.insert(chunks,encode_header_small(header,payload))
  elseif len <= 0xffff then
    payload = bit.bor(payload,126)
    table.insert(chunks,encode_header_medium(header,payload,len))
  elseif len < 2^53 then
    local high = math.floor(len/2^32)
    local low = len - high*2^32
    payload = bit.bor(payload,127)
    table.insert(chunks,encode_header_big(header,payload,high,low))
  end
  if not masked then
    table.insert(chunks,data)
  else
    local m1 = math.random(0,0xff)
    local m2 = math.random(0,0xff)
    local m3 = math.random(0,0xff)
    local m4 = math.random(0,0xff)
    local mask = {m1,m2,m3,m4}
    table.insert(chunks, tools.write_int8(m1,m2,m3,m4))
    table.insert(chunks, xor_mask(data,mask,#data))
  end
  return table.concat(chunks)
end

function frame.encode_close(code,reason)
  if code then
    local data = tools.write_int16(code)
    if reason then
      data = data..tostring(reason)
    end
    return data
  end
  return ''
end

function frame.decode(encoded)
  local encoded_bak = encoded
  if #encoded < 2 then
    return nil,2-#encoded
  end
  local pos,header,payload
  pos,header = tools.read_int8(encoded,1)
  pos,payload = tools.read_int8(encoded,pos)
  local high,low
  encoded = string.sub(encoded,pos)
  local bytes = 2
  local fin = bit.band(header,bit_7) > 0
  local opcode = bit.band(header,bit_0_3)
  local mask = bit.band(payload,bit_7) > 0
  payload = bit.band(payload,bit_0_6)
  if payload > 125 then
    if payload == 126 then
      if #encoded < 2 then
        return nil,2-#encoded
      end
      pos,payload = tools.read_int16(encoded,1)
    elseif payload == 127 then
      if #encoded < 8 then
        return nil,8-#encoded
      end
      pos,high = tools.read_int32(encoded,1)
      pos,low = tools.read_int32(encoded,pos)
      payload = high*2^32 + low
      if payload < 0xffff or payload > 2^53 then
        assert(false,'INVALID PAYLOAD '..payload)
      end
    else
      assert(false,'INVALID PAYLOAD '..payload)
    end
    encoded = string.sub(encoded,pos)
    bytes = bytes + pos - 1
  end
  local decoded
  if mask then
    local bytes_short = payload + 4 - #encoded
    if bytes_short > 0 then
      return nil,bytes_short
    end
    local m1,m2,m3,m4
    pos,m1 = tools.read_int8(encoded,1)
    pos,m2 = tools.read_int8(encoded,pos)
    pos,m3 = tools.read_int8(encoded,pos)
    pos,m4 = tools.read_int8(encoded,pos)
    encoded = string.sub(encoded,pos)
    local m_mask = {
      m1,m2,m3,m4
    }
    decoded = xor_mask(encoded,m_mask,payload)
    bytes = bytes + 4 + payload
  else
    local bytes_short = payload - #encoded
    if bytes_short > 0 then
      return nil,bytes_short
    end
    if #encoded > payload then
      decoded = string.sub(encoded,1,payload)
    else
      decoded = encoded
    end
    bytes = bytes + payload
  end
  return decoded,fin,opcode,encoded_bak:sub(bytes+1),mask
end

function frame.decode_close(data)
  local _,code,reason
  if data then
    if #data > 1 then
      _,code = tools.read_int16(data,1)
    end
    if #data > 2 then
      reason = data:sub(3)
    end
  end
  return code,reason
end

return frame
