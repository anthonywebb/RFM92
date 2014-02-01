RFM92W
=======

RFM92W/93W - Arduino Powered Low Power Long Range Transceiver

Warning, this code doesnt totally work just yet.  Proceed with caution.

DO NOT use the HopeRF documentation to program against, there are innacuracies in the translation.  Use the Semtech version instead.

LESSONS LEARNED
==================
* You must have an antenna (even just a piece of wire) on the antenna lead of the module.  Without one, even if the modules are in close proximity, you will not see data.
* The HopeRF datasheet is garbage.  We wasted an entire week writing to the wrong registers and reading the wrong values wondering why things didnt work.  Whoever translated that should be fired.
* If using LoRa mode, you will need to set 0x40 and 0x41 so that the proper DIO pins fire for you.  By default DIO0 will fire RxDone.  But, if you want to fire TxDone, you will need to set 0x40 to 0x40.  This really tripped us up.  First, the semtech docs tell you to refer to Table23, when in reality you should refer to Table17.  Second, when you are deciding what value to set that to you should check out the table, start with DIO0 and choose what you want to fire 00, 01, 11, etc then proceed the same through DIO3, this will give you 8 bits from DIO0 to DIO3, and the hex value is what you should store in the register.  So to get TxDone you are using 01000000 = 0x40
* This module is really small and soldering leads on the pins sucks.  We contemplated making breakout boards so we could use a breadboard, in the end we didnt and found that some of our problems were just due to our own soldering issues.  I would hihgly recommend using some sort of breakout board for testing.
  
OUTSTANDING QUESTIONS
=======================
* Sometimes on the receiving side, the module will simply reset the mode out of LoRa continuous receive mode (0x85) to non-LoRa standby mode (0x01), I am certain this is not our code changing this.  Particularly since the device must be in sleep mode to change from LoRa mode to something else.  This happens very rarely, for instance, I set the modules to run for 10 hours straight without it occuring.  But, I have seen it happen on numerous occasions.
* There are some phantom messages flowing in pretty regularly.  I have no idea what they are, I suspect some sort of heartbeat, or other notification.  Still trying to decide the best way identify and/or discard them.
* How improved is the distance over the RFM69HW?  I dont know yet, we are waiting on some boards so that we can test that.  I am hoping that it is greatly improved, we need to cover an open area of up to a couple miles which this module is apparently indeed capable of.
* Still a little vague on what size the trace should be on my board from the module to the antenna, I understand it should be short, but not sure what size would be best or if it even matters?
* We would like to have a unique "mac address" type ID for every module we have out there, I am wondering if there is some sort of unique hardware type identifier we can use?
