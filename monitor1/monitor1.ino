#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// "°" symbol
byte degree[8] = {
  B00110,
  B01001,
  B01001,
  B00110,
  B00000,
  B00000,
  B00000,
  B00000
};

// Buttons setup (on PWM pins 2 and 3)
const int button1 = 2; 
const int button2 = 3;

int mode = 0; //0 = CPU/RAM, 1 => modeMax-1 = Disk, modeMax = Infos 
int modeMax = 0;

// Table for disks
#define MAX_DISKS 10

String diskPath[MAX_DISKS];
float disk_used[MAX_DISKS];
float disk_total[MAX_DISKS];
float disk_percent[MAX_DISKS];
int diskCount = 0;

String ip = "", hostname = "", kernel = "", distro = "";
float temp, cpu, ram_used, ram_total, uptime;

// Global var for cycle
unsigned long lastInfoSwitch = 0;
int infoStep = 0; // 0=IP, 1=hostname, 2=kernel+distro, 3=uptime
const int infoDelay = 2500; // 1 sec per screen

void setup() {
  pinMode(button1, INPUT_PULLUP); // Defines buttons
  pinMode(button2, INPUT_PULLUP);

  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degree);
}

unsigned long lastDataTime = 0; 
const unsigned long dataTimeout = 5000; // consider "disconnected" after 5s of silence
bool waitingShown = false;

// Mode: => right
void changeModeR() {
  mode++;
  if (mode > modeMax) mode = 0;
  lcd.clear();
  displayMode();
  delay(500);
  lcd.clear();
  display();
}

// Mode: <= left
void changeModeL() {
  mode--;
  if (mode < 0) mode = modeMax;
  lcd.clear();
  displayMode();
  delay(500);
  lcd.clear();
  display();
}

int lastButton1State = HIGH;
int lastButton2State = HIGH;

void loop() {
  // Read buttons for each loop
  int button1State = digitalRead(button1);
  int button2State = digitalRead(button2);

  if (button1State == LOW && lastButton1State == HIGH) {
    changeModeL();
    delay(50); // debounce
  }
  else if (button2State == LOW && lastButton2State == HIGH) {
    changeModeR();
    delay(50); // debounce
  }
  // resets da buttons positions
  lastButton1State = button1State;
  lastButton2State = button2State;

  // Data treatment
  if (Serial.available() > 0) {
    String line = Serial.readStringUntil('\n');
    lastDataTime = millis(); // mark that we just heard data

    if (waitingShown) {
      lcd.clear();
      waitingShown = false;
    }

    if (line.startsWith("DATA:")) {
      String data = line.substring(5);
      parseData(data);
    } 
    else if (line.startsWith("CMD:")) {
      String cmd = line.substring(4);
      if (cmd == "M") {
        changeModeR();
      }
      else if (cmd == "L") {
        changeModeL();
      } 
      else if (cmd == "K") {
        mode = 0;
        lcd.clear();
        displayMode();
        delay(500);
        lcd.clear();
      }
    }

    display();    
  }
  else if (!waitingShown && millis() - lastDataTime > dataTimeout) {
    lcd.clear();
    lcd.print("Lancer le script");
    waitingShown = true;
  }
  if (mode == modeMax) {
    display(); // keep INFO's cycle alive even with no new serial data
  }
}

void parseData(String jsonStr) {
  String values[9 + 4*MAX_DISKS]; // 9 header fields + 4 fields x MAX_DISKS 

  int lastIdx = 0;
  int currentIdx = 0;
  int valueCount = 0;

  // splits pipe-delimited DATA string into values[] (substrings)
  while ((currentIdx = jsonStr.indexOf('|', lastIdx)) != -1 && valueCount < 59) {
    values[valueCount] = jsonStr.substring(lastIdx, currentIdx);
    lastIdx = currentIdx + 1;
    valueCount++;
  }
  if (lastIdx < jsonStr.length()) {
    values[valueCount] = jsonStr.substring(lastIdx);
    valueCount++;
  }

  temp       =    values[0].toFloat();
  cpu        =    values[1].toFloat();
  ram_used   =    values[2].toFloat();
  ram_total  =    values[3].toFloat();
  ip         =    values[4];
  hostname   =    values[5];
  kernel     =    values[6];
  distro     =    values[7];
  uptime     =    values[8].toFloat();

  diskCount = 0;
  for (int i = 9; i + 3 < valueCount && diskCount < MAX_DISKS; i += 4) {
    disk_used[diskCount]      = values[i].toFloat();
    disk_total[diskCount]     = values[i+1].toFloat();
    disk_percent[diskCount]   = values[i+2].toFloat();
    diskPath[diskCount]       = values[i+3];
    diskCount++;
  }

  modeMax = diskCount + 1; // +1 for info screen is after disks screens
}

