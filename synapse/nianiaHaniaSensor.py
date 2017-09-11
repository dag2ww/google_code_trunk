from synapse.platforms import *

led_1_pin = 15 # phisical pin 11
led_2_pin = 37 # phisical pin 15
led_3_pin = 31 # phisical pin 17
led_4_pin = 30 # phisical pin 18
led_5_pin = 29 # phisical pin 19
led_6_pin = 28 # phisical pin 20

#getNoiseLevelTimer1 = 1
peakLevel = 0;

@setHook(HOOK_STARTUP)
def startupEvent():
    #0 = 256kbps
    #1 = 500kbps
    #2 = 1Mbps
    #3 = 2Mbps    
    setRadioRate(0)
    # Configure LED pin as an input
    setPinDir(led_1_pin, False)
    setPinDir(led_2_pin, False)
    setPinDir(led_3_pin, False)
    setPinDir(led_4_pin, False)
    setPinDir(led_5_pin, False)
    setPinDir(led_6_pin, False)

@setHook(HOOK_100MS)
def reportNoiseLevel():
    rpc('\x5E\x2E\xE5', 'handleNoiseLevel', getNoiseLevel())
 

#read the noise level peak and reset the peak store 
def getNoiseLevel():
    global peakLevel
    result = peakLevel
    peakLevel = 0
    return result

#read the noise level and save the peak (1Khz sampling is ok to detect the peak)
@setHook(HOOK_1MS)
def getNoiseLevelInternal():
    global peakLevel
    #global getNoiseLevelTimer1
    #if ((getNoiseLevelTimer1 % 2) == 0) :
    #    getNoiseLevelTimer1 = 1
    res = 0
    if readPin(led_6_pin) : res = 32 
    else:
        if readPin(led_5_pin) : res = 16 
        else:
            if readPin(led_4_pin) : res = 8
            else:
                if readPin(led_3_pin) : res = 4
                else:
                    if readPin(led_2_pin) : res = 2 
                    else:
                        if readPin(led_1_pin) : res = 1        
    
    if res > peakLevel : peakLevel = res
    ##else: 
    ##    getNoiseLevelTimer1=getNoiseLevelTimer1+1
    #return res        

#read the niose level abs value
#def readADCVoltageLevelNoise():
#    """returns voltage in 0.001 volt increments"""
#    refAdc = readAdc(ADC_CHAN_MIC)
#    inVolt = ((refAdc * 16) / 10) 
#    return inVolt

#Adjust by potentiometer the noise reference level
#def readADCVoltageLevelNoiseRef():
#    """returns voltage in 0.001 volt increments"""
#    refAdc = readAdc(ADC_CHAN_REF)
#    inVolt = ((refAdc * 16) / 10) 
#    return inVolt

