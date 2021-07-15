#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"
#include <Servo.h>

//Servo variable
Servo myservo;  // create servo object to control a servo
int pos;//this variable holds the servo angle

// RTC declarations, variables and arrays
RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tues", "Wed", "Thu", "Fri", "Sat"};
char* mymonth2[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char lasttimeon[20];
char lasttime[20];

char lastdate;
char day_of_week[4] = {};
String mysecond, myminute, myhour, myhour2, myday, mymonth, myyear, mytime, mydate, am;
String datetime;
// RTC variables stop here

//buzzer
int buzzer = 13;
boolean buzzerflag = true;

/*Here are the various EEPROM addresses used in this code
address 0 to 3 - User Pin
address 30 to 33 - Master Pin
address 9 - ff (ff stores the system status whether on or off so it can be restored on startup)
address 12 - count (this holds the number of times the wrong pin has been entered to counter guessing)
address 35 to 55 - stores the power on time
address 57 to 77 - stores the power off time
   
*/

int stats = 1; // this is to monitor the status of the system so that the text message containing password can only act when the system is off
int xpx = 0;//this is used to ensure that the system will not lock if unmatching codes are set while setting codes.
int count;//this is to restrict the maximum number of trials of pin to three(3)
int xppl = 0;

String inputString;
long inputInt;

const char longD = 'Z'; // char returned if long press on 'D'
const unsigned int longPressDuration = 3000;


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = 1, d7 = 0;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


const byte numRows = 4;         //number of rows on the keypad
const byte numCols = 4;         //number of columns on the keypad

//The keymap for the keypad

char keymap[numRows][numCols] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

char keypressed;
char code[4];//change to 5 to compensate for a null terminator if char array has, not too sure at this point
//char code[] = {'1', '2', '1', '2'}; //The default code, you can change it or make it a 'n' digits one

char master[4]; //= {'1', '1', '1', '1'}; //This is the master pin to open the system if the set pin is forgotten


char mastercompare[sizeof(master)];
char master_buff1[sizeof(master)];  //Where the new key is stored
char master_buff2[sizeof(master)];  //Where the new key is stored again so it's compared to the previous one


char code_buff1[sizeof(code)];  //Where the new key is stored
char code_buff2[sizeof(code)];  //Where the new key is stored again so it's compared to the previous one

short a = 0, i = 0, s = 0, j = 0, v = 0, q = 0; //Variables used later

byte rowPins[numRows] = {9, 8, 7, 6}; //Rows 0 to 3 //if you modify your pins you should modify this too
byte colPins[numCols] = {5, 4, 3, 2}; //Columns 0 to 3


Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);


char holdkey;
unsigned long t_hold;

void setup() {
  rtc.begin();

  myKeypad.setHoldTime(longPressDuration);

  pinMode(13, OUTPUT);//set buzzer to output
  pinMode(12, OUTPUT);//set relay to output
  lcd.begin(20, 4);

  lcd.home();
  lcd.print(F("SYSTEM STARTUP..."));      //What's written on the LCD at startup you can change

  delay(1000);


  //EEPROM retrieval of the stored master pin
  EEPROM.get(30, master[0]);
  EEPROM.get(31, master[1]);
  EEPROM.get(32, master[2]);
  EEPROM.get(33, master[3]);

  //EEPROM retrieval of stored code
  EEPROM.get(0, code[0]);
  EEPROM.get(1, code[1]);
  EEPROM.get(2, code[2]);
  EEPROM.get(3, code[3]);

  EEPROM.get(12, count);

  int ff;
  EEPROM.get(9, ff);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RESTORING SYSTEM....");
  lcd.setCursor(0, 1);
  lcd.print("....TO LAST STATUS");
  delay(2000);

  if (ff == 0) {

    stats = 0;
    systemoff();
    myservo.attach(11);  // attaches the servo on pin 11 to the servo object
    myservo.write(90);

  }
  else if (ff == 1) {
    stats = 1;
    count = 0;
    systemon();
    myservo.attach(11);  // attaches the servo on pin 11 to the servo object
    myservo.write(20);
  }

}

