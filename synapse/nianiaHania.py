from synapse.platforms import *


ADC_PIN_MIC = 28 # chosen because it was the ADC pin closest to the corner
ADC_PIN_REF = 29 # chosen because it was the ADC pin closest to the corner
ADC_CHAN_MIC = 4
ADC_CHAN_REF = 5

diff=0

@setHook(HOOK_STARTUP)
def startupEvent():
    """This is hooked into the HOOK_STARTUP event"""
    setPinDir(ADC_PIN_MIC, False)
    setPinDir(ADC_PIN_REF, False)

#read the noise level diff to ref    
def getNoiseLevel():
    global diff
    diff = readADCVoltageLevelNoise() - readADCVoltageLevelNoiseRef();
    #Find the difference and converge the average
    return diff        

#read the niose level abs value
def readADCVoltageLevelNoise():
    """returns voltage in 0.001 volt increments"""
    refAdc = readAdc(ADC_CHAN_MIC)
    inVolt = (refAdc * 16 / 10) 
    return inVolt

#Adjust by potentiometer the noise reference level
def readADCVoltageLevelNoiseRef():
    """returns voltage in 0.001 volt increments"""
    refAdc = readAdc(ADC_CHAN_REF)
    inVolt = (refAdc * 16 / 10) 
    return inVolt