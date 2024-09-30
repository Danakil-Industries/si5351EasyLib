/*
   @file si5351EasyLib.cpp

   @mainpage si5351EasyLib

   @author Kowolski (Danakil Industries)

   @brief Easy driver for the SI5351A Clock Generator

   @section Notes
   
   SI5351 Easy Library v0.3 PRELIMINARY

   Written by: Kowolski

   Notes: This is the second version of the code that is implemented as a library.
          I am also not the best programmer.
           I recognize the fact that my code is likely hard to understand and/or is trash.
		   This is also the first true library that I have written.
          This code has only been tested using an SI5351A in a 10-MSOP package because it's the only type I currently have.
           I'm using the Adafruit breakout board for those who will ask.
           This also means that outputs 3-5 have not been tested yet. That said, it is highly likely that they will work just fine.
          KEEP PLL A SET TO 750 MHz!
           The frequency setting code relies on this frequency
		  The commented portions of code that print stuff to the serial port are left in for dev only. These will be removed in non-preliminary releases.

   I release this code for use with non-profit projects(e.g. hobby projects) and/or open source projects/products.
      Use of any of this code in any closed-source products requires explicit permission from Danakil Industries on a per product basis.
        Whomst can be contacted via github or email.
          My GitHub page https://github.com/Danakil-Industries
          My email address jeffbezoslegit69@gmail.com
        Failure to aquire permission for use in closed-source products may result in:
          My feelings being hurt
          Something else if I feel like it
*/





/*This code assumes:
    There is only one SI5351
    It is an SI5351A variant in either the 10-msop or 20-QFN packages
    PLL A is set to 750 MHz
    All outputs are based on PLL A
*/
#ifndef Arduino_h
  #include "Arduino.h"
#endif

#ifndef si5351EasyLib_h
  #include "si5351EasyLib.h"
#endif

#ifndef uint24_t
  #include <integer24.h>
#endif

#ifndef HardwareSerial_h
  #include <HardwareSerial.h>
#endif



/*
 *         Constructor
 */
si5351::si5351() // constructor
{
	//"explains what should happen when someone creates an instance of your class" - Arduino documentation
	//nothing to do...

  //The only purpose of this function is to initialize the library
}


/*
 * @brief Initializes the SI5351 and performs initial setup (call this function before doing anything else)
 * 
 * @section Return Value
 *  Returns a boolean value
 *  * true: The SI5351's error register indicates everything is fine after initialization
 *  * false: The SI5351's error register indicates a problem after initialization
 * 
 * @section Non-Brief:
 *   Initializes the I2C connection to the si5351A
 *   Updates some misc registers
 *   Updates clock control registers
 *   Disables spread spectrum
 *   Sets XO load capacitance to 10 pF
 *   Configure PLL A to run at 750 MHz
 *   Return false if the error register reveals an error
 */
