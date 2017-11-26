-- File content start from next line
LED = require "LEDblink"

print("Running file mqtt")
tmr.delay(100000)

LED.blink0(0, 2000, 2)

gpio12 = 6
gpio.mode(gpio12, gpio.OUTPUT)
gpio.write(gpio12, gpio.LOW)

gpio13 = 7
gpio.mode(gpio13, gpio.OUTPUT)
gpio.write(gpio13, gpio.LOW)

-- init mqtt client with keepalive timer 120sec
m = mqtt.Client("Greg", 120, "tloyusjo", "y1hq7214TfWK")

-- setup Last Will and Testament (optional)
-- Broker will publish a message with qos = 0, retain = 0, data = "offline" 
-- to topic "/lwt" if client don't send keepalive packet
m:lwt("/lwt", "offline", 0, 0)

m:on("connect", function(client) print ("connected") end)
m:on("offline", function(client) print ("offline") end)

-- on publish message receive event
m:on("message", function(conn, topic, data)
  print("Topic   " ..topic.. " get info. If command match then will run" )
  if topic == "Light1" then
    if data == node.chipid().."ON" then
        print("received message: ON@light1")
        gpio.write(gpio12, gpio.HIGH)
    end
    if data == node.chipid().."OFF" then
        print("received message: OFF@light1")
        gpio.write(gpio12, gpio.LOW)
    end
    
  end
    
  if topic == "Light2" then
    if data == node.chipid().."ON" then
        print("received message: ON@light2")
        gpio.write(gpio13, gpio.HIGH)
    end
    if data == node.chipid().."OFF" then
        print("received message: OFF@light2")
        gpio.write(gpio13, gpio.LOW)
    end
    
  end
     
end)


m:connect("m12.cloudmqtt.com", 17386, 0, function(conn)
    print("Your ID: " ..node.chipid().." connected")
    -- subscribe both topic
    m:subscribe({["Light1"]=0, ["Light2"]=0}, function(conn) 
        print("subscribe Light 1 and 2 success")
    end)
        
    m:publish("ID", node.chipid(),0,0, function(conn) 
        print("sent ID") 
    end)
end)
-- File content end
