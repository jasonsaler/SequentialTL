# SequentialTL
Arduino code for making sequential turn signals in any car with 3 brake/turn signal lights per side.

Firstoff: This should be used with caution. Whenever working with electrical systems be sure to disconnect the battery. 
Some state lawsmay outlaw custom flashing patterns or tampering with brake and turn indicators. 
Check with local state laws before instalation and only for use at the users own risk. As the user agreement states, any damages, 
injury, prosecution, etc that arises from use or misuse of this codeis upon the user and any consequences thereof are the 
responsibility of the user. I am not responsible for anything that results out of use of this program.

With that said, have fun, be safe and enjoy a cool custom light pattern.

To use this program an arduino board with at least 13 pins is necessary. There are multiple boards that will fill this need.
The Uno is a great option as it meets the minimum requirements and is very affordable.

To upload the program onto the board, the arduino software must be used on a PC and the arduino plugged into the PC USB port.
Download the arduino software at: https://www.arduino.cc/en/Main/Software and also find out details and instructions how to 
properly use the software, install programs onto boards, etc. 

Other needs are an 8 relay Arduino relay module, 3 12V auto relays, wire and depending on the vehicle of choice, possibly other supplies. 
This was originally made and intended for use on the Pontiac Fiero, but may be used on other vehicles with 3 tail lights per side. 

Notes about setting up the board properly:
Output pins are 1-8
1: Left(driver's side) Marker lights
2: Left outmost taillight
3: Left Middle Taillight
4: Left Inmost taillight
5: Right(passenger side) inmost taillight
6: Right middle taillight
7: Right outmost taillight
8: Right marker lights

Input Pins are 9-12
9: Input from the right turn indicator switch
10: Input from the left turn indicator switch
11: Input from the brake light power
12: Optional switch for going between sequential pattern to a standard blink pattern. 
    (If switch not desired then hook pin 12 to Arduino 5V output)
    
See the included diagram for a wiring example. (Note: the diagram is just to give a visual representation and is not to scale
 and furhtermore pin locations may differ as it was intended to be easy to read and follow wires. 
 Be sure to check the pin numbers as opposed to location.)
 
 