bool
si5351::begin()
{
  
  Wire.begin(); //Slide into her DMs as a host
  Wire.setClock(400000); // use high speed mode so things don't take as long
  delay(100); //yes, this is probably unnecessarially long. However, this is here for a reason. I would put why here if I could remember, I just remember it was important


  //Clear previous 
  
  //Register 0 is read only
  writeRegister(2, 0x10);//Register 2 is the mask bits.
  writeRegister(1, 0x00);//Register 1 is the sticky status bits. Clear them all.
  writeRegister(3, 0xFF);//Disable the outputs for all the channels.
  //registers 4-8 are restricted
  writeRegister(9, 0xFF);//OEB doesn't control shit because it DNE on the 10-MSOP package, so disable its influence
  //Registers 10-14 are restricted
  writeRegister(15, 0x00);//Leave the PLL input source register at the default (no division, both PLLs are referenced to the XO)

  
  //update registers 16-25
  //configure outputs 0-5 
  SI5351_updateClkCont(0, 1); // configure output 0
  SI5351_updateClkCont(1, 1); // configure output 1
  SI5351_updateClkCont(2, 1); // configure output 2
  SI5351_updateClkCont(3, 1); // configure output 3
  SI5351_updateClkCont(4, 1); // configure output 4
  SI5351_updateClkCont(5, 1); // configure output 5
  //configure Outputs 6 and 7 to be off and set both PLLs into integer mode
  writeRegister(22, 0b11000000); // turn off clock 6 and put PLL A into integer mode
  writeRegister(23, 0b11000000); // turn off clock 7 and put PLL B into integer mode (even though it won't be used)

  writeRegister(149, 0x00);//make sure spread spectrum is off by default

  

  writeRegister(183, 0b11010010); //set the load capacitance to 10 pF

  writeRegister(187, 0x0b11010000); //enable fanout

  
  //SI5351_setPllFreq(0, 750);
  //set PLL A's frequency to 750 MHz in fractional mode using hard coded register values (saves space)
  writeRegister(26, 0x00); // set denominator bits [15:8] to 0
  writeRegister(27, 0x01); // set denominator bits [7:1] to 0 and bit 0 to 1
  writeRegister(28, 0x00); // set integer bits [17:16] to 0
  writeRegister(29, 0x0D); // set integer bits [15:8] to 0x0D
  writeRegister(30, 0x00); // set integer bits [8:0] to 0x00
  writeRegister(31, 0x00); // set numerator bits [19:16] and denominator bits [19:16] to 0
  writeRegister(32, 0x00); // set numerator bits [15:8] to 0
  writeRegister(33, 0x00); // set numerator bits [7:0] to 0

  
  delay(10); // give the PLL enough time to lock and clear an unlock condition if it can lock

  
  
  uint8_t SI5351_errorRegister = readRegister(1) & 0xE8; // read the initialization (error) register and get rid of the superflous errors
  if(SI5351_errorRegister > 7){// if the chip is an an error state, say something
    /*Serial.print("Warning: SI5351 error register is at value 0x");
    Serial.println(SI5351_errorRegister);*/
    return 0;
  }
  else
  {
    return 1;
  }
}




/*
 * @brief Configures the spread spectrum capabilities of the SI5351A
 * 
 * @param percent The spread width as a percentage of the PLL's frequency
 *         * Set to 0 to disable spread spectrum
 *         * Can be between -2.5 and -0.1 when downSpread is true
 *         * Can be between -1.5 and -0.1 or 0.1 and 1.5 when downSpread is false
 *        
 * @param downSpread Chooses the spread spectrum type
 *  * Down spread is used when true 
 *  * Center spread is used when false
 * 
 * @section Return Value
 * Returns a boolean value
 *  * true: The spread spectrum settings were accepted
 *  * false: The spread spectrum settings were rejected
 *  
 * @section Notes:
 *  * Spread spectrum is applied directly to PLL A and will thus effect all outputs.
 *  * Spread spectrum is disabled by default
 */
