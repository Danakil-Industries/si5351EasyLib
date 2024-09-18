This project is a library that I am working on is an attempt to use the SI5351A I2C programmable clock generator to the maximum of its capabilities while also giving the programmer an interface that is easier to use than configuring 150+ registers.

Notes:
	This project is based on the assumption that a 25 MHz XO is being utilized. If you're not using a 25 MHz XO, too bad!
	This project no longer intends to support the B or C variants of the SI5351.
	This project is also based on the registers for the 10 MSOP and/or the 20 QFN packages. The registers for the 16 QFN version are mapped differently. If you intend to use the 16 QFN package, sucks to be you!
	Update and set do *not* mean the same thing in this document. 
		Set means to calculate the new value of a given register in the SI5351 and memorize the new value, but not update the register in the SI5351. 
		Update (or immedietly set) means to send new information to the SI5351.
		
Library v0.2 PRELININARY notes:
 * Outputs 6 and 7 are not supported
 * All enabled outputs are affected by spread spectrum
 * Outputs can be set anywhere between 4 kHz and 112 MHz

Goals:

	Create functions where you update a PLL's frequency rather than a multiplication ratio.
		Successfully implemented. Has been abstracted away.
		
	Create functions where you update an output's frequency and initial phase rather than needing to manually calculate new values for and update (*checks datasheet*) 12 fields across 10 registers.
		Successfully implemented
		
	Create functions that update the Output Enable Control (register 3) and OEB Pin Enable Control (register 9) registers.
		Has been abstracted away.
	
	Create functions that configure the PLLs.
		Successfully implemented. Has been abstracted away.
		
	Create functions that configure the CLK_ Control (registers 16-23 for CLK0-CLK7) and CLK Disable State (registers 24 and 25 for CLK3-CLK0 and CLK7-CLK4).
		Successfully implemented. Has been abstracted away.

	Create a function that updates the MultiSynth, Output divider, and initial phase offset registers for a given output using given values.
		Successfully implemented. Has been abstracted away.
	