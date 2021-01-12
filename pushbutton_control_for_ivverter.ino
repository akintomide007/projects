#include <EEPROM.h>
#include <stdlib.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(0, 1, 2, 3, 4, 5);

int page_counter=1 ;       //To move beetwen pages
long tempPassword;  //Variable to store the input password
String pass = "";   //Variable to store the input password
int y;
const int  Up_buttonPin   = 10;    // the pin that the pushbutton is attached to
const int  Down_buttonPin = 11;
const int Reset_buttonPin = 12;
int charger_sense = 6;
int inverter_sense = 7;
int voltage_charger;
int voltage_inverter;
int buttonPushCounter = 200;   // counter for the number of button presses
int up_buttonState = 0;         // current state of the up button
int up_lastButtonState = 0;     // previous state of the up button
int down_buttonState = 0;         // current state of the up button
int down_lastButtonState = 0;     // previous state of the up button
int Reset_buttonState = HIGH;
int Reset_lastButtonState = 0;
bool bPress = false;
bool uPress = false;
bool rPress = false;
int period = 5000;
unsigned long time_now = 0;
int buttonStatePrevious = LOW;                      // previousstate of the switch
unsigned long minButtonLongPressDuration = 4000;    // Time we wait before we see the press as a long press
unsigned long buttonLongPressMillis;                // Time in ms when we the button was pressed
bool buttonStateLongPress = false;                  // True if it is a long press
const int intervalButton = 50;                      // Time between two readings of the button state
unsigned long previousButtonMillis;                 // Timestamp of the latest reading
unsigned long buttonPressDuration;                  // Time the button is pressed in ms
unsigned long currentMillis;          // Variabele to store the number of milleseconds since the Arduino has started

unsigned long previousMillis = 0;
unsigned long interval = 10000; //Desired wait time 10 seconds


#define SAMPLES 300   //Number of samples you want to take everytime you loop
#define overload A4
#define fan A0
#define bat A3
#define ACS_Pin A1    //ACS712 data pin analong input
#define acvolt A2 //ACS712 data pin analong input
#define NUM_SAMPLES 10
#define THERMISTORPIN A5 // resistance at 25 degrees C
#define THERMISTORNOMINAL 10000 // temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25 // how many samples to take and average, more takes longer
#define NUMSAMPLES 5 // but is more 'smooth' 
#define BCOEFFICIENT 3950 // The beta coefficient of the thermistor (usually 3000-4000)
#define SERIESRESISTOR 10000 // the value of the 'other' resistor
int heat = 0;
int samples[NUMSAMPLES];
int sum = 0;                    // sum of samples taken
unsigned char sample_count = 0; // current sample number
float voltage = 0.0;            // calculated voltage
float voltage1 = 0.00;
float High_peak,Low_peak;         //Variables to measure or calculate
float Amps_Peak_Peak, Amps_RMS;
float a;
float Totalpower = 0.0;
float errorTotalcurrent = 0.0;
int Totalacvoltage = 0;
int Totalcurrent = 0;
int degree = 0;
int temp = 0;
int val;
int maxpower= 1;
int value;
int stream = 0;
bool over;
bool wait;
byte buttonpressed = 9;                // how many times the button has been pressed
byte lastpresscount = 0;               // to keep track of last press count
byte pressed = 5;                // how many times the button has been pressed
byte presscount = 0;               // to keep track of last press count
byte i = 5;
byte x = 0;
byte z = 9;


void setup(){
  pinMode(Up_buttonPin , INPUT_PULLUP);
  pinMode(Down_buttonPin , INPUT_PULLUP);
  pinMode(Reset_buttonPin , INPUT_PULLUP);
  pinMode(charger_sense , INPUT);
  pinMode(inverter_sense , INPUT);
  pinMode(fan , OUTPUT);
  pinMode(ACS_Pin,INPUT);  //Define the pin mode
  pinMode(THERMISTORPIN,INPUT);  //Define the pin mode
  pinMode(acvolt ,INPUT);  //Define the pin mode
  pinMode(bat,INPUT);  //Define the pin mode
  pinMode(overload,OUTPUT);  //Define the pin mode
  pinMode(fan,OUTPUT);  //Define the pin mode
  lcd.begin(16,2);
  lcd.clear();  
  seememory();
 }

void dcvoltage(){
   while (sample_count < NUM_SAMPLES) {
        sum += analogRead(bat);
        sample_count++;
        delay(50);
    }
    voltage = (analogRead(bat) * 4.86) / 1024.0;
     voltage1 = (voltage * 101);   //107.6003761
    sample_count = 0;
    sum = 0;
 }

void temperature(){
  uint8_t i;
  float average;  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(50);
  }
  average = 0; // average all the samples out
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  heat = steinhart;  
 }

void acvoltage(){
  int sensorValue = analogRead(acvolt);
  int voltage = sensorValue * (2500.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 250V)
  Totalacvoltage = voltage;
}

