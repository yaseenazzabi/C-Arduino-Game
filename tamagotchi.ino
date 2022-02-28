/*
 * Begin by importing necessary libraries.
 */
#include <Adafruit_RGBLCDShield.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h> // Not part of AdaFruitRGBLCDShield. Used to load / save the pet.

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

/*
 * Define colours - these will make it easier to set the backlight colour of the display.
 */
#define OFF 0X0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

// Starting variable. Forces choice of save or load when set to false.
bool set = true; 

/*
 * Setup will check if a savegame exists, and set the 'set' variable to reflect what it finds.
 */
void setup() {
  // This code is included in the setup of the HelloWord() example file of Adafruit RGB LCD Shield library.
  Serial.begin(9600);
  lcd.begin(16, 2);
  int time = millis();
  time = millis() - time;
  Serial.print("Took "); Serial.print(time); Serial.println(" ms");
  lcd.setBacklight(WHITE); 
  
  // Determine if a savegame exists.
  bool saveGame = EEPROM.read(6);
  if (saveGame == true) {
    set = false; 
  }
}

/* 
 * These are the default values for all the pet's 'biometrics'. 
 * Saved biometrics will later overwrite these, should they be requested.
 */
bool dead = false;  // When false, a death screen will show up when the pet times out.
int happyCounter = 0; // Counters will help keep track of time.
int fullCounter = 0;
int stage = 0;
int happiness = 2;
int fullness = 3;

/*
 * Functional variables that determine program's output.
 */
String stageString;   // 'Egg', 'Young', 'Adult', 'Mythic'.
String happyString;   // 'Unhappy', 'Content', 'Happy'.
String fullString;    // 'Ravenous', 'Hungry', 'Peckish', 'Full', 'Overfull'.
float ageCounter = 0; // This is what's used to age our pet.
long age = millis();  // millis() will be ~0 before the loop starts. 

uint8_t i=0;

/*
 * The loop will run the program. 
 * The program runs six times per second.
 */
