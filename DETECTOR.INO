#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
LiquidCrystal_I2C lcd(0x27,16,2); //0x20 ; ox27 ; 0x3F
/*Developed by N.E.C.O 17/10/2019
 *website https://neco-desarrollo.es
 */
/*********Menu reference************
 * 1 - Balance 0 - 500uS
 * 2 - Frequency 20 - 120Hz 
 * 3 - Sensetivity 1 - 10 
 * *********************************
 */

#define EEPROM_SIZE 64
#define pulsePine 9
#define pulsePine2 6
#define sPine 10
#define analogPin A0
#define batLevPin A1
#define touch_pin_mas 8
#define touch_pin_menos 7
#define touch_pin_menu 4
//**************Fast ADC*********************
//******************************************

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
//***************Menu*****************************
//************************************************
const int FREQ = 1;
const int FILTER = 2;
const int DUTY = 3;
const int FDIV = 4;
const int VERSION = 5;

//**************General*********************
//******************************************
char dataG;
bool is_done = false;
int main_pulse_state = 0;
int pulse_2_state = 0;
long delay_pulse_2 = 1;
int max_analog_read = 0;
bool read_done = false;
unsigned long startTimer;  
unsigned long currentTime;
long period = 100000;
int last_lengh_count = 0;  
unsigned long startTimeLcdClear = 0;
unsigned long startTimeLcdShow = 0;
int lcdClearDelay = 300;
int lcdShowDelay = 300;
//Menu button
unsigned long startTimeB;  
unsigned long currentTimeB;
const long menuButtonLongP = 2000000;
bool is_long_click = false; 
//**************Translatable****************
//******************************************
const String freq_t = "Frequency";
const String duty_t = "Pulse";
const String sens_t = "Sens";
const String sketch_version_t = "version";
const String sketch_version_v = "1.0.2";
const String balancing = "Balancing..."; 
const String filter_div = "Filter divider"; 
//**************Touch pins******************
//******************************************

bool touched_mas = false;
bool touched_menos = false;
bool touched_menu= false;
int bPinMasState = 0;
int bPinMenosState = 0;
int bPinMenuState = 0;
int menu_state_counter = 0;
int menu_close_counter = 0;
const int close_menu_offset = 10;
const int menu_items = 6;
//**************IMPORTANT*******************
//******************************************
//const int delayTime = 112;
const int delayTime = 12;
int bip_count = 0;
int aRead = 0;
int cycle_count = 0;
int lengh_count = 0;
int filter = 3;
int max_filter  = 10;
const int charged_voltage = 12.10;
const int no_charged_voltage = 9.9;
int analog_read_count = 10;
bool is_balanced = false;
int balance_pulse_count = 0;
const int max_balance_pulse_count = 3;
int time_counter_main = 0;
const int max_duty = 70;
int fdiv = 0;
const int fdiv_def = 5;

//*****************************************
//*****************************************
//EEPROM variables
int addr_duty = 0;
int addr_freq = 1;
int addr_filter = 2;
int addr_fdiv = 3;
int stored_value;
int duty_cycle;
int duty_cycle_temp;
int freq;
int freq_temp;
int duty_def_value = 18;
int freq_def_value = 60;
//Measuring of level of the battery
int volt_in_p = 0;
float r1 = 61000; 
float r2 = 20000; 
float const arefVolt = 3.6f; 
float voutv;
float vinv;
unsigned long startMillisVolts;  
unsigned long currentMillisVolts;
long periodVolts = 1000;  
int sensorValue = 0.0f;
byte level[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};


 void setup() {
//LCD display
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.createChar(0,level);
  lcd.setCursor(2,0);
  //lcd.write(0);
  lcd.setCursor(2,0);
  lcd.print("SpiritPi 4 A");
  lcd.setCursor(3,1);
  lcd.print("by N.E.C.O");
 Serial.begin(115200);
//Menu buttons**************
// set prescale to 16
sbi(ADCSRA,ADPS2) ;
cbi(ADCSRA,ADPS1) ;
cbi(ADCSRA,ADPS0) ;
 pinMode(touch_pin_mas, INPUT);
 pinMode(touch_pin_menos, INPUT);
 pinMode(touch_pin_menu, INPUT);
//**************************
//Pulse pins****************
  pinMode(pulsePine, OUTPUT);
  pinMode(pulsePine2, OUTPUT);
  pinMode(sPine, OUTPUT);
//************************** 


 readFromStorage(addr_duty);
 duty_cycle = stored_value;
 readFromStorage(addr_freq);
 freq = stored_value;
 readFromStorage(addr_filter);
 filter = stored_value;
  readFromStorage(addr_fdiv);
 fdiv = stored_value;
 if(duty_cycle == 255 || duty_cycle == 0){
  writeToStorage(duty_def_value,addr_duty);
  readFromStorage(addr_duty);
  duty_cycle = stored_value;
  }
 
  if(freq == 255 || freq == 0){
  writeToStorage(freq_def_value,addr_freq);
  readFromStorage(addr_freq);
  freq = stored_value;
  }
  if(filter == 255 || filter == 0){
  writeToStorage(filter,addr_filter);
  readFromStorage(addr_filter);
  filter = stored_value;
  }
  if(fdiv == 255 || fdiv == 0){
  writeToStorage(fdiv_def,addr_fdiv);
  readFromStorage(addr_fdiv);
  fdiv = stored_value;
  }
  max_filter = freq / fdiv;
  playSetUpSound();
  
 }


