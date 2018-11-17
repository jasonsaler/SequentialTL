# SequentialTL
Arduino code for making sequential turn signals in any car with 3 brake/turn signal lights per side.

Firstoff: This should be used with caution. Whenever working with electrical systems be sure to disconnect the battery. 
Some state laws may outlaw custom flashing patterns or tampering with brake and turn indicators. 
Check with local state laws before instalation and only for use at the users own risk. As the user agreement states, any damages, 
injury, prosecution, etc that arises from use or misuse of this code is upon the user and any consequences thereof are the 
responsibility of the user. I am not responsible for anything that results out of use of this program.

With that said, have fun, be safe and enjoy a cool custom light pattern.

To use this program an arduino board with at least 13 pins is necessary. There are multiple boards that will fill this need.
The Nano is a great option as it meets the minimum requirements, is small and is very affordable. Download or just open and copy the code from sequencial_turn_signal_4_9_3zxy.ino  

To upload the program onto the board, the arduino software must be used on a PC and the arduino plugged into the PC USB port.
Download the arduino software at: https://www.arduino.cc/en/Main/Software and also find out details and instructions how to 
properly use the software, install programs onto boards, etc. 

Other needs are an 8 relay Arduino relay module, 3 12V auto relays, wire and depending on the vehicle of choice, possibly other supplies. 
This was originally made and intended for use on the Pontiac Fiero, but may be used on other vehicles with 3 tail lights per side. 

Notes about setting up the board properly:
Output pins are 4-11
4: Left(driver's side) Marker lights
5: Left outmost taillight
6: Left Middle Taillight
7: Left Inmost taillight
8: Right(passenger side) inmost taillight
9: Right middle taillight
10: Right outmost taillight
11: Right marker lights

Input Pins are 1-3
1: Input from the right turn indicator switch
2: Input from the left turn indicator switch
3: Input from the brake light power, utilizes interrupt functionality
[Depricated] 12: Optional switch for going between sequential pattern to a standard blink pattern. 
    (If switch not desired then hook pin 12 to Arduino 5V output) 
> As of Version 4.8.7 mode change is made through a quick tap on the emergency flasher button/switch. No separate swithces are needed.
    
See the included diagram for a wiring example. (Note: the diagram is just to give a visual representation and is not to scale
 and furhtermore pin locations may differ as it was intended to be easy to read and follow wires. 
 Be sure to check the pin numbers as opposed to location.)
 
 

