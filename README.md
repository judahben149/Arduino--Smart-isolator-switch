# Arduino--Smart-isolator-switch
Here's my code for a gang isolator switch. It's a switch which requires a password to be turned off and can only  be turned 
back on when that password is entered. It is intended for use in an electric substation to lock up the gang isolator switch 
when some repairs or maintenance is to be carried out. This ensures proper safety of the utility workers in the substation.

In this project the isolator switch is represented using a knife switch which is controlled by a servo motor (MG-996R) connected to the arduino.
Other components include; a 4 by 4 matrix keypad, a DS1307 Real Time Clock module and a buzzer.

Here's how it works;
To turn off the isolator switch, key B is pressed. The prompt is followed to set up a password. The password is then confirmed and saved.
The isolator switch is then turned off.

To turn on the isolator switch again, key C is pressed. The password is entered and checked against. If correct, the switch is turned back on.


Other features of the system include:
* A Real Time Clock module that serves as a log by storing date and time details. This info can be retrieved by pressing key D.
* Protection is offered against multiple guesses such that the system locks up after 4 unsuccessful attempts to unlock it.
It then requires the use of the master password (which can also be changed) to unlock it.