void loop() {
  /*
   * Maximum age reached
   * Age has a maximum value of 9m 59s. 
   * After this point, the pet will die, and a death screen shows up.
   * The age can also be set to 599 in order to end the game.
   */
  while (age >= 599) {
    if (dead == false) {
      dead = death();
    }
    
    lcd.setCursor(0,0);
    lcd.print("Press SELECT to ");
    lcd.setCursor(0,1);
    lcd.print("start a new pet!");   
    
    uint8_t buttons = lcd.readButtons();
    
    // Returns the user to the main screen.
    if (buttons) {
      lcd.clear();
      lcd.setCursor(0,0);
      if (buttons & BUTTON_SELECT) {
        lcd.setBacklight(WHITE);
        lcd.print("Some people give");
        lcd.setCursor(0,1);
        lcd.print("up.. but not us.");
        // Reset variables, for code clarity - they are reset at the Start game portion above.
        dead = false;
        happyCounter = 0;
        fullCounter = 0;
        stage = 0;
        happiness = 2;
        fullness = 3;
        set = false;
        age = 0;
        delay(3000);
        lcd.clear();
      }
    }
  }
  // When quitting or completing a game, a pet's 'dead' value is set to true.
  // This bypasses the death screen, and simply ends the game.
  // When completing the game, its 'set' value is set to false, which allows reloading.
  
  /*
   * Pointer declared in the loop so that they always point to the correct location.
   */
  int *ptrHappy;
  ptrHappy = &happiness;

  /*
   * Start game: When a new game is started, 'set' variable will be false.
   * The set variable can be reset to false within the pause menu.
   * Determines if saved statistics should be used, or if a new pet should be started.
   */
  while(set == false) {
    lcd.setCursor(0,0);
    lcd.print("LOAD GAME: UP");
    lcd.setCursor(0,1);
    lcd.print("NEW GAME : DOWN");
    lcd.setCursor(0, 0);
    
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      lcd.clear();
      lcd.setCursor(0,0);
                 
      if (buttons & BUTTON_UP) {
        lcd.setBacklight(YELLOW);
        lcd.print("You're back!");
        happyCounter = EEPROM.read(0);
        fullCounter = EEPROM.read(1);
        stage = EEPROM.read(2);
        happiness = EEPROM.read(3);
        fullness = EEPROM.read(4);
        age = EEPROM.read(5);
        set = true;
        delay(3000);
        lcd.clear();
        break;
      }
               
      if (buttons & BUTTON_DOWN) {
        lcd.setBacklight(VIOLET);
        lcd.print("Fresh starts...");
        bool dead = false;
        int happyCounter = 0; 
        int fullCounter = 0;
        int stage = 0;
        int happiness = 2;
        int fullness = 3;
        set = true;
        delay(3000);
        lcd.clear();
        break;
      }
    }
  }

  
  /*
   * Aging functions
   * See the functions - they are at the bottom of the file.
   */
  ageCounter = ageCounting(ageCounter);
  age = agingFunction(age, ageCounter); 

  
  /*
   * The Egg stage is simply an introduction screen.
   * It has no user input, as Eggs can't be interacted with.
   */
  while(stage == 0) {
    lcd.setBacklight(YELLOW);
    lcd.setCursor(0, 0);
    lcd.print("A magical");
    lcd.setCursor(0, 1);
    lcd.print("journey awaits.");   
    ageCounter = ageCounting(ageCounter);
    age = agingFunction(age, ageCounter);

    if (age >= 5) {
      delay(3000);
      lcd.clear();
      delay(100);
      age = 1; // x % 0 = 0. This means that if age starts at 0, your pet will immediately grow hungrier and sadder.
      stage = 1;
    }
  }
  
  /*
   * Age decay
   * This is seperate from aging as it isn't necessary until after the Egg stage.
   * These set the values for the variables that the aging functions use.
   * See the bottom of the program for the aging functions themselves.
   */
  if (age % 7 == 0) {
    happyCounter++;
    // the program loops 6 times per second. 
    // This prevents happiness / fullness from being decremented 6 times every n seconds.
    if (happyCounter == 6) {
      if (happiness > 0) {
        happiness--;
      }
      happyCounter = 0;
    }
  }
  
  if (age % 11 == 0) {
    fullCounter++;
    if (fullCounter == 6) {
      if (fullness > 0) {
        fullness--;
      }
      fullCounter = 0;
    }
  }

  /*
   * String operations
   * Depending on the values of the pet's biometrics, different outputs are needed.
   * This part of the program converts numerical values into user-friendly strings.
   */
  if (fullness == 0) {
    happiness == 0;
    fullString = "Ravenous";
  }
  if (fullness == 1) {
    fullString = "Hungry  ";
  }
    if (fullness == 2) {
    fullString = "Peckish ";
  }
    if (fullness == 3) {
    fullString = "Full    ";
  }
    if (fullness == 4) {
    fullString = "Overfull";
  }
 
  if (stage == 0) {
    stageString = "  Egg";
  }
  if (stage == 1) {
    stageString = " Young";
  }
  if (stage == 2) {
    stageString = " Adult";
  }
  if (stage == 3) {
    stageString = "Mythic";
  }
  
  if (happiness == 0) {
    happyString = "Unhappy";
  }
  if (happiness == 1) {
    happyString = "Content";
  }
  if (happiness == 2) {
    happyString = "Happy  ";
  }
  // Takes a lot of memory. Still, a lot better than many confusing if statements within
  // the code (even as pointer functions).
  
  /*
   * Gamestate
   * This part of the program deals with the gameplay itself.
   * Everything visible to the user is beneath this point.
   * The gamestate handles the display and outputs the variables defined above in a user-friendly way.
   */
  int minutes = age / 60; 
  int seconds = (age % 60);
  String minutesString = String(minutes);
  String secondsString = String(seconds);
  if (seconds < 10) {
    secondsString = "0"+secondsString;
  }
  
  lcd.setBacklight(WHITE);
  lcd.setCursor(0,0);
  lcd.print(happyString);
  lcd.print(" ");
  
  lcd.setCursor(10,0); //top-right corner.
  lcd.print(stageString);
  
  lcd.setCursor(0,1); 
  lcd.print(fullString);
  
  lcd.setCursor(12,1); //bottom-right corner.
  lcd.print(minutesString);
  lcd.print("|");
  lcd.print(secondsString);
  
  /*
   * Button functions.
   * UP to feed, DOWN to clear the display, RIGHT to level up and LEFT to play.
   * When the SELECT button is pressed, the pause menu is brought up.
   * In pause: UP to restart, DOWN to delete a save, RIGHT to save the pet and LEFT to continue.
   */
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    
    // Feed the pet.
    if (buttons & BUTTON_UP) {
      lcd.setBacklight(VIOLET);
      lcd.print("Time to eat!");
      delay(50);
      lcd.setCursor(0,1);
      fullness = feedPet(fullness, stage, *ptrHappy); // Changes two variables at once, using a pointer.
    }

    // Clear the display.  
    if (buttons & BUTTON_DOWN) {
      lcd.clear();
    }   
    
    // Play with the pet.     
    if (buttons & BUTTON_LEFT) {
      lcd.setBacklight(TEAL);
      lcd.print("Playtime!");
      delay(50);
      lcd.setCursor(0, 1);  
      happiness = playWithPet(fullness, happiness, stage); // Changes only one variable.
    }
    
    // Level up the pet. Would use a lot of pointer operations, so it is left as it is.
    if (buttons & BUTTON_RIGHT) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.setBacklight(RED);
      
      // If the pet is unavailable to level up:
      if (age < 18 && stage == 1) { 
        lcd.setBacklight(VIOLET);
        lcd.print("She's so young.");
        delay(3000);
        lcd.clear();
      }
      
      // If the pet is young:
      else if (age >= 18 && stage == 1) {
        becomeAdult();       
        stage = 2;
        happiness = 3;
        fullness = 3; 
        lcd.clear();
      }
      
      // If the pet is an adult:
      else if (stage == 2) {
        becomeGod();
        stage = 3;
        fullness = 0;
        happiness = 0;
        age = 0;
        lcd.clear();
      }
      
      // If the pet is a mythic(This ends the game): 
      else {
        endGame(); 
        
        // Reset variables to default values.
        // Although variables are reset elsewhere, this increases code clarity.
        stage = 0;
        age = 0;
        fullness = 3;
        happiness = 2;
        happyCounter = 0;
        fullCounter = 0;
        
        set = false; // Again, 'set' forces the choice of loading or starting a new game.
        lcd.clear();
      }
    }
    
    /*
     * PAUSE menu
     * When the select button is pressed, the pause menu is brought up.
     * UP to restart, DOWN to delete a save, RIGHT to save the pet and LEFT to continue.
     */
    if (buttons & BUTTON_SELECT) {
    // Again, requires many pointer operations to be saved as a function, so left as is.
      lcd.clear();
      bool menu = true; // This determines when to exit the pause menu.
      while (menu == true) {
        // First, scroll the instructions required to use the menu across the display.
        lcd.setBacklight(TEAL);
        lcd.setCursor(0,0);
        lcd.print("LEFT     | RIGHT | UP      | DOWN     | ");
        lcd.setCursor(0,1);
        lcd.print("CONTINUE | SAVE  | NEW PET | DEL SAVE | ");
        lcd.scrollDisplayLeft();
        
        uint8_t buttons = lcd.readButtons();
        if (buttons) {
          lcd.clear();
          lcd.setCursor(0,0);        
          
          // Continue playing, and change nothing:
          if (buttons & BUTTON_LEFT) {
            menu = false;
          }
       
          // Save the current pet to ROM.
          if (buttons & BUTTON_RIGHT) {
            lcd.print("Time to go home!");
            lcd.setCursor(0,1);
            lcd.print("Already?");
            EEPROM.write(0, happyCounter);
            EEPROM.write(1, fullCounter);
            EEPROM.write(2, stage);
            EEPROM.write(3, happiness);
            EEPROM.write(4, fullness);
            EEPROM.write(5, age);
            EEPROM.write(6, true);
            delay(2000);
            lcd.clear();
          }
          
          // Restart the game. Option of reloading or a new pet.
          if (buttons & BUTTON_UP) {
            age = 599;
            set = false;
            dead = true;
            menu = false;
            
          }
          
          // Delete an old saved game.
          if (buttons & BUTTON_DOWN) {
            lcd.print("Slaughtering...");
            lcd.setCursor(0,1);
            lcd.print("You're twisted.");
            EEPROM.write(0, 0);
            EEPROM.write(1, 0);
            EEPROM.write(2, 0);
            EEPROM.write(3, 2);
            EEPROM.write(4, 3);
            EEPROM.write(5, 0);
            EEPROM.write(6, false);
            delay(3000);
          }
          
          if (buttons & BUTTON_SELECT) {
            menu = false;
          }
        }
      }
    }
  }
}

