#include <Adafruit_MPU6050.h> 
#include <Adafruit_Sensor.h> 
#include <Wire.h> 
Adafruit_MPU6050 mpu; 
#include <SoftwareSerial.h> 
SoftwareSerial mySerial(10, 12); 
#include <LCD_I2C.h> 
#include <Keypad.h> 
#define NUM_DEVICES 5 
APPENDIX 
#define MAX_DURATION 60 // Maximum duration in minutes 
float x,y; 
// Initialize LCD 
LCD_I2C lcd(0x27, 16, 2); 
// Define keypad layout 
const byte ROWS = 4; // Four rows 
const byte COLS = 3; // Three columns 
char keys[ROWS][COLS] = { 
{'1', '2', '3'}, 
{'4', '5', '6'}, 
{'7', '8', '9'}, 
{'*', '0', '#'} 
}; 
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); 
 
// Device control variables 
int devicePins[NUM_DEVICES] = {14, 15, 16, 17, 11}; 
unsigned long endTimes[NUM_DEVICES] = {0}; 
bool deviceStatus[NUM_DEVICES] = {false}; 
 
void setup() { 
    Serial.begin(115200); 
    mySerial.begin(9600); 
    lcd.begin(); 
    lcd.backlight(); 
    lcd.setCursor(0, 0); 
    lcd.print("Device Control"); 
     
    for (int i = 0; i < NUM_DEVICES; i++) { 
        pinMode(devicePins[i], OUTPUT); 
        digitalWrite(devicePins[i], HIGH); 
    } 
    pinMode(11, OUTPUT); 
 
     if (!mpu.begin()) { 
    Serial.println("Failed to find MPU6050 chip"); 
//    while (1)  
//    { 
//      delay(10); 
//    } 
  } 
  Serial.println("MPU6050 Found!"); 
 
  //setupt motion detection 
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
   mpu.setMotionDetectionThreshold(1); 
  mpu.setMotionDetectionDuration(20); 
  mpu.setInterruptPinLatch(true);  // Keep it latched.  Will turn off when reinitialized. 
  mpu.setInterruptPinPolarity(true); 
  mpu.setMotionInterrupt(true); 
 
  Serial.println(""); 
  delay(100); 
} 
void loop() { 
 
//   if(mpu.getMotionInterruptStatus()) { 
    sensors_event_t a, g, temp; 
    mpu.getEvent(&a, &g, &temp); 
    x=a.acceleration.x; 
    y=a.acceleration.y; 
 
    if(x>3 || x <-3 || y>3 || y < -3) 
  { 
    Serial.println("Theft detected"); 
    delay(1000); 
    lcd.clear(); 
        lcd.setCursor(0, 0); 
    lcd.print(" THEFT DETECTED"); 
    SendMessage(); 
    lcd.clear(); 
   } 
   else 
   { 
    lcd.setCursor(0, 0); 
    lcd.print("    PRESS *    "); 
    lcd.setCursor(0, 1);
    lcd.print("   to START    "); 
   } 
    unsigned long currentMillis = millis(); 
    char key = keypad.getKey(); 
     
    if (key == '*') { 
        selectDevice(); 
    } 
 
    for (int i = 0; i < NUM_DEVICES; i++) { 
        if (deviceStatus[i] && currentMillis >= endTimes[i]) { 
            digitalWrite(devicePins[i], HIGH); 
            deviceStatus[i] = false; 
            lcd.clear(); 
            lcd.setCursor(0, 0); 
            lcd.print("Device "); 
            lcd.print(i + 1); 
            lcd.print(" OFF"); 
            delay(1000); 
            lcd.clear(); 
        } 
    } 
} 
 
void selectDevice() { 
    lcd.clear(); 
    lcd.print("Select device:"); 
    char key = waitForKey(); 
    int deviceNum = key - '1'; 
     
    if (deviceNum >= 0 && deviceNum < NUM_DEVICES) { 
        lcd.clear(); 
 lcd.print("Set time (min):"); 
        int duration = getDuration(); 
         
        if (duration > 0 && duration <= MAX_DURATION) { 
            digitalWrite(devicePins[deviceNum], LOW); 
            deviceStatus[deviceNum] = true; 
            endTimes[deviceNum] = millis() + (duration * 60000UL); 
             
            lcd.clear(); 
            lcd.setCursor(0, 0); 
            lcd.print("Device "); 
            lcd.print(deviceNum + 1); 
            lcd.print(" ON"); 
            lcd.setCursor(0, 1); 
            lcd.print("Time: "); 
            lcd.print(duration); 
            lcd.print(" min"); 
            delay(1000); 
        } 
    } 
} 
 
char waitForKey() { 
    char key; 
    do { 
        key = keypad.getKey(); 
    } while (!key); 
    return key; 
} 
 
int getDuration() { 
    int duration = 0;
    char key; 
    lcd.setCursor(0, 1); 
    while (true) { 
        key = keypad.getKey(); 
        if (key >= '0' && key <= '9') { 
            duration = duration * 10 + (key - '0'); 
            lcd.print(key); 
        } else if (key == '#') { 
            return duration; 
        } 
    } 
} 
 
 void SendMessage() 
{ 
  mySerial.println("AT+CSMP=17,167,0,0"); 
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode 
  delay(1000);  // Delay of 1000 milli seconds or 1 second 
  mySerial.println("AT+CMGS=\"+918943751360\"\r"); // Replace x with mobile number 
  delay(1000); 
  mySerial.println("THEFT DETECTED....");// The SMS text you want to send 
  delay(100); 
   mySerial.println((char)26);// ASCII code of CTRL+Z 
  delay(1000); 
}