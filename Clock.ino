//Digital clock built with Arduino Nano, DS3231 RTC, and 16x2 LCD
//Displays the Day of the week, Date and Time 

#include <Wire.h>
#include <RtcDS3231.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> 
#include <EEPROM.h>

hd44780_I2Cexp lcd;
RtcDS3231<TwoWire> rtc(Wire);

char *ptr;

void setup(){
  initialize();  
  //setCompiledTime();
}

void initialize()
{
  Wire.begin();
  lcd.begin(16,2);
  lcd.backlight();
  rtc.Begin();
  Serial.begin(9600);
  initializeDst();
}

void loop() {  
  RtcDateTime now = rtc.GetDateTime(); 
  checkForDst(now);
  
  String day  = getDayOfWeek(now);
  lcd.setCursor(0,0);
  lcd.print(day);
  
  displayDate(now);
  displayTime(now);
}

void initializeDst() {
  int dst;
  EEPROM.get(0, dst);

  if(dst != 0 && dst != 1)  
  {     
    dst = 1;  
    EEPROM.put(0, dst);
  }
}

void setCompiledTime(){
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  const char* compileTimeString = __TIME__;

  int hour, minute, second;

  // Parse the time string to extract hour, minute, and second
  sscanf(compileTimeString, "%d:%d:%d", &hour, &minute, &second);

  // Set clock a few seconds ahead 
  second += 40;

  // Adjust the time if seconds exceed 59
  if (second >= 60) {
    second -= 60;
    minute += 1;
    if (minute >= 60) {
      minute -= 60;
      hour += 1;
      if (hour >= 24) {
        hour -= 24;
      }
    }
  }

  // Convert the updated time back to a formatted string
  char updatedTimeString[9]; // HH:MM:SS + null terminator
  sprintf(updatedTimeString, "%02d:%02d:%02d", hour, minute, second);
  
  // Convert string date to RtcDateTime
  RtcDateTime newTime = RtcDateTime(__DATE__,updatedTimeString);

  rtc.SetDateTime(newTime);
}

void displayDate(RtcDateTime now){
  char dateString[20];
  char year[3];  

  // Get last two digits of year
  snprintf(year, sizeof(year), "%02u", now.Year() % 100);

  snprintf_P(dateString, 
          sizeof(dateString),
          PSTR("%02u/%02u/%s"),
          now.Month(),
          now.Day(),
          year);

  lcd.setCursor(0,1);
  lcd.print(dateString);
}

// Returns day of the week. Rtc gives 0-6, 0 = Sunday
String getDayOfWeek(RtcDateTime now){
  int dayOfTheWeek = now.DayOfWeek();
  
  switch(dayOfTheWeek){
    case 0:
      return "Sunday";
    case 1:
      return "Monday";
    case 2:
      return "Tuesday";
    case 3:
      return "Wednesday";
    case 4:
      return "Thursday";
    case 5:
      return "Friday";
    case 6: 
      return "Saturday";
    default:
      return "Invalid day of week";
  }
} 

void displayTime(RtcDateTime now){ 
  unsigned int militaryHour = now.Hour();
  unsigned int hour = getHour(militaryHour);
  unsigned int minute = now.Minute();
  unsigned seconds = now.Second();

  // Turn off backlight between the hours of 8pm and 7am.
  if(militaryHour <= 7 || militaryHour >= 20){
    lcd.noBacklight();
  }
  else{
    lcd.backlight();
  }

  // Clears the screen when going from a double digit hour to a single digit hour or vice versa
  if((hour == 1 || hour == 10) && minute == 0 && seconds == 0){
   lcd.clear();
  }

  // Determines where to set the cursor
  if(hour < 10){
    lcd.setCursor(12,1);
  }
  else{
    lcd.setCursor(11,1);
  }

  // Print time
  lcd.print(hour);
  lcd.print(':');
  if(minute == 0){
    lcd.print("00"); 
  }else if (minute < 10){
    lcd.print("0");
    lcd.print(minute);
  }else{
    lcd.print(minute);
  }
}

// Converts military time to standard format
int getHour(unsigned hour)
{
  if(hour > 12) return(hour-12);
  else if(hour == 0) return 12;
  else return hour;
}

void checkForDst(RtcDateTime now){
  uint8_t hour = getHour(now.Hour());
  uint8_t minute = now.Minute();
  uint8_t seconds = now.Second();
  uint8_t month = now.Month();
  uint8_t day = now.Day();
  uint8_t year = now.Year();
  uint8_t dayOfWeek = now.DayOfWeek();

  int dst;
  
  if (dayOfWeek == 0 && month == 3 && day >= 8 && day <= 16 && hour == 2 && minute == 0 && seconds == 0 && dst == 0)     
  {
    now += 60;
    rtc.SetDateTime(now); 
    dst = 1;       
    EEPROM.put(0, dst);     
  }     
  else if(dayOfWeek == 0 && month == 11 && day >= 1 && day <= 8 && hour == 2 && minute == 0 && seconds == 0 && dst == 1)     
  {   
    now -= 60;
    rtc.SetDateTime(now);
    dst = 0;       
    EEPROM.put(0, dst);     
  }
}