/*
 * Functions:
 */

/*
 * Age functions:
 * These functions keep track of the pet's age.
 */

/*
 * This function is the one that determines when age should be incremented.
 * It runs six times a second in the program loop.
 * Every sixth run, it will return the value 1.0.
 * When the value it returns is 1.0, the age will be incremented,
 */
float ageCounting(float ageCounter) {
  if (ageCounter == 1.0) {
    ageCounter = 0.0;
  }
  ageCounter += 1.0/6.0;
  return ageCounter;
}

/*
 * This function takes the value returned by ageCounting().
 * If this value is 1.0, the age of the pet will be incremented.
 * Every realtime second, the age of the pet will increase by 1.
 */
long agingFunction(long age, float ageCounter) {
  if (ageCounter == 1.0) {
    age += 1;
  }
  return age;
}

/*
 * Gamestate functions
 * These functions allow interaction with the pet.
 */
 
/*
 * Feed the pet. 
 * Uses a pointer to change the values of two variables in one function.
 * Changes both happiness and fullness.
 * Happiness is limited to 0 <= h <= 2, fullness 0 <= f <= 3.
 * For every stage of development, a unique message is displayed when feeding the pet.
 */
int feedPet(int fullness, int stage, int* ptrHappy) {
  ptrHappy = &happiness;
  if (stage == 1) {    
    if(fullness < 3) {
      lcd.print("Thanks so much!");
      fullness++; 
    }
    else if (fullness == 3){
      lcd.print("I'm already full!");
      fullness++;
      *ptrHappy = 0;
    }
    else {
      lcd.print("Please... Stop!");
      *ptrHappy = 0;
    }        
  }

  if (stage == 2) {    
    if(fullness < 3) {
      lcd.print("Excellent!");
      fullness++;    
    }
    else if (fullness == 3){
      lcd.print("Oh... No thanks.");
      fullness++;
      *ptrHappy = 0;
    }
    else {
      lcd.print("I'm so full!");
      *ptrHappy = 0;
    }          
  }

  if (stage == 3) {    
    if(fullness < 3) {
      lcd.print("You may live.");
      fullness++;    
    }
    else if (fullness == 3){
      lcd.print("I am satiated.");
      fullness++;
      *ptrHappy = 0;
    }
    else {
      lcd.print("Stop.");
      *ptrHappy = 0;
    }         
  }
  delay(1000);
  lcd.clear();
  // Because fullness isn't accessed via a pointer, it is returned classically.
  return fullness;
}

