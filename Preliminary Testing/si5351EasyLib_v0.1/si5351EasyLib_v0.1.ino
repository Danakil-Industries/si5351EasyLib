/*
   SI5351 Easy Library v0.1 PRELIMINARY

   Written by: Kowolski

   Notes: This is the first refinement of the preliminary code.
           This code was written with the intent to improve upon the previous code.
           This code also "refines" outputs 6 and 7 out of existence because they're worse and need special treatment.
            You still have 6 highly flexable outputs, so don't complain.
          I am also not the best programmer.
           I recognize the fact that my code is likely hard to understand and/or is trash.
          This code has only been tested using an SI5351A in a 10-MSOP package because it's the only type I currently have.
           I'm using the Adafruit breakout board for those who will ask.
           This also means that outputs 3-5 have not been tested yet.
          KEEP PLL A SET TO 750 MHz!
           The frequency setting code for outputs 0-5 rely on this number

   I release this code for use with non-profit projects(e.g. hobby projects) and/or open source projects/products.
      Use of any of this code in any closed-source products requires explicit permission from Danakil Industries on a per product basis.
        Whomst can be contacted via github or email.
          My GitHub page https://github.com/Danakil-Industries
          My email address jeffbezoslegit69@gmail.com
        Failure to aquire permission for use in closed-source products may result in:
          My feelings being hurt
*/

/*This code assumes:
    There is only one SI5351
    It is an SI5351A variant in either the 10-msop or 20-QFN packages
    PLL A is set to 750 MHz
    All outputs are based on PLL A
*/

//#includes
#include <Wire.h>



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




void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200); // just for debugging
  
  si5351_init(); // initialize the clock generator
}

void loop() {
  // test by sweeping the frequency
  for (float currFreq = 5; currFreq < 100000; currFreq++) {
    si5351_updateOutput(1, currFreq, 0);//uint8_t output#, float frequency (kHz), uint8_t phase (_*90°)
    delay(1000);
  }
}


bool // TODO: Test
si5351_spreadSpectrum(float percent, bool downSpread)
{
  //check if the requested spread is 0 (turn off)
  if(0 == percent)
  { //done?
    //turn off the spread spectrum
      si5351_WriteRegister(149, 0x00); //set spread spec enable bit to 0 (along w/ the rest of the reg, but doesn't matter)
      si5351_WriteRegister(22, 0b11000000); //set PLL A to int mode
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
    
        
    if(0 == downSpread)
    { // done?
      //The spread type is down spread

      //is the requested spread within the allowable limits?
      if((percent >= SI5351_minSprSpecDown) && (percent <= SI5351_maxSprSpecDown))
      {//done?
        //The requested spread is within the requested limits

        si5351_WriteRegister(22, 0b10000000);//set PLL A to fractional mode

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
        si5351_WriteRegister(149, reg149);
        si5351_WriteRegister(150, SSDN_P2[0]);
        si5351_WriteRegister(151, SSDN_P3[1] + 0x00);
        si5351_WriteRegister(152, SSDN_P3[0]);
        si5351_WriteRegister(153, SSDN_P1[0]);
        si5351_WriteRegister(154, reg154);
        si5351_WriteRegister(155, SSUDP[0]);
        si5351_WriteRegister(156, SSUP_P2[1]);
        si5351_WriteRegister(157, SSUP_P2[0]);
        si5351_WriteRegister(158, SSUP_P3[1]);
        si5351_WriteRegister(159, SSUP_P3[0]);
        si5351_WriteRegister(160, SSUP_P1[0]);
        si5351_WriteRegister(161, SSUP_P1[1]);
        
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

        si5351_WriteRegister(22, 0b10000000);//set PLL A to fractional mode

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
        si5351_WriteRegister(149, reg149);
        si5351_WriteRegister(150, SSDN_P2[0]);
        si5351_WriteRegister(151, SSDN_P3[1] + 0x80);
        si5351_WriteRegister(152, SSDN_P3[0]);
        si5351_WriteRegister(153, SSDN_P1[0]);
        si5351_WriteRegister(154, reg154);
        si5351_WriteRegister(155, SSUDP[0]);
        si5351_WriteRegister(156, SSUP_P2[1]);
        si5351_WriteRegister(157, SSUP_P2[0]);
        si5351_WriteRegister(158, SSUP_P3[1]);
        si5351_WriteRegister(159, SSUP_P3[0]);
        si5351_WriteRegister(160, SSUP_P1[0]);
        si5351_WriteRegister(161, SSUP_P1[1]);

        
        return true; // signal that spread spectrum was sucessfully set
      }
      else
      {//done?
        //The requested spread is not within the requested limits
        //Do something, IDK
        return false; // signal that there was a problem
      }
    }
  }
}

