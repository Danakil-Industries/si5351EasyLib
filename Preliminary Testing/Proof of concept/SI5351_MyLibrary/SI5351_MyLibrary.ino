/*
 * SI5351 initial proof of concept code
 * 
 * Written by: Kowolski
 * 
 * Notes: This is initial proof of concept code. 
 *         This code is not written with the intent to become a library. 
 *         It was written with the intent to get it working.
 *        I am also not the best programmer. 
 *         I recognize the fact that my code is likely hard to understand and/or is trash.
 *        This code has only been tested using an SI5351A in a 10-MSOP package because it's the only type I currently have.
 *         I'm using the Adafruit breakour board for those who will ask.
 *        With this implementation, outputs 0-6 should work well. Use of output 7 is... not currently recomended...
 *        KEEP PLL A SET TO 750 MHz!
 *         The frequency setting code for outputs 0-5 rely on this number
 *       
 * I release this code into the public domain for use with non-profit projects(e.g. hobby projects) and/or open source projects/products.
 *    Use of any of this code in any closed-source products requires explicit permission from Kowolski of Danakil Industries.
 *      Whomst can be contacted via my GitHub page: https://github.com/Danakil-Industries
 */

/*This code assumes: 
 *  There is only one SI5351
 *  It is an SI5351A variant in either the 10-msop or 20-QFN packages
 *  
 * 
 */

//#includes
#include <Wire.h>



//Defines
#define SI5351_minFreqOut 4 //minimum acceptable output frequency (kHz) for outputs 0-5
#define SI5351_minFreqOut67 20 //minimum acceptable output frequency (kHz) for outputs 6 and 7
#define SI5351_maxFreqOut 112000 //maximum acceptable output frequency (kHz)
#define SI5351_minMsOutFreq 500 //minimum acceptable multisynth output frequency for the output multisynth (kHz)
#define SI5351_maxMsOutFreq 150000 //max acceptable multisynth output frequency for the output multisynth (kHz)
#define SI5351_minPLLFreq 600000 //minimum acceptable PLL frequency (kHz)
#define SI5351_maxPLLFreq 900000 //maximum acceptable PLL frequency (kHz) 

#define msDenominator 1000000 // this is the MS denominator used for setting the feedbacks

//constants
const uint8_t SI5351_Addr = 0x60; //This is the I2C address of the SI5351

//floats
float si5351_outFreq[8]; // the desired output frequency in kHz. Set to 0 to turn off output.
uint8_t si5351_outPhase90[6]; // the desired initial output phase in 45 degree increments.
float SI5351_freqPllB = 0; // the frequency of PLL B


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // just for debugging
  si5351_init();
  
  //Enable the outputs for all the channels.
  si5351_WriteRegister(3, 0b11111000); //will eventually be its own function
  
}

void loop() {
  // test by sweeping the frequency
  for(float currFreq = 5; currFreq < 100000; currFreq++){
    si5351_outFreq[0] = currFreq; // set the frequency in kHz
    si5351_updateOutput(0);
    delay(10);
  }
}





void
SI5351_AdafruitBoardSetup()
{
  //Register 0 is read only
  si5351_WriteRegister(2, 0x10);//Register 2 is the mask bits.
  si5351_WriteRegister(1, 0x00);//Register 1 is the sticky status bits. Clear them all.
  si5351_WriteRegister(3, 0xFF);//Disable the outputs for all the channels.
  //registers 4-8 are restricted
  si5351_WriteRegister(9, 0xFF);//OEB doesn't control shit because it DNE on the 10-MSOP package, so disable its influence
  //Registers 10-14 are restricted
  si5351_WriteRegister(15, 0x00);//Leave the PLL input source register at the default (no division, both PLLs are referenced to the XO)

  //update registers 16-25
  //configure outputs 0-5 to be in fractional mode, utilizing the associated MultiSynth and PLL A, and a non-inverted output
  SI5351_updateClkCont(0, 0, 0, 0, 0x03); // configure output 0
  SI5351_updateClkCont(1, 0, 0, 0, 0x03); // configure output 1
  SI5351_updateClkCont(2, 0, 0, 0, 0x03); // configure output 2
  SI5351_updateClkCont(3, 0, 0, 0, 0x03); // configure output 3
  SI5351_updateClkCont(4, 0, 0, 0, 0x03); // configure output 4
  SI5351_updateClkCont(5, 0, 0, 0, 0x03); // configure output 5
  //configure outputs 6 and 7 to have non-inverted outputs, use their own multisynths, and use PLL B
  SI5351_updateClkCont(6, 0, 0, 0, 0x03); // configure output 6
  SI5351_updateClkCont(7, 0, 0, 0, 0x03); // configure output 7
  
  si5351_WriteRegister(149, 0x00);//make sure spread spectrum is off

  si5351_WriteRegister(177, 0x00); // turn off the PLL reset bits

  si5351_WriteRegister(183, 0b11010010); //set the load capacitance to 10 pF

  si5351_WriteRegister(187, 0x0b11010000); //enable fanout
}

