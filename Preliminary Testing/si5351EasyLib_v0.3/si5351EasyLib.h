/*
   SI5351 Easy Library v0.2 PRELIMINARY

   Written by: Kowolski

   Notes: This is the first version of the code that is implemented as a library.
          I am also not the best programmer.
           I recognize the fact that my code is likely hard to understand and/or is trash.
		   This is also the first true library that I have written.
          This code has only been tested using an SI5351A in a 10-MSOP package because it's the only type I currently have.
           I'm using the Adafruit breakout board for those who will ask.
           This also means that outputs 3-5 have not been tested yet. That said, it is highly likely that they will work just fine.
          KEEP PLL A SET TO 750 MHz!
           The frequency setting code relies on this frequency

   I release this code for use with non-profit projects(e.g. hobby projects) and/or open source projects/products.
      Use of any of this code in any closed-source products requires explicit permission from Danakil Industries on a per product basis.
        Whomst can be contacted via github or email.
          My GitHub page https://github.com/Danakil-Industries
          My email address jeffbezoslegit69@gmail.com
        Failure to aquire permission for use in closed-source products may result in:
          My feelings being hurt
*/

	


#ifndef si5351EasyLib_h
	#define si5351EasyLib_h //define the library
	
	

	
	#include "Arduino.h"
	#ifndef Wire_h
		#include <Wire.h> // include the wire library if it wasn't already included
	#endif


	//Defines
		#define SI5351_minFreqOut 4 //minimum acceptable output frequency (kHz)
		#define SI5351_maxFreqOut 112000 //maximum acceptable output frequency (kHz)
		#define SI5351_minMsOutFreq 500 //minimum acceptable multisynth output frequency for the output multisynth (kHz)
		#define SI5351_maxMsOutFreq 150000 //max acceptable multisynth output frequency for the output multisynth (kHz)

		#define SI5351_minSprSpecDown -2.5 //minimum acceptable down spread for spread spectrum operation (%)
		#define SI5351_maxSprSpecDown -0.1 //maximum acceptable down spread for spread spectrum operation (%)
		#define SI5351_minSprSpecCent -1.5 // minimum acceptable center spread for spread spectrum operation (%)
		#define SI5351_maxSprSpecCent 1.5 // maximum acceptable center spread for spread spectrum operation (%)

		#define msDenominator 1000000 // this is the MS denominator used for setting the feedbacks

		#define SI5351_Addr 0x60 //This is the I2C address of the SI5351

	
	
	//functions
	class si5351 //create the class si5351
	{	
		public:
			//functions that the programmer can call
			si5351();
			bool begin(); //Initializes the SI5351A
			
			bool spreadSpectrum(float percent, bool downSpread); //Enable/disable the spread spectrum, set the spread type, and width
			bool updateOutput(uint8_t outputNumber, float newFreq, uint8_t newPhase); //Used to set the output frequency and phase(90° increments only) of a given channel
			
			uint8_t readRegister(uint8_t regNumber);//Reads the value in a specified register and returns its value
			void writeRegister(uint8_t regNumber, uint8_t newValue); //Writes a specified value to a specified register. Use with caution, as this can interfere with the library/cause unexpected results
			
			
		private:
			//functions that only the library can call
			uint8_t SI5351_minR(float outputFreqKHz); //returns the minimum output divider value that can be used for a given output frequency
			void SI5351_updateMsParam(uint8_t msNumber, uint32_t ms_intPart, uint32_t ms_numerPart, uint32_t ms_denomPart, uint8_t R_OutputDividerDec);
			void SI5351_updateClkCont(uint8_t outputNumber, bool outDisabled);
	};
	

#endif