void loop() { 
 if(Serial.available()>0)
 {
 dataG = Serial.read();
 setDutyAndFreq(dataG);
 }
//Serial.println(duty_cycle);
//delay(500);
 pulse_main(pulsePine,duty_cycle,freq);
 // lcd.display();

 //readVoltios();

 
}      

//********************Read/Write memopry*************************
//***************************************************************
void writeToStorage(int valor,int addr)
 {
   EEPROM.write(addr, valor);
  
     
}
 int readFromStorage(int addr)
 {
    stored_value = EEPROM.read(addr);

 return stored_value;
 
 }
//********************Set Duty and Freq**************************
//***************************************************************
void setDutyAndFreq  (char valor)
 {
   //"n" valor para aumentar duty cycle
   //"m" valor para disminuir duty cycle
   //"j" valor para aumentar la frequencia
   //"k" valor para des,inuir la frequencia
   //"+" valor para aumentar el balance
   //"-" valor para desminuir el balance
   if(valor == 'n')
   {
    // Serial.println("n Recived");
  readFromStorage(addr_duty);
  duty_cycle = stored_value;
  duty_cycle = duty_cycle + 1;
  writeToStorage(duty_cycle,addr_duty);
  
   }
   else if(valor == 'm')
     {
      //Serial.println("m Recived");
  readFromStorage(addr_duty);
  duty_cycle = stored_value;
  duty_cycle = duty_cycle - 1;
  writeToStorage(duty_cycle,addr_duty);
  
     }
     else if(valor == 'j')
     {
    //    Serial.println("j Recived");
          readFromStorage(addr_freq);
  freq = stored_value;
  freq = freq + 10;
  writeToStorage(freq,addr_freq);
  max_filter = freq / 10;
  
     }
     else if(valor == 'k')
     {
     //   Serial.println("k Recived");
                  readFromStorage(addr_freq);
  freq = stored_value;
  freq = freq - 10;
  writeToStorage(freq,addr_freq);
  max_filter = freq / 10;

     }
      else if(valor == 'p')
     {
      //  Serial.println("m Recived");
         
  writeToStorage(255,addr_freq);
  writeToStorage(255,addr_duty);
  
     }
}
//*********************Send data via Bluetooth*******************
//***************************************************************
 /*void sendData()

  {
      String dataG = "<";
      dataG +=sensorValue;
      dataG +="/";
      dataG +=freq;
      dataG +="/";
      dataG +=duty_cycle;
      dataG +="/";
      dataG +=volt_in_p;
      dataG +=">";
      SerialBT.println(dataG);
 
    }*/

//*********************Battery level read************************
//***************************************************************
  //Volt function
void readVoltios(){
   // Lectura voltios
 if (currentMillisVolts - startMillisVolts >= periodVolts)
 {
      vinv = 0.0f;
      voutv = 0.0f;
  
      voutv = readVoltage(batLevPin);
      vinv = ( (r1 + r2)* voutv) / r2 ;  
  if(vinv < 0.9)
  {
      vinv=0.0f;
  }
 
      volt_in_p = 100 - ((charged_voltage - vinv) * 100) / (charged_voltage - no_charged_voltage); 
 if(volt_in_p > 100)volt_in_p = 100;
 if(volt_in_p < 0)volt_in_p = 0;
      startMillisVolts = currentMillisVolts;
  }
}
 double readVoltage(byte pin)
  {
   /*    double reading = analogRead(pin);
    if(reading < 1 || reading >= 4095) return 0;
       return -0.000000000000016 * pow(reading, 4 ) + 0.000000000118171 * pow(reading, 3 ) - 0.000000301211691 * pow (reading,2) + 0.001109019271794 * reading + 0.034143524634089;*/
  }