/*
 * Play with the pet.
 * No pointer is required as this function only changes one variable.
 * Depending on the fullness of the pet, playing with it may increase the pet's happiness.
 * Happiness is limited to 2 maximum.
 * Like with feeding, each stage of development has unique messages to display when playing with the pet.
 */
int playWithPet(int fullness, int happiness, int stage) {
  if (stage == 1) {
    if (fullness >= 2 && happiness < 2) {
        happiness++;
        lcd.print("Yayyy!");
      }
      else if (fullness >= 2 && happiness == 2) {
        lcd.print("I love you!");
      }
      else { // Don't play with a starving pet!
        lcd.print("I'm hungry!");
      }  
    }
  
  if (stage == 2) {
    if (fullness >= 2 && happiness < 2) {
      happiness++;
      lcd.print("That was fun!");
    }
    else if (fullness >= 2 && happiness == 2) {
      lcd.print("I need to work.");
    }
    else { 
      lcd.print("I have to eat.");
    }
  }
  
  if(stage == 3) {
    if (fullness >= 2 && happiness < 2) {
      happiness++;
      lcd.print("You amuse me.");
    }
    else if (fullness >= 2 && happiness == 2) {
      lcd.print("I grow bored.");
    }
    else { 
      lcd.print("Feed me.");
    } 
  }   
  delay(1000);
  lcd.clear();
  return happiness;
}