void loop() {

  char key = readKeypad();

  if (key == 'C' && stats == 0) {                    // * to open the lock
    if (count <= 3)
    {
      lcd.clear();
      lcd.print("RESTORE POWER?");
      delay(1500);

      lcd.clear();
      lcd.print("ENTER PIN: ");            //Message to show
      lcd.setCursor(2, 2);
      lcd.print("PRESS A TO VERIFY");
      GetCode();                          //Getting code function
      if (a == sizeof(code)) {       //The GetCode function assign a value to a (it's correct when it has the size of the code array)
        restorepower();                   //restore power function if code is correct
        myservo.write(20);
        storepowerontime();
      }
      else {
        lcd.clear();
        lcd.print("WRONG PIN ENTERED!!!");          //Message to print when the code is wrong
        buzzthrice();
        count++;
        EEPROM.put(12, count);
        delay(2000);
        systemoff();             //Return to off mode it's the message do display when waiting
      }

    }

    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MAXIMUM TRIALS");
      lcd.setCursor(8, 1);
      lcd.print("EXCEEDED!!!");
      warningbuzz();

      delay(500);
      lcd.clear();
      lcd.print("ENTER MASTER PIN!!!");
      delay(2500);
      systemoff();             //Return to off mode it's the message do display when waiting
    }
  }

  if (key == 'B' && stats == 1) {                //To lock the system
    //The use of xpx value is seen in the if statement below
    ChangeCode();
    if (xpx == 0) {
      lcd.clear();
      lcd.print("POWER SHUT DOWN");                 //When done it returns to standby mode
      delay(2000);
      systemoff();
      myservo.write(90);

      storepowerofftime();
      stats = 0;
      EEPROM.put(9, stats);
      delay(100);
      successbuzz();

    }

    else if (xpx == 1) {
      lcd.clear();
      lcd.print("REVERTING SYSTEM...");
      lcd.setCursor(0, 1);
      lcd.print("...TO PREVIOUS STATE");
      delay(2000);
      systemon();
    }
  }

  if (key == 'D' && stats == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTER MASTER PIN:");
    lcd.setCursor(0, 2);
    lcd.print("PRESS A TO VERIFY");
    MasterEnter();
    if (a == sizeof(master)) {       //The GetCode function assign a value to a (it's correct when it has the size of the code array)
      MasterRestorePower();                   //Open lock function if code is correct
      storepowerontime();
    }
    else {
      lcd.clear();
      lcd.print("WRONG PIN ENTERED!!!");          //Message to print when the code is wrong
      buzzthrice();
      stats = 0;
      delay(1500);
      systemoff();
    }
  }

  if (key == '*') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LAST SHUT DOWN EVENT");
    lcd.setCursor(0, 1);
    lcd.print("OCCURED AT:");
    showpowerofftime();
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LAST POWER ON EVENT:");
    lcd.setCursor(0, 1);
    lcd.print("OCCURED AT:");
    showpowerontime();

    delay(3000);
    if (stats == 1) {
      systemon();
    }
    else
      systemoff();

  }

  if (key == '#') {
    if(buzzerflag){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM SOUND");
    lcd.setCursor(9, 1);
    lcd.print("DEACTIVATED");
    buzztwice();
    buzzerflag =!buzzerflag;
    delay(1200);
    }

    else if(!buzzerflag){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM SOUND");
    lcd.setCursor(11, 1);
    lcd.print("ACTIVATED");
    buzzerflag =!buzzerflag;
    buzztwice();
    delay(1200);
    }

    if (stats == 1) {
      systemon();
    }
    else
      systemoff();
  }

}


void stringconversions() { //the RTC does this to obtain the details of time and date from the function which holds them
  DateTime now = rtc.now();

  //Here, the time details are converted to strings to be printed later on
  myhour = String(now.hour(), DEC);
  if (myhour.toInt() > 12) {
    myhour2 = myhour.toInt() - 12;
    myhour2 = String(myhour2);
    am = "PM";
  }
  else if (myhour.toInt() == 0) {
    myhour2 = myhour.toInt() + 12;
    am = "AM";
  }
  else if (myhour.toInt() == 12) {
    myhour2 = myhour.toInt();
    am = "PM";
  }
  else {
    myhour2 = myhour;
    am = "AM";
  }
  myminute = String(now.minute(), DEC);
  mysecond = String(now.second(), DEC);

  //Here, the date details are also converted to strings to be printed too
  myday = String(now.day(), DEC);
  mymonth = String(now.month(), DEC);
  mymonth = mymonth2[mymonth.toInt() - 1];
  myyear = String(now.year(), DEC);
  myyear = String(myyear.toInt() - 2000);

  //Here, the strings for date and time are concatenated together
  mytime = myhour2 + ":" + myminute + ":" + mysecond + "" + am;
  mydate = myday + "/" + mymonth + "/" + myyear;

  datetime = mytime + "-" + mydate;

}


