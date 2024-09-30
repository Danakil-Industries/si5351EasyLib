This is the third version of the code that is actually a library. 

Goals for this version
 * Make inputs of some functions ENUMs
   * spreadSpectrum(float, enum)
      * modify spreadSpectrum to work with enums and accept a spreadType of disabled
      * implemented and initial testing performed 9/30/2024 10:01
   * updateOutput(enum, float, enum)
      * implemented and initial testing performed 9/30/2024 09:45


Goals for the next version(s)
 * Add function updateOutputState(enum outputNumber, bool newState)
    * Turn on/off outputs 0-5 independently
    * Turn on/off all manually enabled outputs as close to simultaneously as possible
       * Create static uint8_t outputStates to remember which outputs have been set to a valid frequency by the user.
          * Avoids turning on outputs which the user doesn't want to turn on