/*
 * Flavor text functions
 * These are fun functions that make the game a little more interesting.
 */
 
// Output for when the game's ending is reached (without the pet dying of old age).
void endGame() {
  lcd.setBacklight(RED);
  lcd.print("I raised you.");
  delay(2000);
  
  lcd.noDisplay();
  lcd.clear();
  delay(1000);
  
  lcd.print("Quiet, mortal.");
  lcd.display();
  delay(2000);

  lcd.noDisplay();
  lcd.clear();
  delay(1000);
  
  lcd.setBacklight(RED);
  lcd.print("Enough.");
  lcd.display();
  delay(2000);

  lcd.noDisplay();
  lcd.clear();
  delay(1000);
  
  lcd.setBacklight(RED);
  lcd.print("I'll slaughter");
  lcd.setCursor(0, 1);
  lcd.print("you myself.");
  lcd.display();
  delay(3000);

  lcd.setBacklight(OFF);        
  lcd.noDisplay();
  lcd.clear();
  delay(1500);
  
  lcd.setBacklight(RED);
  lcd.setCursor(0, 0);
  lcd.print("|||||!GAME!|||||");
  lcd.setCursor(0, 1);
  lcd.print("|||||!OVER!|||||");
  lcd.display();
  delay(5000);
}

// Output for when progressing a Young pet into an Adult pet.
void becomeAdult() {
  lcd.setBacklight(RED);
  lcd.print("Wait... What?!?!?");
  delay(2000);
  
  lcd.noDisplay();
  lcd.clear();
  delay(1500);

  lcd.print("I'm growing up!");
  lcd.display();
  delay(2000);

  lcd.setBacklight(OFF);
  lcd.noDisplay();
  lcd.clear();
  delay(1500);
  
  lcd.setBacklight(TEAL);
  lcd.print("Aw. Look at you");
  lcd.setCursor(0,1);
  lcd.print("all grown up!");
  lcd.display();
  delay(3000);
}

// Output for when progressing an Adult pet into a Mythic pet.
void becomeGod() {
  lcd.print("Huh? What?!");
  delay(2000);
  
  lcd.setBacklight(OFF);
  lcd.noDisplay();
  lcd.clear();
  delay(1000);
  
  lcd.setBacklight(RED);
  lcd.print("Finally! Free of");
  lcd.setCursor(0,1);
  lcd.print("this weak form.");
  lcd.display();
  delay(2500);

  lcd.noDisplay();
  lcd.clear();
  delay(1000);
  
  lcd.setBacklight(RED);
  lcd.setCursor(0, 0);
  lcd.print("Oh... my God.");
  lcd.display();
  delay(2000);

  lcd.noDisplay();
  lcd.clear();
  delay(1000);

  lcd.setBacklight(RED); 
  lcd.print("You live to");
  lcd.setCursor(0,1);
  lcd.print("serve me now.");
  lcd.display();
  delay(2500);
  
  lcd.clear();
}

// Output for when a pet dies of old age.
bool death() {
  lcd.clear();
  
  lcd.setBacklight(RED);
  lcd.setCursor(0,0);
  lcd.print("Your pet grows");
  lcd.setCursor(0,1);
  lcd.print("old and weary.");
  delay(2500);
  
  lcd.noDisplay();     
  lcd.clear();
  delay(1000);
  
  lcd.setCursor(0, 0);
  lcd.print("In his dying");
  lcd.setCursor(0, 1);
  lcd.print("breath...");
  lcd.display();
  delay(2500);
  
  lcd.noDisplay();
  lcd.clear();
  delay(1000);
  
  lcd.setCursor(0, 0);
  lcd.print("A whisper...");
  lcd.display();
  delay(2000);
  
  lcd.noDisplay();
  lcd.clear();
  delay(1000);
  
  lcd.setBacklight(RED);
  lcd.print("'I love you.'");
  lcd.display();
  delay(2000);
  
  lcd.noDisplay();
  lcd.clear();
  delay(1000);
  
  lcd.print("As you weep,");
  lcd.setCursor(0, 1);
  lcd.print("the sun sets.");
  lcd.display();
  delay(2500);
  
  lcd.noDisplay();
  lcd.clear();
  delay(1000);
  lcd.display();
  dead = true;
  return dead;
}