void accurrent(){
int cnt;            //Counter
  High_peak = 0;      //We first assume that our high peak is equal to 0 and low peak is 1024, yes inverted
  Low_peak = 1024;  
      for(cnt=0 ; cnt<SAMPLES ; cnt++)          //everytime a sample (module value) is taken it will go through test
      {
        float ACS_Value = analogRead(ACS_Pin); //We read a single value from the module        
        if(ACS_Value > High_peak)                //If that value is higher than the high peak (at first is 0)
            {
              High_peak = ACS_Value;            //The high peak will change from 0 to that value found
            }        
        if(ACS_Value < Low_peak)                //If that value is lower than the low peak (at first is 1024)
            {
              Low_peak = ACS_Value;             //The low peak will change from 1024 to that value found
            }
      }                                        //We keep looping until we take all samples and at the end we will have the high/low peaks values      
  Amps_Peak_Peak = High_peak - Low_peak;      //Calculate the difference
  Amps_RMS = (Amps_Peak_Peak-23);     //Now we have the peak to peak value normally the formula requires only multiplying times 0.3536

  errorTotalcurrent =  Amps_RMS - 0.95;          // subtracting error of 0.95 which can varry with design
    if (errorTotalcurrent >= 0)
    {
      Totalcurrent == errorTotalcurrent;
    }
    else
    {
      Totalcurrent == 0.0;
    }
    
  }

void power() {
  Totalpower = Totalacvoltage * Totalcurrent ;
 }

void shutdown1() {
  if (Totalpower >= val  || heat >= 60) 
  {
  digitalWrite(overload, HIGH);
  }
  else{
      digitalWrite(overload, LOW);
    }
}

void dcvoltage_charger(){
    voltage_charger = digitalRead(charger_sense);
    }

 void dcvoltage_inverter(){
    voltage_inverter = digitalRead(inverter_sense);
    }

void startfan() {
   dcvoltage_charger();
   dcvoltage_inverter();
    if (voltage_charger || voltage_inverter == HIGH ) 
   {
    digitalWrite(fan, HIGH);
 }
if(voltage_charger == LOW){
  digitalWrite(fan, LOW);
}
if(voltage_inverter == LOW){
  digitalWrite(fan, LOW);
}
}

void fault() {  
  if(Totalpower >= val){
  lcd.clear();
  delay(10);
  lcd.setCursor(0,0);
  lcd.print("FAULT:");
  lcd.setCursor(7,0);
  lcd.print("OVERLOAD");
  }
    
 if(heat >= 60)  {
  lcd.clear();
  delay(10);
  lcd.setCursor(0,1);
  lcd.print("FAULT:");
  lcd.setCursor(7,1);
  lcd.print("HIGHTEMP");
  }
  }

void EEPROMWritelong(int maxpower, long tempPassword){
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (tempPassword & 0xFF);
      byte three = ((tempPassword >> 8) & 0xFF);
      byte two = ((tempPassword >> 16) & 0xFF);
      byte one = ((tempPassword >> 24) & 0xFF);
      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(maxpower, four);
      EEPROM.write(maxpower + 1, three);
      EEPROM.write(maxpower + 2, two);
      EEPROM.write(maxpower + 3, one);
      }

long EEPROMReadlong(long maxpower){
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(maxpower);
      long three = EEPROM.read(maxpower + 1);
      long two = EEPROM.read(maxpower + 2);
      long one = EEPROM.read(maxpower + 3);
      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }

void seememory() {
     val = EEPROMReadlong(0);    
    delay(50);
}

void readmemory() {  
 int value = EEPROM.read(temp);
    delay(50);
}

void checkUp(){
wait = 1;
  up_buttonState = digitalRead(Up_buttonPin);  // compare the buttonState to its previous state
  if (up_buttonState != up_lastButtonState) {
    // if the state has changed, increment the counter
    if (up_buttonState == LOW) {
        uPress = true;
      // if the current state is HIGH then the button went from off to on:
      if(buttonPushCounter < 3500)
      {
         //Serial.println("oagfsdfan");
      buttonPushCounter = buttonPushCounter + 200;
      }
    } 
    delay(50);
  }
   up_lastButtonState = up_buttonState;  // save the current state as the last state, for next time through the loop
}

void checkDown(){
  wait = 1;
  down_buttonState = digitalRead(Down_buttonPin);
  // compare the buttonState to its previous state
  if (down_buttonState != down_lastButtonState) {
    // if the state has changed, increment the counter
    if (down_buttonState == LOW) {
        bPress = true;
      // if the current state is HIGH then the button went from off to on:
      if(buttonPushCounter > 200)
      {
       buttonPushCounter = buttonPushCounter - 200 ;
      }
    }
   delay(50);
  }
  down_lastButtonState = down_buttonState;  // save the current state as the last state, for next time through the loop
}

