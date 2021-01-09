local DT = {}

function DT.get_datetime(format)
  local rawTime = timestamp()
  local msec = rawTime % (10 ^ 3)
  rawTime = math.floor(rawTime / (10 ^ 3))
  local dt = os.date("*t", rawTime)
  if format == "hh:mm:ss,zzz" then
    return string.format("%02d:%02d:%02d,%03d",
      dt.hour, dt.min, dt.sec, msec)
  elseif format == "dd-MM-yyyy hh:mm:ss,zzz" then
    return string.format("%02d-%02d-%04d %02d:%02d:%02d,%03d",
      dt.day, dt.month, dt.year, dt.hour, dt.min, dt.sec, msec)
  end
  return ""
end

return DT