bool
si5351::spreadSpectrum(float percent, bool downSpread)
{
  //check if the requested spread is 0 (turn off)
  if(0 == percent)
  { //done?
    //turn off the spread spectrum
      writeRegister(149, 0x00); //set spread spec enable bit to 0 (along w/ the rest of the reg, but doesn't matter)
      writeRegister(22, 0b11000000); //set PLL A to int mode
      return true; // spread spectrum was sucessfully disabled
  }
  else
  {
    //initialize the variables that both modes share
    uint8_t SSUDP[2];
    float SSDN;
    uint8_t SSDN_P1[2];
    uint8_t SSDN_P2[2];
    uint8_t SSDN_P3[2];
    uint8_t SSUP_P1[2];
    uint8_t SSUP_P2[2];
    uint8_t SSUP_P3[2];
    float sscAmp = percent / 100;

    //Calculate and set shared parameters
    SSUDP[1] = 0x00; // based on the XO freq
    SSUDP[0] = 0xC6; // based on the XO freq
    SSDN = (320/33) * (sscAmp / (1+sscAmp)) * (2 - ((float)downSpread)); // also based on SSUDP //check if works
    SSDN_P1[1] = ((uint8_t)(((uint16_t)floor(SSDN))>> 8)) & 0x0F;
    SSDN_P1[0] = (uint8_t)(((uint16_t)floor(SSDN)) & 0x00FF);
    SSDN_P2[1] = (uint8_t)((((uint16_t)(32767 * (SSDN - floor(SSDN)))) & 0x7F00) >> 8);
    SSDN_P2[0] = (uint8_t)(32767 * (SSDN - floor(SSDN)));
    SSDN_P3[1] = 0x7F; //given in AN619
    SSDN_P3[0] = 0xFF; //given in AN619
    
        
    if(true == downSpread)
    { // done?
      //The spread type is down spread
      //is the requested spread within the allowable limits?
      if((percent >= SI5351_minSprSpecDown) && (percent <= SI5351_maxSprSpecDown))
      {//done?
        //The requested spread is within the requested limits

        writeRegister(22, 0b10000000);//set PLL A to fractional mode

        //create variables for spread spec params
          //shares all variables w/ center spread

        //calculate up spread params
          //given in datasheet for down spread
          SSUP_P1[1] = 0x00;
          SSUP_P1[0] = 0x00;
          SSUP_P2[1] = 0x00;
          SSUP_P2[0] = 0x00;
          SSUP_P3[1] = 0x00;
          SSUP_P3[0] = 0x01;

        //calculate new register values
        uint8_t reg149 = SSDN_P2[1] + 0x80;
        //register 150 is SSDN_P2[0]
        //register 151 is SSDN_P3[1] + 0x00
        //register 152 is SSDN_P3[0]
        //register 153 is SSDN_P1[0]
        uint8_t reg154 = ((uint8_t)(SSUDP[1] << 4)) + SSDN_P1[1]; // check if works
        //register 155 is SSUDP[0]
        //register 156 is SSUP_P2[1]
        //register 157 is SSUP_P2[0]
        //register 158 is SSUP_P3[1]
        //register 159 is SSUP_P3[0]
        //register 160 is SSUP_P1[0]
        //register 161 is SSUP_P1[1]

        
        //update registers
        writeRegister(149, reg149);
        writeRegister(150, SSDN_P2[0]);
        writeRegister(151, SSDN_P3[1] + 0x00);
        writeRegister(152, SSDN_P3[0]);
        writeRegister(153, SSDN_P1[0]);
        writeRegister(154, reg154);
        writeRegister(155, SSUDP[0]);
        writeRegister(156, SSUP_P2[1]);
        writeRegister(157, SSUP_P2[0]);
        writeRegister(158, SSUP_P3[1]);
        writeRegister(159, SSUP_P3[0]);
        writeRegister(160, SSUP_P1[0]);
        writeRegister(161, SSUP_P1[1]);
        
        return true; // signal that spread spectrum was sucessfully set
      }
      else
      {//done?
        //The requested spread is not within the requested limits
        //Do something, IDK
        return false; // signal that there was a problem
      }
    }
    else //-------------------------------------------
    { // done?
      //The spread type is center spread

      //is the requested spread within the allowable limits?
      if((percent >= SI5351_minSprSpecCent) && (percent <= SI5351_maxSprSpecCent))
      {//done?
        //The requested spread is within the requested limits

        writeRegister(22, 0b10000000);//set PLL A to fractional mode

        //calculate rest of the parameters
        float SSUP = (640/33) * (sscAmp / (1-sscAmp)); 
        SSUP_P1[1] = ((uint8_t)(((uint16_t)floor(SSUP))>> 8)) & 0x0F;
        SSUP_P1[0] = (uint8_t)(((uint16_t)floor(SSUP)) & 0x00FF);
        SSUP_P2[1] = (uint8_t)((((uint16_t)(32767 * (SSUP - floor(SSUP)))) & 0x7F00) >> 8);
        SSUP_P2[0] = (uint8_t)(32767 * (SSUP - floor(SSUP)));
        SSUP_P3[1] = 0x7F;
        SSUP_P3[0] = 0xFF;

        
        //calculate new register values
        uint8_t reg149 = SSDN_P2[1] + 0x80;
        //register 150 is SSDN_P2[0]
        //register 151 is SSDN_P3[1] + 0x00
        //register 152 is SSDN_P3[0]
        //register 153 is SSDN_P1[0]
        uint8_t reg154 = ((uint8_t)(SSUDP[1] << 4)) + SSDN_P1[1]; // check if works
        //register 155 is SSUDP[0]
        //register 156 is SSUP_P2[1]
        //register 157 is SSUP_P2[0]
        //register 158 is SSUP_P3[1]
        //register 159 is SSUP_P3[0]
        //register 160 is SSUP_P1[0]
        //register 161 is SSUP_P1[1]

        
        //update registers
        writeRegister(149, reg149);
        writeRegister(150, SSDN_P2[0]);
        writeRegister(151, SSDN_P3[1] + 0x80);
        writeRegister(152, SSDN_P3[0]);
        writeRegister(153, SSDN_P1[0]);
        writeRegister(154, reg154);
        writeRegister(155, SSUDP[0]);
        writeRegister(156, SSUP_P2[1]);
        writeRegister(157, SSUP_P2[0]);
        writeRegister(158, SSUP_P3[1]);
        writeRegister(159, SSUP_P3[0]);
        writeRegister(160, SSUP_P1[0]);
        writeRegister(161, SSUP_P1[1]);

        
        return true; // signal that spread spectrum was sucessfully set
      }
      else
      {//done?
        //The requested spread is not within the requested limits
        //Short VCC to ground or something, IDK
        return false; // signal that there was a problem
      }
    }
  }
}