void storepowerontime() {
  stringconversions();

  //itt is to increment the address for each character stored in the EEPROM
  int atton = 35;//this does the same for the characters in the array being saved
  int itt;
  for (itt = 0; itt < 21; itt++) {
    EEPROM.put(atton, datetime[itt]);
    atton++;
  }
}


void storepowerofftime() {
  stringconversions();

  //itt is to increment the address for each character stored in the EEPROM
  int att = 57;//this does the same for the characters in the array being saved
  for (int itt = 0; itt < 21; itt++) {
    EEPROM.put(att, datetime[itt]);
    att++;
  }
}

void showpowerontime() {
  int apx = 0;
  int attton = 35;
  int itt;
  for (itt = 0; itt < 21; itt++) {
    lasttimeon[apx] = EEPROM.read(attton);
    apx++;
    attton++;
  }

  lcd.setCursor(0, 2);
  lcd.print(lasttimeon);
}


void showpowerofftime() {
  int apx = 0;
  int attt = 57;
  for (int itt = 0; itt < 21; itt++) {
    lasttime[apx] = EEPROM.read(attt);
    apx++;
    attt++;
  }

  lcd.setCursor(0, 2);
  lcd.print(lasttime);
}

char readKeypad()
{
  static bool dIsPressed = false;
  char key = NO_KEY;

  if (myKeypad.getKeys()) { // true if some activity
    for (int i = 0; key == NO_KEY && i < LIST_MAX; i++) {
      if (myKeypad.key[i].stateChanged ) {
        switch (myKeypad.key[i].kstate) {
          case PRESSED:
            if (myKeypad.key[i].kchar == 'D') {
              dIsPressed = true;
            } else key = myKeypad.key[i].kchar;
            break;
          case HOLD:
            if (myKeypad.key[i].kchar == 'D') {
              key = longD;

              changemaster();
              dIsPressed = false;
            }
            break;
          case RELEASED:
            if (dIsPressed && myKeypad.key[i].kchar == 'D') {
              key = 'D';
              dIsPressed = false;
            }
            break;
          default: break;
        } // end switch
      } // end state changed
    } // end for
  }
  return key;
}

void GetCode() {                 //Getting code sequence
  i = 0;                    //All variables set to 0
  a = 0;
  j = 0;

  while (keypressed != 'A') {                                   //The user press A to confirm the code otherwise he can keep typing
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {     //If the char typed isn't A and neither "nothing"
      lcd.setCursor(j, 1);                                 //This to write "*" on the LCD whenever a key is pressed it's position is controlled by j
      lcd.print("*");
      buzzonce();
      j++;
      if (keypressed == code[i] && i < sizeof(code)) {       //if the char typed is correct a and i increments to verify the next caracter
        a++;                                              //Now I think maybe I should have use only a or i ... too lazy to test it -_-'
        i++;
      }
      else
        a--;                                               //if the character typed is wrong a decrements and cannot equal the size of code []
    }
  }
  keypressed = NO_KEY;

}


void MasterEnter() {                 //Getting code sequence
  i = 0;                    //All variables set to 0
  a = 0;
  j = 0;

  while (keypressed != 'A') {                                   //The user press A to confirm the code otherwise he can keep typing
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {     //If the char typed isn't A and neither "nothing"
      lcd.setCursor(j, 1);                                 //This to write "*" on the LCD whenever a key is pressed it's position is controlled by j
      lcd.print("*");
      buzzonce();
      j++;
      if (keypressed == master[i] && i < sizeof(master)) {       //if the char typed is correct a and i increments to verify the next caracter
        a++;                                              //Now I think maybe I should have use only a or i ... too lazy to test it -_-'
        i++;
      }
      else
        a--;                                               //if the character typed is wrong a decrements and cannot equal the size of code []
    }
  }
  keypressed = NO_KEY;
  //count = 0;
}



