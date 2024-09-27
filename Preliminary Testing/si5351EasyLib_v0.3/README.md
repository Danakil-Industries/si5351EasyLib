This is the second version of the code that is actually a library. 
	The purpose of this version is to try and track down anything taking a long time and try to re-write it so that it takes less time.

Possible future ideas:
  * Make some inputs of public functions enums
     * I.E. ...
        * mySynth.updateOutput(enum out_0, float frequencyInkHz, enum phase_90);
        * spreadSpectrum(float percent, enum centerSpread);
  * Make readRegister(...) and writeRegister(...) private functions
  * Inline functions used only once
  * See about inlining other functions too if desperate for speed
  * Change to use uint24_t where possible
     * Create uint24_t typedef
  * Add user function to turn on/off outputs
     * Include ability to turn on/off all outputs w/ a single function call (aka "simultaneously" turn them on/off)
     * change updateOutput(...) to not automatically turn on output unless user function has set output state to ON