//*********************Touch butttons read***********************
//***************************************************************
 void touchRead()
 {
  
   setMenos_1();
   setMas_1();
   touchMenu();
 
  }
  void touchMenu()
{
        
       bPinMenuState = digitalRead(touch_pin_menu);
          if(bPinMenuState == HIGH && !touched_menu)
    {
      startTimeB = micros();
    
   
    
       touched_menu = true;  
    }
    else if( bPinMenuState == LOW && touched_menu )
    {
      if(!is_long_click){
          if(menu_state_counter < menu_items - 1){
      
      menu_state_counter++;
      menu_close_counter = 0;
      mMenu(menu_state_counter);
  
     } 
     else
     {
      menu_state_counter = 0;
      mMenu(menu_state_counter);
    
      }
      
      }
      else
      {
        is_long_click = false;
         
        }
     
       touched_menu = false;
       
    }
    else if( bPinMenuState == HIGH && touched_menu && !is_long_click)
    {
       currentTimeB = micros()- startTimeB;
       
        if(currentTimeB  > menuButtonLongP)
      {
        //Long click
        is_long_click = true;
        startTimeB = 0;
        currentTimeB = 0;
       startBalanceSound();
       startBalance();
        
      }  
     
    }
}
void setMasMenuSelect(int itemMenu)
{
   switch(itemMenu)
  {
    case 0:
     if(delay_pulse_2 < 500)delay_pulse_2++;
    break;
    case FREQ:
    readFromStorage(addr_freq);
    freq = stored_value;
    if(freq < 200 )
    {
      
    freq = freq + 10;
    lcdShowFreqMenu(freq);
    writeToStorage(freq,addr_freq);
      max_filter = freq / fdiv;
      }
  
    break;
    case FILTER:
    if(filter < max_filter){
      
       filter++;
       writeToStorage(filter,addr_filter);
    }
 
    lcdShowSensMenu(filter);
    break;
     case DUTY:
    readFromStorage(addr_duty);
    duty_cycle = stored_value;
    if(duty_cycle < max_duty)
    {
    duty_cycle = duty_cycle + 1;
    writeToStorage(duty_cycle,addr_duty);
      
      }
    lcdShowDutyMenu(duty_cycle);
    break;
      case FDIV:
    readFromStorage(addr_fdiv);
    fdiv = stored_value;
    if(fdiv < 10)
    {
    fdiv = fdiv + 1;
    writeToStorage(fdiv,addr_fdiv);
      }
    lcdShowFDivider(fdiv);
     
    break;
    }
  }
  void setMenosMenuSelect(int itemMenu)
{
   switch(itemMenu)
  {
    case 0:
     if(delay_pulse_2 > 0)delay_pulse_2--;
    break;
    case FREQ:
    readFromStorage(addr_freq);
    freq = stored_value;
    if(freq > 20 )
    {
    freq = freq - 10;
    lcdShowFreqMenu(freq);
    writeToStorage(freq,addr_freq);
    max_filter = freq / fdiv;
    break;
    case FILTER:
    if(filter > 0){
      filter--;
       writeToStorage(filter,addr_filter);
    }
    
    lcdShowSensMenu(filter);
    break;
    case DUTY:
    readFromStorage(addr_duty);
    duty_cycle = stored_value;
    if(duty_cycle > 16)
    {
    duty_cycle = duty_cycle - 1;
    writeToStorage(duty_cycle,addr_duty);
      }
    lcdShowDutyMenu(duty_cycle);
     
    break;
     case FDIV:
    readFromStorage(addr_fdiv);
    fdiv = stored_value;
    if(fdiv > 2)
    {
    fdiv = fdiv - 1;
    writeToStorage(fdiv,addr_fdiv);
      }
    lcdShowFDivider(fdiv);
     
    break;
    }
  }
  }
 void mMenu(int itemMenu)
 {
  switch(itemMenu)
  {
    case 0:
    lcdShowMainMenu(true);
    break;
    case FREQ:
    lcdShowFreqMenu(freq);
    break;
    case FILTER:
    lcdShowSensMenu(filter);
    break;
    case DUTY:
    lcdShowDutyMenu(duty_cycle);
    break;
     case FDIV:
     lcdShowFDivider(fdiv);
    break;
      case VERSION:
     lcdShowVersion();
    break;
    }
  }
  void closeMenu()
  {
    mMenu(0);
    menu_close_counter = 0;
    menu_state_counter = 0;
    }
 void setMas_1()
    {
     
       
       bPinMasState = digitalRead(touch_pin_mas);
          if(bPinMasState == HIGH && !touched_mas)
    {
  
         setMasMenuSelect(menu_state_counter);
         touched_mas = true;
         menu_close_counter = 0;
         
    }
    else if( bPinMasState == LOW && touched_mas)
    {
       touched_mas = false;
    }
}
 void setMenos_1()
    {
      
     
       
       bPinMenosState = digitalRead(touch_pin_menos);
          if(bPinMenosState == HIGH && !touched_menos)
    {
  
         setMenosMenuSelect(menu_state_counter);
         touched_menos = true;
         menu_close_counter = 0;
         
    }
    else if( bPinMenosState == LOW && touched_menos)
    {
       touched_menos = false;
    }
        
       
  
}
//**********Read Data From Coil**********************************
//***************************************************************
void balance()
{
  if(!is_balanced)
  {
    balance_pulse_count++;
if(balance_pulse_count == max_balance_pulse_count)
{
  is_balanced = true;
  balance_pulse_count = 0;
  finishBalanceSound();
  
}
    }
  }
  void startBalance()
  {
    delay_pulse_2 = 1;
    is_balanced = false;
    lcdShowBalancing();

   }  
  void timeCounter()
  {
      if(time_counter_main > freq / 2){
              time_counter_main = 0;
            }
            else
            {
              time_counter_main++;
              }
    }

       
                
