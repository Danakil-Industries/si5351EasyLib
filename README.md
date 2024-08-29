This project is a library that I am working on is an attempt to use the SI5351 A/B/C I2C programmable clock generator to the maximum of its capabilities while also giving the programmer an interface that is easier to use than configuring 150+ registers.

Notes:
	This project is based on the assumption that a 25 MHz XO is being utilized. If you're not using a 25 MHz XO, too bad!
	This project is also based on the registers for the 10 MSOP and/or the 20 QFN packages. The registers for the 16 QFN version are mapped differently. If you intend to use the 16 QFN package, sucks to be you!
	Update and set do *not* mean the same thing in this document. 
		Set means to calculate the new value of a given register in the SI5351 and memorize the new value, but not update the register in the SI5351. 
		Update (or immedietly set) means to send new information to the SI5351.
	
Goals:

	**Create functions where you update a PLL's frequency rather than a multiplication ratio.
		Maybe return the actual frequency for error checking purposes too. The actual freq will be needed for accurately updating the output freq anyway.
		Something along the lines of uint24_t actualFreqPllA = pllAFreq(freqInKHz);
		
	**Create functions where you update an output's frequency and initial phase rather than needing to manually calculate new values for and update (*checks datasheet*) 12 fields across 10 registers.
		Maybe return the actual frequency for error checking purposes too.
		
	Create diagnostic functions that check and/or reset status and error registers.
		Return a boolean 1 if there is a fault.
			Create functions that update what bits in what registers are checked and/or considered faults.
			Create functions that update the Interrupt Status Mask register (register 2).
		Read the initialization status, PLL A unlock, PLL B unlock, and Loss Of CLKIN Signal bits from register 0.
		Read the System Init sticky bit, PLL A unlock sticky bit, PLL B unlock sticky bit, and Loss Of CLKIN sticky bits from register 1.
		
	Create functions that update the Output Enable Control (register 3) and OEB Pin Enable Control (register 9) registers.
		Create a function that updates the state of a given output (register 3) to a given state.
		Create a function that updates a given bit in the OEB Pin Enable register (register 9) to a given value.
		Create a function that immedietly sets the entire Output Enable Control register (register 3) to a given value.
	
	Create functions that configure the PLLs.
		Create a function that updates the PLL Input Source register (register 15).
		Probably other functions too.
		
	Create functions that configure the CLK_ Control (registers 16-23 for CLK0-CLK7) and CLK Disable State (registers 24 and 25 for CLK3-CLK0 and CLK7-CLK4).
		Create a function that sets a given bit in the Control Register for a given clock to a given value.
		Create a function that immedietly sets the Control Register for a given clock to a given value.
		Create a function that updates the CLK Control register using the value currently in the MCU's RAM.
	
	**registers 26-41**
	
	Create a function that updates the MultiSynth, Output divider, and initial phase offset registers for a given output using given values.
		Output 0: registers 42-49 and 165
		Output 1: registers 50-57 and 166
		Output 2: registers 58-65 and 167
		Output 3: registers 66-73 and 168
		Output 4: registers 74-81 and 189
		Output 5: registers 82-89 and 170
		Output 6: registers 90 and 92
		Output 7: registers 91 and 92
		
	**registers 93-164**
	
	Create a function that handles reseting a given PLL (update register 177).
	
	Create a function that updates the internal XO load capacitor values (update register 183).
	
	