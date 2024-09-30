/*
   SI5351 Easy Library v0.3 PRELIMINARY

   Written by: Kowolski

   Notes: This is the second version of the code that is implemented as a library.
          I am also not the best programmer.
           I recognize the fact that my code is likely hard to understand and/or is trash.
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
//#includes
#include "si5351EasyLib.h"


//constructors
si5351 mySynth;//create instance of the library "mySynth"



  const float fmAmplitude = 1;
  const float fmCenter = 500;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // just for debugging
  
  mySynth.begin();
  //mySynth.spreadSpectrum(1.0,false);
}

void loop() { // run a couple demos


  //mySynth.updateOutput(1, 100, 0);
  //unsigned long startTime;
  //unsigned long endTime;
  //startTime = micros();
  for(float theta = 0; theta <= 360; theta++){//FM sweep
    delay(30);
    float currFreq = fmAmplitude * sin((theta * 3.1415) / 180) + fmCenter;
    float currFreq2 = currFreq;
    mySynth.updateOutput(0, currFreq, 0);
    mySynth.updateOutput(1, currFreq2, 0);
  }
  //endTime = micros();
  //Serial.print("sweepDuration:");
  //Serial.println(endTime - startTime);
}