void readDataFromCoil()
          {
          
           int aReadMax = 0;
          for (int i = 0; i < 20; i++)
         {
     aRead = analogRead(analogPin);
     if(aRead > aReadMax)aReadMax = aRead;
   }
   aRead = aReadMax;
     if(menu_state_counter > 0)
     {
     menu_close_counter++;
     if(menu_close_counter >= (freq * close_menu_offset))closeMenu();
     }
    
     // sensorValue = aRead;
       //sendData();
       
     cycle_count++;
   if(aRead > 0){
    bip_count++;
  }

 
  
  if(cycle_count > max_filter){
    if(bip_count > filter) {
     if(is_balanced){
      tone(sPine, 170 + (10 * lengh_count) );
      delay(30);
      noTone(sPine);
     // sensorValue = lengh_count;
     if(menu_state_counter == 0)lcdShowSignalLevel(aRead);
      
     
     
     }
     else
     {
      delay_pulse_2++;
      }
    
        }
        else
        {
          balance();
          
          if(menu_state_counter == 0) showLevelZero();
          }
    //  sendData();
    
      bip_count = 0;
      cycle_count = 0;
  

       }
            }

//*******************Sound***************************************
//***************************************************************
  void playSetUpSound()
         {
//Start melody #1
/*    ledcWriteTone(channel, 329);
      delay(200);
      ledcWriteTone(channel, 246);
      delay(200);
      ledcWriteTone(channel, 146);
      delay(200);
      ledcWriteTone(channel, 196);
      delay(300);
      ledcWriteTone(channel, 0);
      delay(400);
      ledcWriteTone(channel, 429);
      delay(300);
      // ledcWriteTone(channel, 329);
      ledcWriteTone(channel, 0);
      ledcDetachPin(14); */
//Start melody #2
      delay(150);
      tone(sPine, 148);
      delay(150);
      tone(sPine, 222);
      delay(150);
      tone(sPine, 300);
      delay(150);
      tone(sPine, 373);
      delay(400);
      noTone(sPine);
      delay(400);
      tone(sPine, 773);
      delay(500);
      noTone(sPine);
      lcdShowBalancing();
     // lcd.write(0);
      
}


 void finishBalanceSound()
         {
      
      tone(sPine, 435);
      delay(230);
      noTone(sPine);
      delay(200);
      tone(sPine, 132);
      delay(230);
     noTone(sPine);
      lcdShowMainMenu(true);
     /* delay(200);
      tone(sPine, 222);
      delay(130);
      noTone(sPine);*/
      
}
 void startBalanceSound()
         {
      
      tone(sPine, 535);
      delay(500);
      noTone(sPine);
    
     // lcdShowMainMenu(true);
     /* delay(200);
      tone(sPine, 222);
      delay(130);
      noTone(sPine);*/
      
}



