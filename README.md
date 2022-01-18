# Arduino--Smart-Gang-Isolator-switch

Here's code for a gang isolator switch. It's a switch which requires a password to be turned off and can only  be turned 
back on when that password is entered. It is intended for use in an electric substation to lock up the gang isolator switch 
when some repairs or maintenance is to be carried out. This ensures proper safety of the utility workers in the substation.

In this project the isolator switch is represented using a knife switch which is controlled by a *servo motor (MG-996R)* connected to the Arduino.
Other components include; a __*4 by 4 matrix keypad*__, a __*DS1307 Real Time Clock module*__ and a __*buzzer*__.

## Here's how it works;
* To turn off the isolator switch, **key B** is pressed. 
* The prompt is followed to set up a password. 
* The password is then confirmed and saved.
* The isolator switch is then turned off.
----------------------------------------
* To turn on the isolator switch again, **key C** is pressed. 
* The password is entered and checked against. If correct, the switch is turned back on.
-----------------------------------
* To unlock with MASTER pin, **key D** is pressed
-----------------------------------
* To view the time and date of the most recent shut down and restore events, __key "*"__ is pressed
-----------------------------------
* To use the system with or without sound from the buzzer, the __"#" key__ is pressed


## Features of the system include:
* A Real Time Clock module that serves as a log by storing recent date and time details. This info can be retrieved by pressing key D.
* Protection is offered against multiple guesses such that the system locks up after **4 unsuccessful attempts** to unlock it.
It then compels the use of the master password (which can also be changed) to unlock it.

## More about the project:
* Check out full work ![here](https://drive.google.com/drive/folders/17kKo5f47Q3DIloETUw0UTkBpKQbuce93)
* Check out publication ![here](https://drive.google.com/file/d/1PZTvI8H6rhTr6Pn7KUztUm5rlEfYT4mB/view)

![1626733078812](https://user-images.githubusercontent.com/71103838/149950502-d2585652-034c-459e-a141-03de719bf0de.jpg)
