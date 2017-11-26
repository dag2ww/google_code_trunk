
PIN = 4 --  data pin, GPIO2



--if h == nil then
--  print("Error reading from DHT22")
--else
  -- temperature in degrees Celsius  and Farenheit
  -- floating point and integer version:
--  print("Temperature: "..((t-(t % 10)) / 10).."."..(t % 10).." deg C")
  -- only integer version:
--  print("Temperature: "..(9 * t / 50 + 32).."."..(9 * t / 5 % 10).." deg F")
  -- only float point version:
--  print("Temperature: "..(9 * t / 50 + 32).." deg F")
--
  -- humidity
  -- floating point and integer version
--  print("Humidity: "..((h - (h % 10)) / 10).."."..(h % 10).."%")
--end

-- release module
--dht22 = nil
--package.loaded["dht22"]=nil

wifi.setmode(wifi.STATION)
wifi.sta.config("BTHub4-6TPJ","649c46a487")
print(wifi.sta.getip())
led1 = 3
--led2 = 4
gpio.mode(led1, gpio.OUTPUT)
--gpio.mode(led2, gpio.OUTPUT)
srv=net.createServer(net.TCP)
srv:listen(80,function(conn)
    conn:on("receive", function(client,request)
    --dht22 = require("dht22")
    --tmr.delay(1100)
  --dht22.read(PIN)
  --tmr.delay(1000)
--t = dht22.getTemperature()
--h = dht22.getHumidity()

--dht22 = nil
--package.loaded["dht22"]=nil
print("DHT22", dofile("dht22.lua").read(4, true))
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
        buf = buf.."<h1> ESP8266 Web Server:</h1>";
        buf = buf..((t-(t % 10)) / 100).."."..(t % 10);
        buf = buf.."  ";
if h == nil then
  buf = buf.."Error reading from DHT22  ";
else
 buf = buf.."Humidity: "..((h - (h % 10)) / 10).."."..(h % 10).."%";
end
        buf = buf.."  ";
        buf = buf.."<p>GPIO0 <a href=\"?pin=ON1\"><button>ON</button></a>&nbsp;<a href=\"?pin=OFF1\"><button>OFF</button></a></p>";
        buf = buf.."<p>GPIO2 <a href=\"?pin=ON2\"><button>ON</button></a>&nbsp;<a href=\"?pin=OFF2\"><button>OFF</button></a></p>";
        local _on,_off = "",""
        if(_GET.pin == "ON1")then
              gpio.write(led1, gpio.HIGH);
        elseif(_GET.pin == "OFF1")then
              gpio.write(led1, gpio.LOW);
        elseif(_GET.pin == "ON2")then
  --            gpio.write(led2, gpio.HIGH);
        elseif(_GET.pin == "OFF2")then
  --            gpio.write(led2, gpio.LOW);
        end
        client:send(buf);
        client:close();
        collectgarbage();
    end)
end)