void //Works!
SI5351_setPllFreq(uint8_t pllNumber, float desiredFreqMHz) 
{
  //First, make sure that the desired frequency is between (roughly) 600 and 900 MHz before changing anything
  if((desiredFreqMHz < 900.01) && (desiredFreqMHz > 599.99)){
    //find the overall multiplication ratio
    float requiredMultiplication = desiredFreqMHz / 25;

    
    //find the integer and decimal parts of the required multiplication ratio
    uint32_t feedbackInteger = 0;
    float decimalFeedback = requiredMultiplication;
    while(decimalFeedback >= 1){
      decimalFeedback = decimalFeedback - 1;
      feedbackInteger++;
    }
    
    
    //find the numerator part by multiplying decimalFeedback by the denominator
    uint32_t feedbackNumerator = (uint32_t)(decimalFeedback * msDenominator);

    //update the PLL's config by updating the associated MultiSynth
    Serial.print("Setting Pll ");
    Serial.println(pllNumber);
    si5351_updateMsParam((10 + pllNumber), feedbackInteger, feedbackNumerator, msDenominator, 0, 0);
    Serial.print("Set PLL ");
    Serial.print(pllNumber);
    Serial.print(" to ");
    Serial.print(desiredFreqMHz, 7);
    Serial.print(" MHz using by multiplying by ");
    Serial.print(requiredMultiplication, 5);
    Serial.print(" or ");
    Serial.print(feedbackInteger);
    Serial.print(" and ");
    Serial.print(feedbackNumerator);
    Serial.print(" / ");
    Serial.println(msDenominator);
  }
}


void //this function will (hopefully) be used to calculate the required parameters given the requested frequencies and phases, and configure the SI5351 accordingly.
si5351_autoUpdateOutputs()
{
  //WIP
}

bool
si5351_updateOutput(uint8_t outputNumber)
{
  //first, check if the requested frequency is within the allowed limits
  if((si5351_outFreq[outputNumber] >= SI5351_minFreqOut) && (si5351_outFreq[outputNumber] <= SI5351_maxFreqOut))
  {
    //if it's within the allowed limits, carry on

    
    if(outputNumber > 5){//Check if what is being updated is outputs 6 or 7, as they need to be treated differently.
      //Output 6 will be what sets PLL B's frequency, so its frequency should be pretty close. Output 7 will... have some issues...
      //there is probably some way to have output 6 control the frequency of PLL A, and output 7 control the freq of PLL B, but I can't figure it out.

      //find the rDiv value that will be used
      uint8_t rDiv = SI5351_maxR(si5351_outFreq[outputNumber]);

      //calculate the frequency that the corresponding MS needs to output
      float msOutFreq = ((float)rDiv) * si5351_outFreq[outputNumber];

      //if the output frequency isn't within bounds, abort
      if(si5351_outFreq[outputNumber] > SI5351_minFreqOut67){
        if(outputNumber == 6){
          //the output being addressed is output 6

          //find the maximum MS value for the output
          uint8_t intPart = 254;
          float currFreq = msOutFreq * ((float)intPart);
          while(currFreq < SI5351_minPLLFreq) // increase the required PLL frequency until it is above the minimum threashold
          {
            currFreq = msOutFreq * ((float)intPart);
            if(currFreq < SI5351_minPLLFreq){
              //if the frequency is too low...
              if(intPart == 6){
                //if the frequency can't increase any further, then abort.
                //turn off the output --------------------------------------------------------------------------
                return false;
              }
              else
              {
                //if the frequency can be increased, do so by subtracting 2 from the intPart
                intPart = intPart - 2;
              }
            }
          }
          //the integer part (the only part) if the MS divider has been found.

          //configure the MS
          si5351_updateMsParam(outputNumber, (uint32_t) intPart, 0, 1, rDiv, 0);
          
          //Set the frequency of PLL B
          SI5351_setPllFreq(1, (msOutFreq * ((float)intPart)));
          SI5351_freqPllB = (msOutFreq * ((float)intPart));

          //Make sure that output 7 is up to date
          si5351_updateOutput(7); //By making the function call itself. What fun!
          return true; 
        }
        else
        {
          //output being addressed is output 7

          //We don't really have much control over this output, so it will more or less land wherever it wants

          if(SI5351_freqPllB == 0)
          {
            // if PLL B is inactive, configure it optimally
            //find the maximum MS value for the output
            uint8_t intPart = 254;
            float currFreq = msOutFreq * ((float)intPart);
            while(currFreq < SI5351_minPLLFreq) // increase the required PLL frequency until it is above the minimum threashold
            {
              currFreq = msOutFreq * ((float)intPart);
              if(currFreq < SI5351_minPLLFreq){
                //if the frequency is too low...
                if(intPart == 6){
                  //if the frequency can't increase any further, then abort.
                  //turn off the output --------------------------------------------------------------------------
                  return false;
                }
                else
                {
                  //if the frequency can be increased, do so by subtracting 2 from the intPart
                  intPart = intPart - 2;
                }
              }
            }
            //the integer part (the only part) if the MS divider has been found.
  
            //configure the MS
            si5351_updateMsParam(outputNumber, (uint32_t) intPart, 0, 1, rDiv, 0);
            
            //Set the frequency of PLL B
            SI5351_setPllFreq(1, (msOutFreq * ((float)intPart)));
            SI5351_freqPllB = (msOutFreq * ((float)intPart));

            return true;
          }
          else
          {
            //find how close we can get to the requested frequency
            //WIP
          }
          
        }
      }
      else
        //turn off the output -----------------------------------------------------------------
        return false; // abort
        
    }
    else
    {//the addressed output is one of the highly flexible ones that make my life easier

      //find the rDiv value that will be used
      uint8_t rDiv = SI5351_minR(si5351_outFreq[outputNumber]);

      //calculate the frequency that the corresponding MultiSynth needs to output
      float msOutFreq = ((float)rDiv) * si5351_outFreq[outputNumber];

      //now find the required int and numer values to configure the MS
      //find the ratio
      float divRatio = 750000 / msOutFreq; //this is the part that relies on PLL A being at 750 MHz
      //find the int part of the ratio
      uint32_t intPart = 0;
      while(divRatio > 1)
      { // while there is still more integer to go...
        if(divRatio > 1){ // make certain that there is more integer
          divRatio--; // subtract 1 from the ratio...
          intPart++;  // and add it to the integer part
        }
      }
      //find the numer part
      uint32_t numerPart = (uint32_t)(divRatio * msDenominator);

      Serial.print("Set output ");
      Serial.print(outputNumber, DEC);
      Serial.print("'s frequency to ");
      Serial.print(si5351_outFreq[outputNumber],7);
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
      Serial.println(rDiv);

      si5351_updateMsParam(outputNumber, intPart, numerPart, msDenominator, rDiv, 0); // update the addressed multisynth and its divider
      

      si5351_WriteRegister((165 + outputNumber),si5351_outPhase90[outputNumber]); // Set the output phase
      
    }
    
    
    
    
    
    return true;
  }
  else
  {
    //if its not within the allowed limits, turn off the output
    //turn off the output ----------------------------------------------------------------------------------------------
    return false;
  }
}




