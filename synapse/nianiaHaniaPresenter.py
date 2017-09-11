from synapse.platforms import *

#Pin definitions
buttonPin = 29 #phisical pin 19
buzzerPin = 9 # phisical pin 20
linkPin = 28 # shows the link status based on real communication happening
led_1_pin = 15 # phisical pin 11
led_2_pin = 12 # phisical pin 12
led_3_pin = 8 # phisical pin 13
led_4_pin = 37 # phisical pin 15
led_5_pin = 23 # phisical pin 16
led_6_pin = 31 # phisical pin 17

#global variables
lastLinkOn = 0
buzzerOnLevel = 0
lastButtonState = True

@setHook(HOOK_STARTUP)
def init():
    #0 = 256kbps
    #1 = 500kbps
    #2 = 1Mbps
    #3 = 2Mbps
    setRadioRate(0)
    # Configure LED pins as an output
    setPinDir(buzzerPin, True)
    setPinDir(linkPin, True)
    setPinDir(led_1_pin, True)
    setPinDir(led_2_pin, True)
    setPinDir(led_3_pin, True)
    setPinDir(led_4_pin, True)
    setPinDir(led_5_pin, True)
    setPinDir(led_6_pin, True)

    # Pulse the LEDs once during start-up
    pulsePin(linkPin, 200, True)
    sleep(0,-5)
    pulsePin(led_1_pin, 200, True)
    sleep(0,-5)
    pulsePin(led_2_pin, 200, True)
    sleep(0,-5)
    pulsePin(led_3_pin, 200, True)
    sleep(0,-5)
    pulsePin(led_4_pin, 200, True)
    sleep(0,-5)
    pulsePin(led_5_pin, 200, True)
    sleep(0,-5)
    pulsePin(led_6_pin, 200, True)    
    sleep(0,-5)
    pulsePin(buzzerPin, 200, True)    
    sleep(0,-5)
 
@setHook(HOOK_1S)
def countOffTimeAndBlinkPowered():
    global lastLinkOn
    global lastButtonState
    global buzzerOnLevel
    lastLinkOn = lastLinkOn + 1
    if lastLinkOn >=3: 
        lastLinkOn = 0
        levelLedsOff()
        pulsePin(linkPin, 100, True)
    buttonState = readPin(buttonPin)
    if ((buttonState == False) and (lastButtonState == False)):
        lastButtonState = buttonState
        #next level and led indication
        buzzerOnLevel = buzzerOnLevel+1
        if buzzerOnLevel > 6: 
            buzzerOnLevel = 0
            writePin(buzzerPin, False)
        
        if buzzerOnLevel == 1: led1On() 
        else: led1Off()
            
        if buzzerOnLevel == 2: led2On() 
        else: led2Off()

        if buzzerOnLevel == 3: led3On() 
        else: led3Off()

        if buzzerOnLevel == 4: led4On() 
        else: led4Off()

        if buzzerOnLevel == 5: led5On() 
        else: led5Off()

        if buzzerOnLevel == 6: led6On() 
        else: led6Off()
    lastButtonState = buttonState    
        
     
# Request the remote leds status - 10 times per second     
#@setHook(HOOK_100MS)
#def requestRemoteNoiseLevel():
#    linkOff() # will get On once response comes
#    #call callback on remote node which will call handleNoiseLevel on our node with result of remote getNoiseLevel
#    rpc('\x5D\xA4\x3D', 'callback', 'handleNoiseLevel', 'getNoiseLevel')
        
def handleNoiseLevel(lastNoiseLevel):
    #linkOn()
    global lastLinkOn
    lastLinkOn = 0
    pulsePin(linkPin, 200, True)
    #present the mic reading only in case noone keeps the buzzerLevel button pressed
    if lastButtonState == True:
        #split the value into 6 LEDs
        if lastNoiseLevel >= 1: led1On() 
        else: led1Off()
            
        if lastNoiseLevel  >=2: led2On() 
        else: led2Off()

        if lastNoiseLevel >= 4: led3On() 
        else: led3Off()

        if lastNoiseLevel >= 8: led4On() 
        else: led4Off()

        if lastNoiseLevel >= 16: led5On() 
        else: led5Off()

        if lastNoiseLevel >= 32: led6On() 
        else: led6Off()
    
#Helpers 

def setBeeperOn():
    if lastButtonState == True:
        writePin(buzzerPin, True)

def setBeeperOff():
    writePin(buzzerPin, False)
       
def levelLedsOff():
    led1Off()
    led2Off()
    led3Off()
    led4Off()
    led5Off()
    led6Off()
    
def led1On():
    writePin(led_1_pin, True)    # write led_1_pin high
    if buzzerOnLevel == 1: setBeeperOn()

def led2On():
    writePin(led_2_pin, True)    # write led_2_pin high
    if buzzerOnLevel == 2: setBeeperOn()

def led3On():
    writePin(led_3_pin, True)    # write led_3_pin high
    if buzzerOnLevel == 3: setBeeperOn()

def led4On():
    writePin(led_4_pin, True)    # write led_4_pin high
    if buzzerOnLevel == 4: setBeeperOn()
    
def led5On():
    writePin(led_5_pin, True)    # write led_5_pin high
    if buzzerOnLevel == 5: setBeeperOn()

def led6On():
    writePin(led_6_pin, True)    # write led_6_pin high
    if buzzerOnLevel == 6: setBeeperOn()

def led1Off():
    writePin(led_1_pin, False)    # write led_1_pin high
    if buzzerOnLevel == 1: writePin(buzzerPin, False)

def led2Off():
    writePin(led_2_pin, False)    # write led_2_pin high
    if buzzerOnLevel == 2: writePin(buzzerPin, False)

def led3Off():
    writePin(led_3_pin, False)    # write led_3_pin high
    if buzzerOnLevel == 3: writePin(buzzerPin, False)

def led4Off():
    writePin(led_4_pin, False)    # write led_4_pin high
    if buzzerOnLevel == 4: writePin(buzzerPin, False)

def led5Off():
    writePin(led_5_pin, False)    # write led_5_pin high
    if buzzerOnLevel == 5: writePin(buzzerPin, False)

def led6Off():
    writePin(led_6_pin, False)    # write led_6_pin high
    if buzzerOnLevel == 6: writePin(buzzerPin, False)

def linkOn():
    writePin(linkPin, True)    # write blinkPin high

def linkOff():
    linkState = 0
    writePin(linkPin, False)    # write blinkPin high
    