/*
 * @brief Updates the frequency and phase of the given output. 
 * 
 * @section Return Value
 * Returns a boolean value
 * * true: The new settings were accepted
 * * false: The new settings were rejected. Also turns off output.
 * 
 * @param outputNumber The output number of the chip 
 *  * Can be between 0 and 5.
 * @param newFreq The desired frequency in kHz
 *  * Can be between 4 kHz and 112 MHz
 * @param newPhase The initial phase angle 
 *  * Can be 0, 90, 180, or 270.
 */
bool
si5351::updateOutput(uint8_t outputNumber, float newFreq, uint8_t newPhase)
{

  //first, check if the requested frequency is within the allowed limits
  if ((newFreq >= SI5351_minFreqOut) && (newFreq <= SI5351_maxFreqOut)) // 12 µs
  {
    //if it's within the allowed limits, carry on

    //find the rDiv value that will be used
    uint8_t rDiv = SI5351_minR(newFreq); // 72µs

    //calculate the frequency that the corresponding MultiSynth needs to output
    float msOutFreq = ((float)rDiv) * newFreq;

    //now find the required int and numer values to configure the MS
    //find the ratio
    float divRatio = 750000 / msOutFreq; //this is the part that relies on PLL A being at 750 MHz
    //find the int part of the ratio
    uint24_t intPart = 0;
    intPart = floor(divRatio); // this was taking between roughly 8 and 18 ms before changing how it was done.
    divRatio = divRatio - ((float)intPart);

    
    //find the numer part
    uint24_t numerPart = (uint24_t)(divRatio * msDenominator);

    /*Serial.print("Set output ");
    Serial.print(outputNumber, DEC);
    Serial.print("'s frequency to ");
    Serial.print(newFreq, 7);
    Serial.println(" kHz");
    Serial.print("  using an integer of ");
    Serial.print(intPart);
    Serial.println(",");
    Serial.print("  a numerator of ");
    Serial.print(numerPart);
    Serial.println(",");
    Serial.print("  a denominator of ");
    Serial.print(msDenominator);
    Serial.println(",");
    Serial.print("  and an output divider of ");
    Serial.println(rDiv);*/
    SI5351_updateMsParam(outputNumber, intPart, numerPart, msDenominator, rDiv); // update the addressed multisynth and its divider // taking about 1ms, which is understandable(?)

    writeRegister((165 + outputNumber), newPhase); // Set the output phase // taking about 128 µs

    /*Serial.print("Output ");
    Serial.print(outputNumber);
    Serial.println(" turned on");*/
    SI5351_updateClkCont(outputNumber, 0); // turn on the output // taking about 532 µs

    
    
  }
  else
  {
    //if its not within the allowed limits, turn off the output
    /*Serial.print("Output ");
    Serial.print(outputNumber);
    Serial.println(" turned off");*/
    SI5351_updateClkCont(outputNumber, 1); // turn off the output
    return false;
  }

  return true;
}



/*
 * @brief Calculates and returns the minimum allowable R output divider setting for the specified output frequency.
 * 
 * @param outputFreqKHz The frequency that the user wants to be leaving the clock generator IC
 * 
 * @section Return Value
 * Returns a uint8_t
 *  * returns 1, 2, 4, 8, 16, 32, 64, or 128 (the minimum allowable R divider value) if the specified frequency can be reached
 *  * returns 0 if the specified frequency cannot be reached
 *  
 * @section Notes:
 * Returned value is the minimum R output divider required to keep the output's multisynth frequency above the minimum allowable frequency.
 */
