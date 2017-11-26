-- Demo http server for sensor DHT22 - tested with NodeMCU 0.9.5 build 20150107
-- 1. Flash NodeMCU 0.9.5 build 20150107 or later to ESP module.
-- 2. Load program monDHT22.lua and dht22.lua to ESP8266 with LuaLoader
-- 3. You can rename the program monDHT22.lua to init.lua
-- 3. HW reset module
-- 4. Login module to your AP - wifi.setmode(wifi.STATION),wifi.sta.config("yourSSID","yourPASSWORD")
-- 5. Run program monDHT22.lua - dofile(monDHT22.lua)
-- 6. Test IP address - wifi.sta.getip()
-- 7. Test it with your browser and true IP address of module.
-- 8. The sensor is repeatedly read every minute.
-- 9. The pictures on page are external.
--10. The length of html code is limited to 1460 characters.
--11. The author of the program module dht22.lua for reading DHT sensor is Javier Yanez

led1 = 3
gpio.mode(led1, gpio.OUTPUT)

wifi.setmode(wifi.STATION)
wifi.sta.config("BTHub4-6TPJ","649c46a487")
tmr.delay(2000000)
humi="XX"
temp="XX"
fare="XX"
bimb=1
PIN = 4 --  data pin, GPIO2
--load DHT22 module and read sensor
function ReadDHT22()
     dht22 = require("dht22")
     dht22.read(PIN)
     t = dht22.getTemperature()
     h = dht22.getHumidity()
     humi=(h/10).."."..(h%10)
     temp=(t/10).."."..(t%10)
     fare=(9*t/50+32).."."..(9*t/5%10)
     print("Humidity:    "..humi.."%")
     print("Temperature: "..temp.." deg C")
     print("Temperature: "..fare.." deg F")
     -- release module
     dht22 = nil
     package.loaded["dht22"]=nil
end

ReadDHT22()
tmr.alarm(1,5000, 1, function() ReadDHT22() bimb=bimb+1 if bimb==5 then bimb=0 wifi.sta.connect() print("Reconnect")end end)

srv=net.createServer(net.TCP)
srv:listen(80,function(conn)
    conn:on("receive", function(client,request)
    local buf = "";
    local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP");
    if(method == nil)then
       _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP");
    end
    local _GET = {}
    if (vars ~= nil)then
       for k, v in string.gmatch(vars, "(%w+)=(%w+)&*") do
           _GET[k] = v
       end
    end
    local _on,_off = "",""
        if(_GET.pin == "ON1")then
              gpio.write(led1, gpio.HIGH);
        elseif(_GET.pin == "OFF1")then
              gpio.write(led1, gpio.LOW);
        end
 
     --print(payload) -- for debugging only
     --generates HTML web site
    buf = buf..'HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nCache-Control: private, no-store\r\n\r\n\
     <!DOCTYPE HTML>\
    <html><head><meta content="text/html;charset=utf-8"><meta http-equiv="refresh" content="5" >\
    <title>ESP8266</title></head>\
     <body bgcolor="#ffe4c4"><h2>Hygrometer with<br>DHT22 sensor</h2>\
     <h3><font color="green">\
     <IMG SRC="http://esp8266.fancon.cz/common/hyg.gif"WIDTH="64"HEIGHT="64"><br>\
     <input style="text-align: center"type="text"size=4 name="j"value="'..humi..'"> % of relative humidity<br><br>\
     <IMG SRC="http://esp8266.fancon.cz/common/tmp.gif"WIDTH="64"HEIGHT="64"><br>\
     <input style="text-align: center"type="text"size=4 name="p"value="'..temp..'"> Temperature grade C<br>\
     <input style="text-align: center"type="text"size=4 name="p"value="'..fare..'"> Temperature grade F</font></h3>\
     <p>GPIO0 <a href=\"?pin=ON1\"><button>ON</button></a>&nbsp;<a href=\"?pin=OFF1\"><button>OFF</button></a></p>\
     </body></html>'   
       client:send(buf)
        client:close();
        collectgarbage();
    --conn:on("sent",function(conn) conn:close() end)
    end)
end)