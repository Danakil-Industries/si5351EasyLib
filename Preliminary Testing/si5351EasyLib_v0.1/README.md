This version of the code is intended to take the previous code, cut out the fat, and make it better suited for becoming a library.
	Support for outputs 6 and 7 will be removed in this version because they're a headache to implement and make the code a decent amount larger.
	User configurable PLLs will be removed. Instead, outputs 0-5 will use PLL A as their reference, which will operate at a fixed frequency of 750 MHz.
	Spread spectrum configuration will be added. The spread type and width will be configurable by the end user.
	
	This version of the code has been succeeded by si5351EasyLib_v0.2