void ChangeCode() {                     //Change code sequence
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TURN OFF POWER?");
  delay(800);
  lcd.setCursor(0, 2);
  lcd.print("CREATE A PREFERRED");
  lcd.setCursor(2, 3);
  lcd.print("   FOUR-DIGIT PIN");
  delay(2000);

  GetNewCode1();            //Get the new code
  GetNewCode2();            //Get the new code again to confirm it
  s = 0;
  for (i = 0 ; i < sizeof(code) ; i++) { //Compare codes in array 1 and array 2 from two previous functions
    if (code_buff1[i] == code_buff2[i])
      s++;                                //again this how we verifiy, increment s whenever codes are matching
  }
  if (s == sizeof(code)) {        //Correct is always the size of the array

    for (i = 0 ; i < sizeof(code) ; i++) {
      code[i] = code_buff2[i];       //the code array now receives the new code
      EEPROM.put(i, code[i]);        //And stores it in the EEPROM

    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PIN SET ");
    lcd.setCursor(5, 1);
    lcd.print("SUCCESSFULLY...");
    buzzonce();
    xpx = 0;
    delay(2000);
  }
  else {                        //In case the new codes aren't matching
    lcd.clear();
    lcd.print("PINS ARE NOT");
    lcd.setCursor(9, 1);
    lcd.print("MATCHING!!");
    buzzthrice();
    xpx = 1;
    delay(2000);
  }

}


void GetNewCode1() {
  i = 0;
  j = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER PIN: ");
  lcd.setCursor(0, 2);
  lcd.print("PRESS A TO CONFIRM");     //Press A keep showing while the top row print ***

  while (keypressed != 'A') {          //A to confirm and quits the loop
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {
      lcd.setCursor(j, 1);
      lcd.print("*");               //It shows * or can be changed to keypressed to show the keys
      buzzonce();
      code_buff1[i] = keypressed;   //Store caracters in the array
      i++;
      j++;
    }
  }
  keypressed = NO_KEY;
}

void GetNewCode2() {                        //This is exactly like the GetNewCode1 function but this time the code is stored in another array
  i = 0;
  j = 0;

  lcd.clear();
  lcd.print("VERIFY YOUR PIN");
  lcd.setCursor(6, 1);
  lcd.print("AND PRESS A...");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER PIN: ");
  lcd.setCursor(0, 2);
  lcd.print("PRESS A TO VERIFY");

  while (keypressed != 'A') {
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {
      lcd.setCursor(j, 1);
      lcd.print("*");
      buzzonce();
      code_buff2[i] = keypressed;
      i++;
      j++;
    }
  }
  keypressed = NO_KEY;
}

void restorepower() {            //Lock opening function open for 3s
  lcd.clear();
  lcd.print("PIN ACCEPTED!");       //With a message printed
  buzztwice();
  delay(1000);
  lcd.clear();
  lcd.print("POWER TURNING ON...");
  delay(1000);
  stats = 1;
  EEPROM.put(9, stats);
  systemstatus();
  successbuzz();
  count = 0;
  EEPROM.put(12, count);
}


void MasterRestorePower() {            //Lock opening function open for 3s
  lcd.clear();
  lcd.print("ADMINISTRATOR PIN...");       //With a message printed
  lcd.setCursor(8, 1);
  lcd.print("...ENTERED!");
  buzztwice();
  delay(20);
  buzztwice();
  delay(20);
  successbuzz();
  successbuzz();
  delay(1200);
  lcd.clear();
  lcd.print("RESTORING POWER...");
  delay(1000);
  systemstatus();
  myservo.write(20);

  stats = 1;
  EEPROM.put(9, stats);
  successbuzz();
  count = 0;
  EEPROM.put(12, count);
}


void changemaster() {
  v = 0;
  q = 0;
  s = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CHANGE MASTER PIN?");
  delay(1500);
  lcd.clear();
  lcd.print("ENTER OLD PIN:");
  lcd.setCursor(2, 2);
  lcd.print("PRESS A TO VERIFY");

  keypressed = NO_KEY; //After the first try, the variable "keypressed still has the value of "A"
  //This makes it to skip the while loop below and go to the else statement for "not accepted!"
  delay(50); //this is just a margin of error; giving the arduino little time to set the variable above to NO_KEY

  while (keypressed != 'A') {          //A to confirm and quit the loop
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A') {
      lcd.setCursor(q, 1);
      lcd.print(keypressed); //shows the old keys
      buzzonce();
      mastercompare[v] = keypressed;   //Store caracters in the array 'mastercompare'. we'll use this to compare against the existing master before a change can be effected
      v++;
      q++;
    }
  }

  for (i = 0; i < sizeof(master); i++) {
    if (mastercompare[i] == master[i])
      s++; //if s is then incremented up till 4, we know that each character was correct and it can proceed
  }

  if (s == sizeof(master)) {
    lcd.clear();
    lcd.print("OLD PIN CONFIRMED!");
    s = 0;
    delay(1200);
    keypressed = NO_KEY;

    GetNewMaster1();            //Get the new master pin
    GetNewMaster2();            //Get the new master pin again to confirm it
    s = 0;
    for (i = 0 ; i < sizeof(master) ; i++) { //Compare codes in array 1 and array 2 from two previous functions
      if (master_buff1[i] == master_buff2[i])
        s++;                                //again this how we verifiy, increment s whenever codes are matching
    }

    if (s == 4) {        //Correct is always the size of the array
      j = 0;

      for (i = 0 ; i < sizeof(master) ; i++) {
        j = i + 30;
        master[i] = master_buff2[i];       //the master array now receives the new code
        EEPROM.put(j, master[i]);        //And stores it in the EEPROM

      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MASTER PIN CHANGED");
      lcd.setCursor(5, 1);
      lcd.print("SUCCESSFULLY!!");
      s = 0;
      buzzonce();
      xpx = 0;
      delay(2000);
      if (stats == 1)
        systemon();
      else
        systemoff();
    }
    else {                        //In case the new codes aren't matching
      lcd.clear();
      lcd.print("PINS ARE NOT");
      lcd.setCursor(9, 1);
      lcd.print("MATCHING!!");
      s = 0;
      buzzthrice();
      xpx = 1;
      delay(2000);
      if (stats == 1)
        systemon();
      else
        systemoff();
    }
  }
  else {
    lcd.clear();
    lcd.print("NOT ACCEPTED!");
    //mastercompare[0] = '\0';
    buzzthrice();
    delay(1500);
    if (stats == 1)
      systemon();
    else
      systemoff();
  }
}


void GetNewMaster1() {
  i = 0;
  j = 0;
  lcd.clear();
  lcd.print("CREATE A MASTER PIN");   //tell the user to enter the new code and press A
  lcd.setCursor(0, 1);
  lcd.print("PRESS A TO CONFIRM");
  delay(1800);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER NEW PIN: ");
  lcd.setCursor(0, 2);
  lcd.print("PRESS A TO CONFIRM");     //Press A keep showing while the top row print ***

  while (keypressed != 'A') {          //A to confirm and quits the loop
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {
      lcd.setCursor(j, 1);
      lcd.print("*");               //On the new code you can show * as I did or change it to keypressed to show the keys
      buzzonce();
      master_buff1[i] = keypressed;   //Store caracters in the array
      i++;
      j++;
    }
  }
  keypressed = NO_KEY;
}

void GetNewMaster2() {                        //This is exactly like the GetNewCode1 function but this time the code is stored in another array
  i = 0;
  j = 0;

  lcd.clear();
  lcd.print("CONFIRM YOUR ENTRY");
  lcd.setCursor(7, 1);
  lcd.print("AND PRESS A");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONFIRM PIN: ");
  lcd.setCursor(0, 2);
  lcd.print("PRESS A TO CONFIRM");

  while (keypressed != 'A') {
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {
      lcd.setCursor(j, 1);
      lcd.print("*");
      buzzonce();
      master_buff2[i] = keypressed;
      i++;
      j++;
    }
  }
  keypressed = NO_KEY;
}

void buzzonce() {
  if (buzzerflag) {
    digitalWrite(buzzer, HIGH);
    delay(150);
    digitalWrite(buzzer, LOW);
  }
}

void buzztwice() {
  if (buzzerflag) {

    digitalWrite(buzzer, HIGH);
    delay(100);
    digitalWrite(buzzer, LOW);

    delay(20);
    digitalWrite(buzzer, HIGH);
    delay(100);
    digitalWrite(buzzer, LOW);
  }
}

void buzzthrice() {
  if (buzzerflag) {

    digitalWrite(buzzer, HIGH);
    delay(80);
    digitalWrite(buzzer, LOW);

    delay(20);
    digitalWrite(buzzer, HIGH);
    delay(80);
    digitalWrite(buzzer, LOW);

    delay(20);
    digitalWrite(buzzer, HIGH);
    delay(80);
    digitalWrite(buzzer, LOW);
  }
}

void successbuzz() {
  if (buzzerflag) {

    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(buzzer, LOW);
  }
}

void warningbuzz() {
  if (buzzerflag) {

    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    delay(30);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    delay(50);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    delay(30);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    delay(50);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    delay(30);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    delay(50);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    delay(30);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    delay(30);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    delay(30);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
  }
}


void systemstatus() {
  if (stats = 0) {
    systemoff();
    
  }
  else if (stats = 1) {
    systemon();
    
  }
}

void systemoff() {
  lcd.clear();
  lcd.print("SYSTEM STATUS: ON");
  lcd.setCursor(0, 2);
  lcd.print("POWER STATUS: OFF");

}

void systemon() {
  lcd.clear();
  lcd.print("SYSTEM STATUS: ON");
  lcd.setCursor(0, 2);
  lcd.print("POWER STATUS: ON");
}