uint8_t
si5351::SI5351_minR(float outputFreqKHz)
{
  uint16_t currentR = 1;
  float currentFreq = 0;
  while (currentFreq < SI5351_minMsOutFreq) //while the frequency is too low
  {
    currentFreq = outputFreqKHz * ((float) currentR); // calculate what the requred MS freq would be at the current output divider setting
    if (currentFreq <= SI5351_minMsOutFreq) { // if the frequency is too low...
      if (currentR >= 128) {
        //If the output divider is already at its max setting...
        currentR = 0; //signal an error
        currentFreq = SI5351_minMsOutFreq + 1; // break out of the loop
      }
      else
      {
        //If the output divider isn't at its max setting...
        currentR = currentR * 2;
      }
    }
  }

  return (uint8_t) currentR;
}



/*
 * @brief Updates the multisynth parameters and R output divider settings
 *     
 * @param msNumber Which multisynth's parameters are being updated. 
 *  * Valid inputs are between 0 and 5, 10, and 11
 *    * 0-5 are for outputs 0-5 
 *    * 10 and 11 are for PLLs A and B respectively
 * 
 * @param ms_intPart The encoded integer part of the ms divider
 * 
 * @param ms_numerPart The encoded numerator part of the ms divider
 * 
 * @param ms_denomPart The encoded denominator part of the ms divider
 * 
 * @param R_OutputDividerDec The desired R output divider setting
 */
