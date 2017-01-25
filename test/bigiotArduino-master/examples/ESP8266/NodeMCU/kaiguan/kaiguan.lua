DEVICEID = "1354"
APIKEY = "4b5d90d07"
INPUTID = "36"
host = host or "www.bigiot.net"
port = port or 8181
LED = 15
gpio.mode(LED,gpio.OUTPUT)
local function run()
  local cu = net.createConnection(net.TCP)
  cu:on("receive", function(cu, c) 
    print(c)
    r = cjson.decode(c)
    if r.M == "say" then
      if r.C == "play" then   
        gpio.write(LED, gpio.HIGH)  
        ok, played = pcall(cjson.encode, {M="say",ID=r.ID,C="LED turn on!"})
        cu:send( played.."\n" )
      end
      if r.C == "stop" then   
        gpio.write(LED, gpio.LOW)
        ok, stoped = pcall(cjson.encode, {M="say",ID=r.ID,C="LED turn off!"})
        cu:send( stoped.."\n" ) 
      end
    end
  end)
  cu:on('disconnection',function(scu)
    cu = nil
    --停止心跳包发送定时器�?秒后重试
    tmr.stop(1)
    tmr.alarm(6, 5000, 0, run)
  end)
  cu:connect(port, host)
  ok, s = pcall(cjson.encode, {M="checkin",ID=DEVICEID,K=APIKEY})
  if ok then
    print(s)
  else
    print("failed to encode!")
  end
  cu:send(s.."\n")
  tmr.alarm(1, 60000, 1, function()
    cu:send(s.."\n")
  end)
end
run()