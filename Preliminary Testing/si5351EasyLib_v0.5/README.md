This is the fourth version of the code that is actually a library. 

Goals for this version
 * Make comments better
 * Modify updateOutput function to simplify the numerator and denomiator (might improve spurious performance)


Goals for the next version(s)
 * Finish testing spreadSpectrum function
 * Finish testing updateOutput function



Plan to do (abandoned goals)
 * Add function updateOutputState(enum outputNumber, bool newState)
    * Turn on/off outputs 0-5 independently
    * Turn on/off all manually enabled outputs as close to simultaneously as possible
       * Create static uint8_t outputStates to remember which outputs have been set to a valid frequency by the user.
          * Avoids turning on outputs which the user doesn't want to turn on