bool //works //done?
si5351_updateOutput(uint8_t outputNumber, float newFreq, uint8_t newPhase)
{
  //first, check if the requested frequency is within the allowed limits
  if ((newFreq >= SI5351_minFreqOut) && (newFreq <= SI5351_maxFreqOut))
  {
    //if it's within the allowed limits, carry on
    
    //find the rDiv value that will be used
    uint8_t rDiv = SI5351_minR(newFreq);

    //calculate the frequency that the corresponding MultiSynth needs to output
    float msOutFreq = ((float)rDiv) * newFreq;

    //now find the required int and numer values to configure the MS
    //find the ratio
    float divRatio = 750000 / msOutFreq; //this is the part that relies on PLL A being at 750 MHz
    //find the int part of the ratio
    uint32_t intPart = 0;
    while (divRatio > 1)
    { // while there is still more integer to go...
      if (divRatio > 1) { // make certain that there is more integer
        divRatio--; // subtract 1 from the ratio...
        intPart++;  // and add it to the integer part
      }
    }
    //find the numer part
    uint32_t numerPart = (uint32_t)(divRatio * msDenominator);

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

    SI5351_updateMsParam(outputNumber, intPart, numerPart, msDenominator, rDiv); // update the addressed multisynth and its divider


    si5351_WriteRegister((165 + outputNumber), newPhase); // Set the output phase


    /*Serial.print("Output ");
    Serial.print(outputNumber);
    Serial.println(" turned on");*/
    SI5351_updateClkCont(outputNumber, 0); // turn on the output

    return true;
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
}

uint8_t //works // done?
SI5351_minR(float outputFreqKHz) //calculates the minimum output divider setting to keep the output's multisynth above the minimum threashold
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


bool //works
si5351_init()
{
  
  Wire.begin(); //Slide into her DMs as a host
  delay(100);


  //Clear previous 
  
  //Register 0 is read only
  si5351_WriteRegister(2, 0x10);//Register 2 is the mask bits.
  si5351_WriteRegister(1, 0x00);//Register 1 is the sticky status bits. Clear them all.
  si5351_WriteRegister(3, 0xFF);//Disable the outputs for all the channels.
  //registers 4-8 are restricted
  si5351_WriteRegister(9, 0xFF);//OEB doesn't control shit because it DNE on the 10-MSOP package, so disable its influence
  //Registers 10-14 are restricted
  si5351_WriteRegister(15, 0x00);//Leave the PLL input source register at the default (no division, both PLLs are referenced to the XO)

  
  //update registers 16-25
  //configure outputs 0-5 
  SI5351_updateClkCont(0, 1); // configure output 0
  SI5351_updateClkCont(1, 1); // configure output 1
  SI5351_updateClkCont(2, 1); // configure output 2
  SI5351_updateClkCont(3, 1); // configure output 3
  SI5351_updateClkCont(4, 1); // configure output 4
  SI5351_updateClkCont(5, 1); // configure output 5
  //configure Outputs 6 and 7 to be off and set both PLLs into integer mode
  si5351_WriteRegister(22, 0b11000000); // turn off clock 6 and put PLL A into integer mode
  si5351_WriteRegister(23, 0b11000000); // turn off clock 7 and put PLL B into integer mode (even though it won't be used)

  si5351_WriteRegister(149, 0x00);//make sure spread spectrum is off

  

  si5351_WriteRegister(183, 0b11010010); //set the load capacitance to 10 pF

  si5351_WriteRegister(187, 0x0b11010000); //enable fanout

  
  //SI5351_setPllFreq(0, 750);
  //set PLL A's frequency to 750 MHz in fractional mode using hard coded register values (saves space)
  si5351_WriteRegister(26, 0x00); // set denominator bits [15:8] to 0
  si5351_WriteRegister(27, 0x01); // set denominator bits [7:1] to 0 and bit 0 to 1
  si5351_WriteRegister(28, 0x00); // set integer bits [17:16] to 0
  si5351_WriteRegister(29, 0x0D); // set integer bits [15:8] to 0x0D
  si5351_WriteRegister(30, 0x00); // set integer bits [8:0] to 0x00
  si5351_WriteRegister(31, 0x00); // set numerator bits [19:16] and denominator bits [19:16] to 0
  si5351_WriteRegister(32, 0x00); // set numerator bits [15:8] to 0
  si5351_WriteRegister(33, 0x00); // set numerator bits [7:0] to 0

  
  delay(10); // wait to make sure the PLL has locked if it will lock

  
  
  uint8_t SI5351_errorRegister = si5351_ReadRegister(1) & 0xE8; // read the initialization (error) register and get rid of the superflous errors
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

void //works // done?
SI5351_updateMsParam(uint8_t msNumber, uint32_t ms_intPart, uint32_t ms_numerPart, uint32_t ms_denomPart, uint8_t R_OutputDividerDec)
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
  ms_intPart = (128 * ms_intPart)  + ((uint32_t)(floor(128 * (((float)ms_numerPart) / ((float)ms_denomPart))))) - 512;
  //Serial.print(" to ");
  //Serial.println(ms_intPart);
  //Serial.print("Corrected numerator ");
  //Serial.print(ms_numerPart);
  ms_numerPart = (128 * ms_numerPart) - ((uint32_t)(((float)ms_denomPart) * (floor(128 * (((float)ms_numerPart) / ((float)ms_denomPart))))));
  //Serial.print(" to ");
  //Serial.println(ms_numerPart);



  //Make sure there isn't extra data in the inputs that shouldn't be there and move the data around if necessarry to do so in this step
  ms_intPart = ms_intPart & 0x0003FFFF; // trim the ms integer part to 18 bits
  ms_numerPart = ms_numerPart & 0x000FFFFF; // trim the ms numerator part to 20 bits
  ms_denomPart = ms_denomPart & 0x000FFFFF; // trim the ms denominator part to 20 bits
  //R_OutputDividerDec doesn't get filtering

  //Update the msDivider ratio


  //Format the data into the required format
  //format the integer part
  uint8_t formattedMs_intPart[3];
  formattedMs_intPart[0] = (uint8_t) (ms_intPart & 0x000000FF);
  formattedMs_intPart[1] = (uint8_t) ((ms_intPart & 0x0000FF00) >> 8);
  formattedMs_intPart[2] = (uint8_t) ((ms_intPart & 0x00FF0000) >> 16);
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
  formattedMs_numerPart[0] = (uint8_t) (ms_numerPart & 0x000000FF);
  formattedMs_numerPart[1] = (uint8_t) ((ms_numerPart & 0x0000FF00) >> 8);
  formattedMs_numerPart[2] = (uint8_t) ((ms_numerPart & 0x00FF0000) >> 16);
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
  formattedMs_denomPart[0] = (uint8_t) (ms_denomPart & 0x000000FF);
  formattedMs_denomPart[1] = (uint8_t) ((ms_denomPart & 0x0000FF00) >> 8);
  formattedMs_denomPart[2] = (uint8_t) ((ms_denomPart & 0x00FF0000) >> 16);
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
  si5351_WriteRegister(firstRegister, formattedMs_denomPart[1]);
  si5351_WriteRegister(firstRegister + 1, formattedMs_denomPart[0]);
  si5351_WriteRegister(firstRegister + 2, byte3);
  si5351_WriteRegister(firstRegister + 3, formattedMs_intPart[1]);

  si5351_WriteRegister(firstRegister + 4, formattedMs_intPart[0]);
  si5351_WriteRegister(firstRegister + 5, byte6);
  si5351_WriteRegister(firstRegister + 6, formattedMs_numerPart[1]);
  si5351_WriteRegister(firstRegister + 7, formattedMs_numerPart[0]);

}

void //works // done?
SI5351_updateClkCont(uint8_t outputNumber, bool outDisabled)
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
  si5351_WriteRegister(regNumberA, regA_Value);

  //update register B
  si5351_WriteRegister(regNumberB, 0x00); // make the disable state low for all the outputs in this register

  //update the output disable bit
  uint8_t outputState = si5351_ReadRegister(3);
  outputState = (uint8_t)(outputState & ((0xFE << outputNumber)|(0x7F >> (7 - outputNumber))));
  outputState = outputState + ((uint8_t)(((uint8_t)outDisabled) << outputNumber));
  si5351_WriteRegister(3, outputState);
  
}


uint8_t //works // done?
si5351_ReadRegister(uint8_t regNumber)
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
si5351_WriteRegister(uint8_t regNumber, uint8_t newValue)
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