// MODE 0
void CPU_RAM() {
  lcd.setCursor(0, 0);
  lcd.print("C:");
  char cpuStr[6]; // Always +1 minimum between Str[] and dtostrf arg
  dtostrf(cpu, 5, 1, cpuStr); // assumes cpu 0-100% (5 for 5-digits including ".")
  lcd.print(cpuStr);

  lcd.print("% T:");
  char tempStr[4];
  dtostrf(temp, 3, 0, tempStr); // same here 3-digits
  lcd.print(tempStr);
  lcd.write(byte(0));
  lcd.print("C");
  

  lcd.setCursor(0, 1);
  lcd.print("RAM: ");
  char ramUsedStr[6];
  dtostrf(ram_used, 4, 1, ramUsedStr); // 4-digits
  lcd.print(ramUsedStr);
  lcd.print("/");
  char ramTotalStr[4];
  dtostrf(ram_total, 3, 0, ramTotalStr);
  lcd.print(ramTotalStr);
  lcd.print("GiB");
}

// MODE 1 => MODE modeMax - 1
void DISK(int index) {
  if (index >= 0 && index < diskCount) {
    lcd.setCursor(0, 0);
    String path = diskPath[index];
    while (path.length() < 10) path += " "; // right padding
    lcd.print(path.substring(0, 10));
    lcd.print(": ");
    char pctStr[4];
    dtostrf(disk_percent[index], 3, 0, pctStr); // 3-digits
    lcd.print(pctStr);
    lcd.print("%");

    lcd.setCursor(0, 1);
    char usedStr[7];
    // max 9.76 TB drive (6-digits limit => 9999.9GiB)
    dtostrf(disk_used[index], 6, 1, usedStr); // 6-digits here because TB are shown as GB (TB = 10^3 GB). We assume it could reach to this amount of digits
    lcd.print(usedStr);
    lcd.print("/");
    char totalStr[7];
    dtostrf(disk_total[index], 6, 1, totalStr); // same thing here 6-digits
    lcd.print(totalStr);
    lcd.print("GiB");
  }
}

// MODE modeMax
void INFO() {
  // Change step every sec
  if (millis() - lastInfoSwitch > infoDelay) {
    lastInfoSwitch = millis();
    infoStep++;
    if (infoStep > 3) infoStep = 0;
    lcd.clear();
  }

  lcd.setCursor(0, 0);
  lcd.setCursor(0, 1);

  if (infoStep == 0) {
    lcd.setCursor(0, 0);
    lcd.print("IPv4:");
    lcd.setCursor(0, 1);
    lcd.print(ip);
  }
  else if (infoStep == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Hostname:");
    lcd.setCursor(0, 1);
    lcd.print(hostname);
  }
  else if (infoStep == 2) {
    lcd.setCursor(0, 0);
    lcd.print(distro);
    lcd.setCursor(0, 1);
    lcd.print(kernel);
  }
  else if (infoStep == 3) {
    lcd.setCursor(0, 0);
    lcd.print("Uptime:");
    lcd.setCursor(0, 1);
    lcd.print(uptime, 2);
    lcd.print(" h");
  }
}

void display() {
  if (mode == 0) {
    CPU_RAM();
  } 
  else if (mode >= 1 && mode <= diskCount) {
    DISK(mode - 1); // mode 2 => disk 0, mode 3 => disk 1...
  }
  else if (mode == modeMax) {
    INFO();
  }
}

void displayMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (mode == 0) {
    lcd.print("Main");
    lcd.setCursor(0, 1);
    lcd.print("CPU/RAM");
  }
  else if (mode >= 1 && mode <= diskCount) {
    lcd.print("Disque " + String(mode));
    lcd.setCursor(0, 1);
    lcd.print("'"+diskPath[mode-1]+"'"); // Shows disk path
  } 
  else if (mode == modeMax) {
    lcd.print("Divers");
  }
}