void //works // taking about 1.05 ms
si5351::SI5351_updateMsParam(uint8_t msNumber, uint24_t ms_intPart, uint24_t ms_numerPart, uint24_t ms_denomPart, uint8_t R_OutputDividerDec)
{
  /*Serial.print("Updating MS");
    Serial.print(msNumber);
    Serial.println(" parameters");*/
  /*ms param registers
     R26 - R33: MSNA (PLL A Feedback)
     R34 - R41: MSNB (PLL B Feedback)
     R42 - R49: MS0
     R50 - R57: MS1
     R58 - R65: MS2
     R66 - R73: MS3
     R74 - R81: MS4
     R82 - R89: MS5
  */

  //apply the correction to the integer part
  //Serial.print("Corrected integer ");
  //Serial.print(ms_intPart);
  ms_intPart = (128 * ms_intPart)  + ((uint24_t)(floor(128 * (((float)ms_numerPart) / ((float)ms_denomPart))))) - 512;
  //Serial.print(" to ");
  //Serial.println(ms_intPart);
  //Serial.print("Corrected numerator ");
  //Serial.print(ms_numerPart);
  ms_numerPart = (128 * ms_numerPart) - ((uint24_t)(((float)ms_denomPart) * (floor(128 * (((float)ms_numerPart) / ((float)ms_denomPart))))));
  //Serial.print(" to ");
  //Serial.println(ms_numerPart);


  //Make sure there isn't extra data in the inputs that shouldn't be there and move the data around if necessarry to do so in this step
  ms_intPart = ms_intPart & 0x03FFFF; // trim the ms integer part to 18 bits
  ms_numerPart = ms_numerPart & 0x0FFFFF; // trim the ms numerator part to 20 bits
  ms_denomPart = ms_denomPart & 0x0FFFFF; // trim the ms denominator part to 20 bits
  //R_OutputDividerDec doesn't get filtering
  

  //Update the msDivider ratio


  //Format the data into the required format
  //format the integer part
  uint8_t formattedMs_intPart[3];
  formattedMs_intPart[0] = (uint8_t) (ms_intPart & 0x0000FF);
  formattedMs_intPart[1] = (uint8_t) ((ms_intPart & 0x00FF00) >> 8);
  formattedMs_intPart[2] = (uint8_t) ((ms_intPart & 0xFF0000) >> 16);
  /*Serial.print("Formatted integer ");
    Serial.print(ms_intPart, DEC);
    Serial.print("(0x");
    Serial.print(ms_intPart, HEX);
    Serial.print(") into 0x");
    Serial.print(formattedMs_intPart[2], HEX);
    Serial.print(", 0x");
    Serial.print(formattedMs_intPart[1], HEX);
    Serial.print(", 0x");
    Serial.println(formattedMs_intPart[0], HEX);*/
  //format the numerator part
  uint8_t formattedMs_numerPart[3];
  formattedMs_numerPart[0] = (uint8_t) (ms_numerPart & 0x0000FF);
  formattedMs_numerPart[1] = (uint8_t) ((ms_numerPart & 0x00FF00) >> 8);
  formattedMs_numerPart[2] = (uint8_t) ((ms_numerPart & 0xFF0000) >> 16);
  /*Serial.print("Formatted numerator ");
    Serial.print(ms_numerPart, DEC);
    Serial.print("(0x");
    Serial.print(ms_numerPart, HEX);
    Serial.print(") into 0x");
    Serial.print(formattedMs_numerPart[2], HEX);
    Serial.print(", 0x");
    Serial.print(formattedMs_numerPart[1], HEX);
    Serial.print(", 0x");
    Serial.println(formattedMs_numerPart[0, HEX]);*/
  //format the denominator part
  uint8_t formattedMs_denomPart[3];
  formattedMs_denomPart[0] = (uint8_t) (ms_denomPart & 0x0000FF);
  formattedMs_denomPart[1] = (uint8_t) ((ms_denomPart & 0x00FF00) >> 8);
  formattedMs_denomPart[2] = (uint8_t) ((ms_denomPart & 0xFF0000) >> 16);
  /*Serial.print("Formatted denominator ");
    Serial.print(ms_denomPart, DEC);
    Serial.print("(0x");
    Serial.print(ms_denomPart, HEX);
    Serial.print(") into 0x");
    Serial.print(formattedMs_denomPart[2], HEX);
    Serial.print(", 0x");
    Serial.print(formattedMs_denomPart[1], HEX);
    Serial.print(", 0x");
    Serial.println(formattedMs_denomPart[0], HEX);*/
  //format the R output divider config bits
  uint8_t formattedR_OutputDivider;
  switch (R_OutputDividerDec) {
    case 1:
      formattedR_OutputDivider = 0x00;
      break;
    case 2:
      formattedR_OutputDivider = 0x01;
      break;
    case 4:
      formattedR_OutputDivider = 0x02;
      break;
    case 8:
      formattedR_OutputDivider = 0x03;
      break;
    case 16:
      formattedR_OutputDivider = 0x04;
      break;
    case 32:
      formattedR_OutputDivider = 0x05;
      break;
    case 64:
      formattedR_OutputDivider = 0x06;
      break;
    case 128:
      formattedR_OutputDivider = 0x07;
      break;
    default:
      break;
  }

  //If the MS selected is for a PLL feedback, make sure the reserved bits are 0
  if ((10 == msNumber) || (11 == msNumber)) {
    formattedR_OutputDivider = 0;
  }


  //Create the bytes to be written
  /* Structure of data to be sent in register order
     Byte 1: formattedMs_denomPart[1] (8 bits)
     Byte 2: formattedMs_denomPart[0] (8 bits)
     Byte 3: 0b0 (1 bit) + formattedR_OutputDivider (3 bits) + formattedMs_divBy4En (2 bits) + formattedMs_intPart[2] (2 bits)
     Byte 4: formattedMs_intPart[1] (8 bits)
     Byte 5: formattedMs_intPart[0] (8 bits)
     Byte 6: formattedMs_denomPart[2] (4 bits) + formattedMs_numerPart[2] (4 bits)
     Byte 7: formattedMs_numerPart[1] (8 bits)
     Byte 8: formattedMs_numerPart[0] (8 bits)
  */
  //first byte to set is formattedMs_denomPart[1], which is already in the correct format.
  //next byte to set is formattedMs_denomPart[0], which is already in the correct format.
  uint8_t byte3 = ((uint8_t)(formattedR_OutputDivider << 4)) + formattedMs_intPart[2];
  //next byte to set is formattedMs_intPart[1], which is already in the correct format.
  //next byte to set is formattedMs_intPart[0], which is already in the correct format.
  uint8_t byte6 = ((uint8_t)(formattedMs_denomPart[2] << 4)) + formattedMs_numerPart[2];
  //next byte to set is formattedMs_numerPart[1], which is already in the correct format.
  //final byte to set is formattedMs_numerPart[0], which is already in the correct format.


  //Select the first register's address
  uint8_t firstRegister;
  switch (msNumber) {
    case 0:
      firstRegister = 42;
      break;
    case 1:
      firstRegister = 50;
      break;
    case 2:
      firstRegister = 58;
      break;
    case 3:
      firstRegister = 66;
      break;
    case 4:
      firstRegister = 74;
      break;
    case 5:
      firstRegister = 82;
      break;
    case 6:
      firstRegister = 90;
      break;
    case 7:
      firstRegister = 91;
      break;
    case 10:
      firstRegister = 26;
      break;
    case 11:
      firstRegister = 34;
      break;
  }

  //Write the bytes to the correct registers
  //The rest of the multisynth registers follow the same order
  writeRegister(firstRegister, formattedMs_denomPart[1]);
  writeRegister(firstRegister + 1, formattedMs_denomPart[0]);
  writeRegister(firstRegister + 2, byte3);
  writeRegister(firstRegister + 3, formattedMs_intPart[1]);

  writeRegister(firstRegister + 4, formattedMs_intPart[0]);
  writeRegister(firstRegister + 5, byte6);
  writeRegister(firstRegister + 6, formattedMs_numerPart[1]);
  writeRegister(firstRegister + 7, formattedMs_numerPart[0]);

  
}