uint8_t //works
SI5351_minR(float outputFreqKHz) //calculates the minimum output divider setting to keep the output's multisynth above the minimum threashold
{
  uint16_t currentR = 1;
  float currentFreq = 0;
  while(currentFreq < SI5351_minMsOutFreq) //while the frequency is too low
  {
    currentFreq = outputFreqKHz * ((float) currentR); // calculate what the requred MS freq would be at the current output divider setting
    if(currentFreq <= SI5351_minMsOutFreq){ // if the frequency is too low...
      if(currentR >= 128){
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

uint8_t // works
SI5351_maxR(float outputFreqKhz) //calculates the maximum output divider setting to keep the output's multisynth above the minimum threashold
{
  uint16_t currentR = 128;
  float currentFreq = 200000;
  while(currentFreq > SI5351_maxMsOutFreq)
  {
    currentFreq = outputFreqKhz * ((float)currentR);
    if(currentFreq > SI5351_maxMsOutFreq){ // if the frequency is too low...
      if(1 == currentR){
        //If the output divider is already at its minimum setting...
        //Serial.print("(breakoutTriggered)");
        currentFreq = SI5351_maxMsOutFreq - 1; // break out of the loop
      }
      else
      {
        //If the output divider isn't at its max setting...
        currentR = currentR / 2;
      }
    }
  }
  return (uint8_t) currentR;
}



void //Works
si5351_init()
{
  Wire.begin(); //Slide into her DMs as a host
  delay(100);
  
  SI5351_AdafruitBoardSetup();
  
  SI5351_setPllFreq(0, 750); // set PLL A's frequency to 750 MHz
  SI5351_setPllFreq(1, 900); // set PLL B's frequency to 900 MHz
}

void //Works
si5351_updateMsParam(uint8_t msNumber, uint32_t ms_intPart, uint32_t ms_numerPart, uint32_t ms_denomPart, uint8_t R_OutputDividerDec, uint8_t ms_divBy4En)
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
     R90, R92: MS6
     R91, R92: MS7
  */

  //Apply the equations from pg3 and pg5 of Skyworks AN619 to the int part and numer part if msNumber is not 6 or 7
  if((msNumber > 7)||(msNumber < 6)){
    //apply the correction to the integer part
    //Serial.print("Corrected integer ");
    //Serial.print(ms_intPart);
    ms_intPart = (128 * ms_intPart)  + ((uint32_t)(floor(128*(((float)ms_numerPart)/((float)ms_denomPart))))) - 512;
    //Serial.print(" to ");
    //Serial.println(ms_intPart);
    //Serial.print("Corrected numerator ");
    //Serial.print(ms_numerPart);
    ms_numerPart = (128 * ms_numerPart) - ((uint32_t)(((float)ms_denomPart)*(floor(128*(((float)ms_numerPart)/((float)ms_denomPart))))));
    //Serial.print(" to ");
    //Serial.println(ms_numerPart);
  }

  
  //Make sure there isn't extra data in the inputs that shouldn't be there and move the data around if necessarry to do so in this step
  ms_intPart = ms_intPart & 0x0003FFFF; // trim the ms integer part to 18 bits
  ms_numerPart = ms_numerPart & 0x000FFFFF; // trim the ms numerator part to 20 bits
  ms_denomPart = ms_denomPart & 0x000FFFFF; // trim the ms denominator part to 20 bits
  //R_OutputDividerDec doesn't get filtering
  ms_divBy4En = ms_divBy4En & 0x01; // trim the ms_divBy4En to 1 bit

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
  uint8_t formattedMs_divBy4En = (uint8_t)(ms_divBy4En * 3);
  
  //If the MS selected is for a PLL feedback, make sure the reserved bits are 0
  if ((10 == msNumber) || (11 == msNumber)) {
    formattedR_OutputDivider = 0;
    formattedMs_divBy4En = 0;
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
  uint8_t byte3 = ((uint8_t)(formattedR_OutputDivider << 4)) + ((uint8_t)(formattedMs_divBy4En << 2)) + formattedMs_intPart[2];
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
  if ((6 == msNumber) || (7 == msNumber)) {
    //the addressed MultiSynth is MS6 or MS7, which are setup differently than the rest
    si5351_WriteRegister(firstRegister, formattedMs_intPart[0]);
    uint8_t lowBitNumber = (uint8_t) ((msNumber - 6) << 2);
    si5351_updateBits(92, lowBitNumber, lowBitNumber + 2, formattedR_OutputDivider);
  }
  else
  {
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
  
}

void //Works
SI5351_updateClkCont(uint8_t outputNumber, uint8_t MS_INT, uint8_t MS_SRC, uint8_t MS_INV, uint8_t CLK_SRC)
{
  // This function is used to update registers 16-25, which are the clock control registers.
  //Yes, some values are hard coded. Sucks to be you if you want to change these values.

  uint8_t regNumberA = outputNumber + 16;// this selects which register, between registers 16 and 23, is associated with the intended output.
  uint8_t regNumberB = 24; //if the outputNumber is 3 or less, the desired register is register 24.
  if (outputNumber > 3) { // if the outputNumber is more than 3, the desired register is register 25.
    regNumberB = 25;
  }

  //update register A
  uint8_t regA_Value = 0b00000011; //preset register A to *my* default value
  regA_Value = regA_Value + ((MS_INT & 0x01) << 6);
  regA_Value = regA_Value + ((MS_SRC & 0x01) << 5);
  regA_Value = regA_Value + ((MS_INV & 0x01) << 4);
  regA_Value = (uint8_t)(regA_Value + ((CLK_SRC & 0x03) << 2));
  si5351_WriteRegister(regNumberA, regA_Value);

  //update register B
  outputNumber = (uint8_t) (outputNumber << 1);
  if (outputNumber > 6)
  {
    outputNumber = outputNumber - 8;
  }
  si5351_updateBits(regNumberB, outputNumber, outputNumber + 1, 0x00);
}

uint8_t //buggy???
si5351_updateBits(uint8_t regNumber, uint8_t lowBitNumber, uint8_t highBitNumber, uint8_t newValues)
{
  uint8_t currVal = si5351_ReadRegister(regNumber);

  uint8_t newVal = 0;
  for (uint8_t i = 0; i <= 7; i++) {
    if ((i < lowBitNumber) || (i > highBitNumber)) { // if we're not supposed to change this bit...
      newVal = newVal + SI5351_getBitVal(currVal, i); //then set the bit to what it was before
    }
    else { // if we are supposed to change this bit
      newVal = newVal + (uint8_t (SI5351_getBitVal(newValues, i))); // set the bit to the new value
    }
  }
  si5351_WriteRegister(regNumber, newVal);
}

uint8_t //buggy???
SI5351_getBitVal(uint8_t inputByte, uint8_t bitNumber)
{
  bitNumber = (uint8_t)(0x01 << bitNumber);
  return (inputByte & bitNumber);
}


uint8_t //works
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

void //works
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
