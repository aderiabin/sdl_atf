local bit = require('luv.network.websocket.bit')
local mime = require('mime')

local tools = {}

local DEFAULT_PORTS = { ws = 80, wss = 443 }

function tools.parse_url(url)
  local protocol, address, uri = url:match('^(%w+)://([^/]+)(.*)$')
  if not protocol then error('Invalid URL:'..url) end
  protocol = protocol:lower()
  local host, port = address:match("^(.+):(%d+)$")
  if not host then
    host = address
    port = DEFAULT_PORTS[protocol]
  end
  if not uri or uri == '' then uri = '/' end
  return protocol, host, tonumber(port), uri
end

local function read_n_bytes(str, pos, n)
  pos = pos or 1
  return pos+n, string.byte(str, pos, pos + n - 1)
end

local function read_int8(str, pos)
  return read_n_bytes(str, pos, 1)
end

local function read_int16(str, pos)
  local new_pos,a,b = read_n_bytes(str, pos, 2)
  return new_pos, bit.lshift(a, 8) + b
end

local function read_int32(str, pos)
  local new_pos,a,b,c,d = read_n_bytes(str, pos, 4)
  return new_pos,
  bit.lshift(a, 24) +
  bit.lshift(b, 16) +
  bit.lshift(c, 8 ) +
  d
end

local write_int8 = string.char

local function write_int16(v)
  return string.char(bit.rshift(v, 8), bit.band(v, 0xFF))
end

local function write_int32(v)
  return string.char(
    bit.band(bit.rshift(v, 24), 0xFF),
    bit.band(bit.rshift(v, 16), 0xFF),
    bit.band(bit.rshift(v,  8), 0xFF),
    bit.band(v, 0xFF)
  )
end

local function base64_encode(data)
  return mime.b64(data)
end

function tools.generate_key()
  local r1 = math.random(0,0xfffffff)
  local r2 = math.random(0,0xfffffff)
  local r3 = math.random(0,0xfffffff)
  local r4 = math.random(0,0xfffffff)
  local key = write_int32(r1) .. write_int32(r2) .. write_int32(r3) .. write_int32(r4)
  assert(#key==16, #key)
  return base64_encode(key)
end

math.randomseed(os.time())

-- SHA1 hashing from luacrypto, if available
local sha1_crypto
local done,crypto = pcall(require,'crypto')
if done then
  sha1_crypto = function(msg)
    return crypto.digest('sha1',msg,true)
  end
end

-- from wiki article, not particularly clever impl
local sha1_wiki = function(msg)
  local h0 = 0x67452301
  local h1 = 0xEFCDAB89
  local h2 = 0x98BADCFE
  local h3 = 0x10325476
  local h4 = 0xC3D2E1F0

  local bits = #msg * 8
  -- append b10000000
  msg = msg .. string.char(0x80)

  -- 64 bit length will be appended
  local bytes = #msg + 8

  -- 512 bit append stuff
  local fill_bytes = 64 - (bytes % 64)
  if fill_bytes ~= 64 then
    msg = msg .. string.rep(string.char(0), fill_bytes)
  end

  -- append 64 big endian length
  local high = math.floor(bits/2^32)
  local low = bits - high*2^32
  msg = msg..write_int32(high)..write_int32(low)

  assert(#msg % 64 == 0,#msg % 64)

  for j=1,#msg,64 do
    local chunk = msg:sub(j,j+63)
    assert(#chunk==64,#chunk)
    local words = {}
    local next = 1
    local word
    repeat
      next,word = read_int32(chunk, next)
      table.insert(words, word)
    until next > 64
    assert(#words==16)
    for i=17,80 do
      words[i] = bit.bxor(words[i-3],words[i-8],words[i-14],words[i-16])
      words[i] = bit.rol(words[i],1)
    end
    local a = h0
    local b = h1
    local c = h2
    local d = h3
    local e = h4

    for i=1,80 do
      local k,f
      if i > 0 and i < 21 then
        f = bit.bor(bit.band(b,c),bit.band(bit.bnot(b),d))
        k = 0x5A827999
      elseif i > 20 and i < 41 then
        f = bit.bxor(b,c,d)
        k = 0x6ED9EBA1
      elseif i > 40 and i < 61 then
        f = bit.bor(bit.band(b,c),bit.band(b,d),bit.band(c,d))
        k = 0x8F1BBCDC
      elseif i > 60 and i < 81 then
        f = bit.bxor(b,c,d)
        k = 0xCA62C1D6
      end

      local temp = bit.rol(a,5) + f + e + k + words[i]
      e = d
      d = c
      c = bit.rol(b,30)
      b = a
      a = temp
    end

    h0 = h0 + a
    h1 = h1 + b
    h2 = h2 + c
    h3 = h3 + d
    h4 = h4 + e

  end

  -- necessary on sizeof(int) == 32 machines
  h0 = bit.band(h0,0xffffffff)
  h1 = bit.band(h1,0xffffffff)
  h2 = bit.band(h2,0xffffffff)
  h3 = bit.band(h3,0xffffffff)
  h4 = bit.band(h4,0xffffffff)

  return write_int32(h0)..write_int32(h1)..write_int32(h2)..write_int32(h3)..write_int32(h4)
end

tools.sha1 = sha1_crypto or sha1_wiki
tools.base64 = {
  encode = base64_encode
}
tools.read_int8 = read_int8
tools.read_int16 = read_int16
tools.read_int32 = read_int32
tools.write_int8 = write_int8
tools.write_int16 = write_int16
tools.write_int32 = write_int32

return tools