void checkReset() {
   if(currentMillis - previousButtonMillis > intervalButton) {       
   int Reset_buttonState = digitalRead(Reset_buttonPin);    
   if (Reset_buttonState == HIGH && buttonStatePrevious == LOW && !buttonStateLongPress) {
      buttonLongPressMillis = currentMillis;
      buttonStatePrevious = HIGH;
    }
    buttonPressDuration = currentMillis - buttonLongPressMillis;
    if (Reset_buttonState == HIGH && !buttonStateLongPress && buttonPressDuration >= minButtonLongPressDuration) 
    {
      buttonStateLongPress = true;
      tempPassword = buttonPushCounter;
       long maxpower=0;
       EEPROMWritelong(maxpower, tempPassword);
       delay(50);
       seememory();
       wait = 0;
    }
    if (Reset_buttonState == LOW && buttonStatePrevious == HIGH) {
      buttonStatePrevious = LOW;
      buttonStateLongPress = false;
   if (!buttonStateLongPress && buttonPressDuration > 2000 && buttonPressDuration < minButtonLongPressDuration) 
     {
         buttonPushCounter = 0;
         tempPassword = buttonPushCounter;
         long maxpower=0;
       EEPROMWritelong(maxpower, tempPassword);
       delay(50);
       seememory();
         wait = 0;
      }
    }    
        previousButtonMillis = currentMillis;
  }
}  


void lcdprint(){
    lcd.setCursor(0,0);
    lcd.print("MAXP:");
    lcd.setCursor(5,0);
    lcd.print(val);
    lcd.println("W ");
    
    lcd.setCursor(11,0);
    lcd.print("P:");
    lcd.setCursor(13,0);
    lcd.print(Totalpower);
    lcd.println("W");
        
    lcd.setCursor(0,1);
    lcd.print("BAT:");
    lcd.setCursor(4,1);
    lcd.print(voltage1); 
    lcd.println("V ");
    
    lcd.setCursor(11,1);
    lcd.print("ACv:");
    lcd.setCursor(15,1);
    lcd.print(Totalacvoltage);
    lcd.println("V ");
    
    lcd.setCursor(0,2);
    lcd.print("Temp:");
    lcd.setCursor(5,2);
    lcd.print(heat); 
    lcd.println("C   ");

    lcd.setCursor(11,2);
    lcd.print("I:");
    lcd.setCursor(13,2);
    lcd.print(Totalcurrent);
    lcd.println("A   ");

    lcd.setCursor(0,3);
    lcd.print("SET MAX P:");
    lcd.setCursor(10,3);
    lcd.print(buttonPushCounter);
}

void lcdprint16(){
  switch (page_counter) {
     case 1:{ 
    lcd.setCursor(0,0);
    lcd.print("BAT:");
    lcd.setCursor(4,0);
    lcd.print(voltage1);
        
    lcd.setCursor(10,0);
    lcd.print("P:");
    lcd.setCursor(12,0);
    lcd.print(Totalpower);
    lcd.println("W");
        
    lcd.setCursor(0,1);
    lcd.print("SET MAX P:");
    lcd.setCursor(10,1);
    lcd.print(buttonPushCounter); 
    
    }
 break;
  case 2: {
    lcd.setCursor(0,0);
    lcd.print("ACV:");
    lcd.setCursor(4,0);
    lcd.print(Totalacvoltage);
    lcd.println("V");
    
    lcd.setCursor(8,0);
    lcd.print("Temp:");
    lcd.setCursor(13,0);
    lcd.print(heat); 
    lcd.println("C");

    lcd.setCursor(0,1);
    lcd.print("SET MAX P:");
    lcd.setCursor(10,1);
    lcd.print(buttonPushCounter); 
  }
  break;
  case 3:{
    lcd.setCursor(0,0);
    lcd.print("MP:");
    lcd.setCursor(3,0);
    lcd.print(val);

    lcd.setCursor(10,0);
    lcd.print("I:");
    lcd.setCursor(12,0);;
    lcd.print(Totalcurrent);
    lcd.println("A ");

    lcd.setCursor(0,1);
    lcd.print("SET MAX P:");
    lcd.setCursor(10,1);
    lcd.print(buttonPushCounter); 
  }
  }
   
   if (currentMillis - previousMillis > interval) {  //If interval is reached, scroll page
     previousMillis = currentMillis;                   //replace previous millis with current millis as new start point
     lcd.clear();                                      //lcd clear if page is changed.
     if (page_counter <3){                             //Page counter never higher than 3 (total of pages)
     page_counter = page_counter +1;                   //Go to next page
     }
     else{
      page_counter=1;                                  //if counter higher than 3 (last page) return to page 1
     }
     } 
     
}

void loop(){
   lcdprint16();
   readmemory();
   startfan();
   checkUp();
   checkDown();
   checkReset();
   fault();
   shutdown1();
   if(millis() > time_now + period){
       time_now = millis(); 
        power();
        dcvoltage();
        temperature();
        accurrent();
        acvoltage();
   }
   if( bPress){
       bPress = false;
   }
   if( uPress){
       uPress = false;
   }
   currentMillis = millis();    // store the current time
}
