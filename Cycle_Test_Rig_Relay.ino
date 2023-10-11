//constant holding the value of the desired pin used in place of the mechanical switch
#define ACTPIN 2
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

int num_tools = 10;
int pins[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

bool cooldownEnabled = true;

//Current Cycles of Current Set
int cycles = 0;
//Total Cylces Over Duration Of Test
int total_cycles = 0;

//The Number Of Cycles Per Set
int set_cycles = 15;

//True if cycles >= set_cycles (test for if the set is complete)
bool isMax = false;

//Used to define the end of overall testing
bool isEnd = false;
bool endSignaled = false;

//Interval and duration in Miliseconds
int interval = 4000;

//The cycle "ON" duration in Miliseconds
int duration = 4500;

//Cooldown duration in Miliseconds
unsigned long cooldown = 120000;

bool cooldownActive = false;
unsigned long cooldown_last = 0;

unsigned long cooldown_UI = 500;
unsigned long cooldown_UI_last = 0;

bool cooldownFlashState = false;


//Use this int to set a firm limit on test cycles. if test_limit_cycles is set to -1 then test will continue as long as board is supplied power (and int overflow does not ouccur)
int test_limit_cycles = -1;

void setup() {
  LCDStart();
  //Set ACTPIN to output mode
  for (int i = 0; i < num_tools; i++) {
    pinMode(pins[i], OUTPUT);
  }


  //Start logging data via USB serial
  Serial.begin(9600);
}

void loop() {
  unsigned long current_time = millis();

  if (cooldownActive) {
    CooldownUpdateTick(current_time);
  } else {

    if (!isEnd) {
      if (cooldownEnabled) {
        //Check if the cycle count has been met
        if (cycles >= set_cycles) {
          isMax = true;
          //reset current set cylces to 0
         
          
          EnterCooldown(current_time);
          

          
          
          
        }
      }

      //Check if current cycles is greater than or equal to max cycles per set (basically pointless because its unreachable too)
      if (!isMax) {
        //Set the actuation pin to HIGH or tool "ON" emulates closed switch
        SetPins(HIGH);
        //Keep tool in HIGH or "ON" for the on duratiuon
        delay(duration);

        //Set the actuation pin to LOW or tool "OFF" emulates an open switch
        SetPins(LOW);

        //Keep tool in LOW or "OFF" for the off duration
        cycles += 1;
        total_cycles += 1;

        LCDUpdate();
        //Serial outputs for
        Serial.println(String(cycles) + "< Cycles This Set");
        Serial.println(String(total_cycles) + " Total Cycles Complete");
        delay(interval);

        //Count up current cycles and total cycles (Note this does not account for the acutal condition of the tool or if a cycle has actually been completed)
      }


      //Check for end of testing
      if (test_limit_cycles != -1 && !isEnd) {
        if (total_cycles >= test_limit_cycles) {
          isEnd = true;
        }
      }


    } else {
      if (!endSignaled) {
        Serial.println("-------------------------");
        Serial.println("Test Complete");
        Serial.println("-------------------------");
        endSignaled = true;
      }
    }
  }
}


void SetPins(bool state) {
  for (int i = 0; i < num_tools; i++) {
    digitalWrite(pins[i], state);
  }
}

void LCDStart() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print(" ");
  lcd.setCursor(4, 1);
  lcd.print("Huskie Tools");
  lcd.setCursor(4, 2);
  lcd.print("SLC Test Rig");
  lcd.setCursor(2, 3);
  lcd.print(" ");
  delay(2000);
  LCDUpdate();
}

void LCDUpdate() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Total Cnt > " + String(total_cycles));

  if (cooldownEnabled) {
    lcd.setCursor(0, 2);
    lcd.print("Set Cnt > " + String(cycles) + "/" + String(set_cycles));
  }
}

void EnterCooldown(unsigned long cur_time) {
  lcd.clear();
  lcd.setCursor(2, 2);
  lcd.print("Cooldown Active");
  cooldownActive = true;
  cooldown_last = cur_time;
}

void EndCooldown() {
  cooldownActive = false;
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Cooldown Finish");
  lcd.setCursor(3, 2);
  lcd.print("Resuming Test");
  cycles = 0;
   
  delay(1500);
  LCDUpdate();

  //Basically pointless since its unreachable during delay
  isMax = false;
}


void CooldownUpdateTick(unsigned long cur_time) {
  if (cur_time - cooldown_last >= cooldown) {
    EndCooldown();


  } else {
    if(cur_time - cooldown_UI_last >= cooldown_UI) {
      lcd.clear();
      if(!cooldownFlashState) {
        lcd.setCursor(2, 0);
        lcd.print("Cooldown Active");
        cooldownFlashState = true;
      } else {
        cooldownFlashState = false;
      }

      lcd.setCursor(2, 2);
      lcd.print("Time Remaining:");

      lcd.setCursor(0,3);
      lcd.print((cooldown - (cur_time - cooldown_last))/1000);
      cooldown_UI_last = cur_time;
    }
  }
}