//*************************LCD methods*****************************
//*****************************************************************
void lcdShowFDivider(int divider)
{
      max_filter = freq / fdiv;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print(filter_div);
       lcd.setCursor(7,1);
      lcd.print(divider);
      lcd.display();
  }
void lcdShowBalancing()
{
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print(balancing);
      // lcd.setCursor(0,1);
     // lcd.print("                ");
      lcd.display();
  }
  
void lcdShowTest(int message)
{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(message);
      lcd.display();
      
  }
void lcdShowMainMenu(bool dispaly_show)
{
  if(dispaly_show){
      lcd.clear();
      }
      lcd.setCursor(0,0);
      lcd.print("F-");
      lcd.print(freq);
      lcd.print(";");
      lcd.print("D-");
  int duty = duty_cycle * 10;
      lcd.print(duty);
      lcd.print(";");
      lcd.print("Ft-");
      lcd.print(filter);
      if(dispaly_show){
      lcd.display();
      }
      
  }
void lcdShowFreqMenu(int freq_p)
{
      
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print(freq_t);
      lcd.setCursor(6,1);
      lcd.print(freq_p);
      lcd.print("Hz");
      lcd.display();
      
  }
  void lcdShowSensMenu(int sens_p)
{     
      
      lcd.clear();
      lcd.setCursor(6,0);
      lcd.print(sens_t);
      lcd.setCursor(7,1);
      lcd.print(sens_p);
      lcd.display();
      
  }
    void lcdShowDutyMenu(int duty_p)
{     
     
      lcd.clear();
      lcd.setCursor(6,0);
      lcd.print(duty_t);
      lcd.setCursor(6,1);
      duty_p = duty_p * 10;
      lcd.print(duty_p);
      lcd.print("uS");
      lcd.display();
      
  }
   
   void lcdShowSignalLevel(int data)
{ 
  if((micros() - startTimeLcdShow) > lcdShowDelay){
         if(data > 8)
      {
        lengh_count = 5;
        }   
          else if(data > 6)
      {
        lengh_count = 4;
        }
          else if(data > 4)
      {
        lengh_count = 3;
        }
          else if(data > 2)
      {
        lengh_count = 2;
        }
         else if(data > 0)
      {
        lengh_count = 1;
        }
        else
        {
           lengh_count = 0;
          }
     
         if(data > 8)
         {
          lengh_count = lengh_count + (data / 8); 
         if(lengh_count > 16){
          lengh_count = 16;
         }
        
         }
      if(lengh_count != last_lengh_count )
      {
     
      String print_value;
      for(int i = 0; i < 16;i++)
      {
      
    if(lengh_count > i){
        print_value += "X";
      }
      else
      {
        print_value += " ";
        }
      
      }
      lcd.setCursor(0,1);
      lcd.print(print_value);
      lcd.display();
    
      }
        last_lengh_count = lengh_count;
      startTimeLcdShow = micros();
  
  }
  
     
      
  }
  void showLevelZero()
  {
    if(lengh_count > 0 && ( micros() - startTimeLcdClear ) > lcdClearDelay ){
      lengh_count = 0;
     // lcd.clear();
     // lcdShowMainMenu(false);
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.display(); 
      startTimeLcdClear = micros();
      }
    }
   void lcdShowVersion()
    {
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print(sketch_version_t);
        lcd.setCursor(5,1);
      lcd.print(sketch_version_v);
      lcd.display(); 
      }
//*****************************************************************
//*****************************************************************   
   
//*************************Pulse genarator*************************
//*****************************************************************
  void pulse_main( int pulse_pin, int duty_uS, int freq_main_Hz)
          {
            currentTime = micros();
            if (currentTime - startTimer >= period)
 {
      duty_uS = duty_uS * 10;
      period = 1000000 / freq;
//Start main pulse********************
//************************************
      digitalWrite(pulse_pin,HIGH);
      delayMicroseconds(duty_uS);
      digitalWrite(pulse_pin, LOW);
//Stop main pulse********************
//***********************************
//Start control pulse****************
//***********************************       
      digitalWrite(pulsePine2,HIGH);
      delayMicroseconds(delay_pulse_2);
      digitalWrite(pulsePine2, LOW);
//Stop control pulse*****************
//***********************************
//Read data**************************
//***********************************        
      delayMicroseconds(delayTime);
    readDataFromCoil();
    touchRead();
    startTimer = currentTime;
   
    }
   
            }
//*************************END*************************************
//*****************************************************************          