/*
 * @brief Enables/disables the specified output
 * 
 * @param outputNumber The output whose state is being changed
 *  * Can be between 0 and 5
 *  
 * @param outDisabled
 *  * Is the new state disabled?
 * 
 * @section Notes:
 *  Does more than just enable/disable the output
 *   * Updates the clock control bits for the specified output
 */
void //works // done?
si5351::SI5351_updateClkCont(uint8_t outputNumber, bool outDisabled) // taking about 532 µs
{
  // This function is used to update registers 16-25, which are the clock control registers.
  //Yes, some values are hard coded. Sucks to be you if you want to change these values.
  
  
  uint8_t regNumberA = outputNumber + 16;// this selects which register, between registers 16 and 23, is associated with the intended output.
  uint8_t regNumberB = 24; //if the outputNumber is 3 or less, the desired register is register 24.
  if (outputNumber > 3) { // if the outputNumber is more than 3, the desired register is register 25.
    
    regNumberB = 25;
  }

  //update register A
  uint8_t regA_Value = 0b00001111; //preset register A to *my* default value
  regA_Value = regA_Value + (((uint8_t)outDisabled) << 7); // bit 7 is the disable bit
  writeRegister(regNumberA, regA_Value);

  //update register B
  writeRegister(regNumberB, 0x00); // make the disable state low for all the outputs in this register

  //update the output disable bit
  uint8_t outputState = readRegister(3);
  outputState = (uint8_t)(outputState & ((0xFE << outputNumber)|(0x7F >> (7 - outputNumber))));
  outputState = outputState + ((uint8_t)(((uint8_t)outDisabled) << outputNumber));
  writeRegister(3, outputState);
  
}



//These two functions are self explanatory
uint8_t //works // done?
si5351::readRegister(uint8_t regNumber)
{
  //Tell the SI5351 what register we're interested in
  Wire.beginTransmission(SI5351_Addr);
  Wire.write(regNumber);
  Wire.endTransmission();
  //Get the value of that register and return it
  Wire.requestFrom(SI5351_Addr, 1); // request the SI5351 return 1 byte
  uint8_t dataRx = Wire.read();
  /*Serial.print("Found register ");
    Serial.print(regNumber, DEC);
    Serial.print(" to be 0x");
    Serial.println(dataRx, HEX);*/
  return dataRx;
}

void //works // done?
si5351::writeRegister(uint8_t regNumber, uint8_t newValue)
{
  Wire.beginTransmission(SI5351_Addr);
  Wire.write(regNumber);
  Wire.write(newValue);
  /*Serial.print("Wrote 0x");
    Serial.print(newValue, HEX);
    Serial.print(" to register ");
    Serial.println(regNumber);*/
  Wire.endTransmission();
}
