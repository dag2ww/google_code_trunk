-- ***************************************************************************
-- DHT22/11 module for ESP8266 with nodeMCU
--
-- Written by Javier Yanez
-- but based on a script of Pigs Fly from ESP8266.com forum
--
-- MIT license, http://opensource.org/licenses/MIT
-- ***************************************************************************

local moduleName = ...
local M = {}
_G[moduleName] = M

local humidity
local temperature
local checksum
local checksumTest
local minCountForLow = 1

function M.read(pin)
  humidity = 0
  temperature = 0
  checksum = 0

  -- Use Markus Gritsch trick to speed up read/write on GPIO
  gpio_read = gpio.read
  gpio_write = gpio.write

  bitStream = {}
  for j = 1, 40, 1 do
    bitStream[j] = 0
  end
  bitlength = 0

  -- Step 1:  send out start signal to DHT
  gpio.mode(pin, gpio.OUTPUT)
  --MCU needs to pull data line down >18mS to signal DHT response
  gpio.write(pin, gpio.LOW)
  tmr.delay(20000)
  --MCU awaiting response
  gpio.mode(pin, gpio.INPUT)

  -- Step 2:  DHT send response signal
  -- MCU looks for DHT to respond with 80uS low on the data line
  while (gpio_read(pin) == 0 ) do end
  -- Followed by a 80uS high on the data line
  while (gpio_read(pin) == 1) do end
  -- MCU looks for DHT to indicate start bit by pulling data line down for 50uS
  while (gpio_read(pin) == 0 ) do end
  
  -- Step 3: DHT send data (40 bits)
  for j = 1, 40, 1 do
    while (gpio_read(pin) == 1 and bitlength < 3 ) do
       bitlength = bitlength + 1
    end
    bitStream[j] = bitlength
    bitlength = 0
    -- MCU looks for DHT to indicate start of next bit by pulling data line down for 50uS
    while (gpio_read(pin) == 0) do end
  end

  --DHT data acquired, process.
  for i = 1, 16, 1 do
    if (bitStream[i + 0] > minCountForLow) then
      -- Add up all the "high" bit postion values
      humidity = humidity + 2 ^ (16 - i)
    end
    if (bitStream[i + 16] > minCountForLow) then
      -- Add up all the "high" bit postion values
      temperature = temperature + 2 ^ (16 - i)
    end
  end
  for i = 1, 8, 1 do
    if (bitStream[i + 32] > minCountForLow) then
      -- Add up all the "high" bit postion values
      checksum = checksum + 2 ^ (8 - i)
    end
  end
  checksumTest=((humidity-humidity%256)/256+humidity%256+(temperature-temperature%256)/256+temperature%256)%256
  if temperature > 0x8000 then
    -- convert to negative format
    temperature = -(temperature - 0x8000)
  end

  if tostring(checksum) ~= tostring(checksumTest) then
  humidity = -1
  end
end

function M.getTemperature()
  return temperature
end

function M.getHumidity()
  return humidity
end

return M