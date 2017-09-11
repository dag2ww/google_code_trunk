# Blink an LED once a second

from synapse.platforms import *

#Pins definitions
linkPin = 9 
led_1_pin = 15
led_2_pin = 12
led_3_pin = 8
led_4_pin = 37
led_5_pin = 23
led_6_pin = 31

#global variables
debugEnabled = False
linkState = 0
lastNoiseLevel = 0
getNoiseLevelTimer1 = 1

@setHook(HOOK_STARTUP)
def init():
    # Configure LED pin as an output
    setPinDir(linkPin, True)
    setPinDir(led_1_pin, True)
    setPinDir(led_2_pin, True)
    setPinDir(led_3_pin, True)
    setPinDir(led_4_pin, True)
    setPinDir(led_5_pin, True)
    setPinDir(led_6_pin, True)

    # Pulse the LEDs once during start-up
    pulsePin(led_1_pin, 200, True)
    sleep(1,1)
    pulsePin(led_2_pin, 200, True)
    sleep(1,1)
    pulsePin(led_3_pin, 200, True)
    sleep(1,1)
    pulsePin(led_4_pin, 200, True)
    sleep(1,1)
    pulsePin(led_5_pin, 200, True)
    sleep(1,1)
    pulsePin(led_6_pin, 200, True)    
    sleep(1,1)
    
@setHook(HOOK_10MS)
def requestRemoteNoiseLevel():
    global getNoiseLevelTimer1
    if ((getNoiseLevelTimer1 % 5) == 0) :
        getNoiseLevelTimer1 = 1     
        linkOff()
        #call callback on remote node which will call handleNoiseLevel on our node with result of remote getNoiseLevel
        rpc('\x5D\xA4\x3D', 'callback', 'handleNoiseLevel', 'getNoiseLevel')
    else: 
        getNoiseLevelTimer1=getNoiseLevelTimer1+1
        
def handleNoiseLevel(obj):
    linkOn()
    #split the value into 6 LEDs
    global lastNoiseLevel
    #lastNoiseLevel = abs(obj)
    lastNoiseLevel = obj
    if lastNoiseLevel > 100: led1On() 
    else: led1Off()
        
    if lastNoiseLevel > 250: led2On() 
    else: led2Off()

    if lastNoiseLevel > 400: led3On() 
    else: led3Off()

    if lastNoiseLevel > 700: led4On() 
    else: led4Off()

    if lastNoiseLevel > 1000: led5On() 
    else: led5Off()

    if lastNoiseLevel > 1300: led6On() 
    else: led6Off()

    rprint(lastNoiseLevel)
    
def getLastNoiseLevel():
    global lastNoiseLevel
    return lastNoiseLevel

#Helpers

#Print on the portal
def rprint(Obj):
    if debugEnabled:
        rpc('\x00\x00\x01', 'printIt', Obj)
    
def led1On():
    writePin(led_1_pin, True)    # write led_1_pin high

def led2On():
    writePin(led_2_pin, True)    # write led_2_pin high

def led3On():
    writePin(led_3_pin, True)    # write led_3_pin high

def led4On():
    writePin(led_4_pin, True)    # write led_4_pin high

def led5On():
    writePin(led_5_pin, True)    # write led_5_pin high

def led6On():
    writePin(led_6_pin, True)    # write led_6_pin high

def led1Off():
    writePin(led_1_pin, False)    # write led_1_pin high

def led2Off():
    writePin(led_2_pin, False)    # write led_2_pin high

def led3Off():
    writePin(led_3_pin, False)    # write led_3_pin high

def led4Off():
    writePin(led_4_pin, False)    # write led_4_pin high

def led5Off():
    writePin(led_5_pin, False)    # write led_5_pin high

def led6Off():
    writePin(led_6_pin, False)    # write led_6_pin high

def linkOn():
    writePin(linkPin, True)    # write blinkPin high

def linkOff():
    writePin(linkPin, False)    # write blinkPin high
    