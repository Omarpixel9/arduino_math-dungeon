#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Creating object from LCD library to control LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configuring Keypad using keypad library
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', '-'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', '-'},
  {'*', '0', '#', '-'}
};

// Keypad PIN configuration
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

// Software related variables
bool printedOnce = false; // Used to print LCD only once
bool secondPrintFlag = false;
String keyInput = ""; // To buffer multiple entered keys
int level = 11; // Keep track of what level the player is in
int finalLevel = 10; // Final level. EXAMPLE: if(level=finalLevel+1) -> player wins

// Obstacle flags
bool currentlyInObstacle = false;
bool obstacleClear = false;

// Temperature
int temperaturePin = A0;
int temperatureInput;
double temperature = 0;
double newTemperature;
double initialTemperature;
bool initialTempSet = false;

// Microphone setup
int micPin = 10;
int micInput;

// Potentiometer setup
int potentiometerPin = A1;
int potentioInput;
int potentioOutput;
int potentioTarget; // Target for player to reach
int potentioInitial = -1;
bool targetSet = false;

// Touch setup
int touchPin = 10;
int touchValue;
int pressCounter;
long touchTarget;

// HOLD buttons confiugration
int button1Pin = 13;
int button2Pin = 12;
int button3Pin = 11;
int button1;
int button2;
int button3;

// Obstacle ID
int obstacleID;
bool firstObstacle = true;

// Operation
int operationID;

// Problem variables
int num1;
int num2;
char operation;
String problemText;
int solution;

//In-game timer
bool atStartOfGame = true; // To record program runtime at start of game
bool atHalfOfGame = false;
unsigned long initialTime; // Records time at beginning of game
unsigned long currentTime; // Records current time;
unsigned long timeChange;
unsigned long timeLimit = 180000; // Time limit in milliseconds (ms)

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Setup
void setup() {
  // put your setup code here, to run once:
  pinMode(micPin, INPUT);
  pinMode(touchPin, INPUT);
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  randomSeed(random(2, 3)); // To randomize the numbers every different runtime
  obstacleID = random(1, 5); // Start with a random obstacle
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (level == finalLevel + 1) {
    winGameScreen();
  }
  if (atStartOfGame) {
    initialTime = millis();
    atStartOfGame = false;
  }
  currentTime = millis();
  timeChange = currentTime - initialTime;
  Serial.println(String(timeChange) + "ms");

  if (timeChange >= timeLimit) {
    lcd.clear();
    lcd.print("Time's up!");
    lcd.setCursor(0, 1);
    lcd.print("GAME OVER");
    delay(3000);
    lcd.clear();
    lcd.print("FINAL SCORE");
    lcd.setCursor(0, 1);
    lcd.print("You can do it");
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("Score=" + String(level) + "/" + String(finalLevel));
    delay(10000);
    lcd.clear();
    lcd.print("Reset To");
    lcd.setCursor(0, 1);
    lcd.print("Try Again :)");
    while (true) {
      void(); // To stop the program
    }

  } else if (timeChange >= timeLimit / 2 && !atHalfOfGame) {
    atHalfOfGame = true;
    Serial.println("Half time passed");
  }
  generateRandomProblem();
  while (currentlyInObstacle) {
    if (!printedOnce) {
      lcd.print("Obstacle Alert");
      delay(1000);
      lcd.clear();
      printedOnce = true;
      if (!firstObstacle) {
        if (obstacleID == 4) {
          obstacleID = 1;
        } else {
          obstacleID += 1;
        }

      }
    }
    stuckInObstacle();
  }
  //delay(10);
}

void winGameScreen() { // The screen that shows up if the Player wins

  byte heart[8] = {
    B00000,
    B01010,
    B11111,
    B11111,
    B11111,
    B11111,
    B01110,
    B00100
  };

  char text1[] = "THANK YOU ";
  char text2[] = "4 PLAYING ";

  lcd.clear();
  lcd.print("YOU WON! :D");
  lcd.setCursor(0, 1);
  lcd.print("CONGRATULATIONS!");
  // delay(3000);
  lcd.clear();
  lcd.print("Time taken:");
  //delay(1500);
  lcd.setCursor(0, 1);
  lcd.print(String(timeChange / 1000) + "s/" + String(timeLimit / 1000) + "s");
  //delay(4000);
  lcd.clear();
  lcd.createChar(0, heart);
  lcd.setCursor(0,0);

  while (true) {
    for (int i = 0; i < sizeof(text1); i++) {
      lcd.print(text1[i]);
      delay(250);
    }
    for (int i = 0; i < 5; i++) {
      lcd.write(0);
      delay(250);
    }
    lcd.setCursor(0, 1);
    for (int i = 0; i < sizeof(text2); i++) {
      lcd.print(text2[i]);
      delay(250);
    }
    for (int i = 0; i < 5; i++) {
      lcd.write(0);
      delay(250);
    }
    delay(1500);
    lcd.clear();
  }
}

void launchMainMenu() {
  lcd.clear();
  char text[] = "Press 1 to Play ";
  lcd.setCursor(16, 0);
  lcd.print(" =Math Dungeon=  ");
  lcd.setCursor(16, 1);
  lcd.autoscroll();
  for (int i = 0; i < sizeof(text) - 1; i++) {
    lcd.print(text[i]);
    delay(250);
  }
  lcd.noAutoscroll();
  char key = keypad.getKey();
  // Loop breaks as soon as player presses '1' to play.
  while (key != '1') {
    key = keypad.getKey();
  }

  lcd.clear();

}

void generateRandomProblem() { // Generates a problem randomly
  char key = keypad.getKey();

  if (!printedOnce) {
    lcd.clear();
    lcd.print("LEVEL " + String(level));
    delay(1000);
    lcd.clear();
    printedOnce = true;
    operation = getRandomOperation();
    problemText = String(num1) + operation + String(num2) + " = ?";
    lcd.print(problemText);
  }

  if (key) {
    if (key == '*') {
      lcd.clear();
      lcd.print(problemText);
      keyInput = ""; // Clears the input
      lcd.noCursor();
    } else if (key == '#') {
      solution = getArithmeticProblemSolution(num1, num2, operation);
      if (keyInput.toInt() == solution) { // Checks user input against solution
        lcd.clear();
        lcd.print("Good job!");
        lcd.noCursor();
        // **==Play Buzzer Sound Here==**
        delay(1500);
        lcd.clear();
        level++;
        currentlyInObstacle = true;
        printedOnce = false; // To print obstacle message only once
        keyInput = ""; // Clear keypad input for next question
      }

    } else if (key == '-') {
      lcd.clear();
      lcd.print(problemText);
      keyInput = String(keyInput.toInt() * -1); // Clears the input
      lcd.setCursor(0, 1);
      lcd.print(keyInput);

    } else {
      lcd.setCursor(0, 1);
      lcd.cursor();
      keyInput += key;
      lcd.print(keyInput);
    }

  }


}

char getRandomOperation() {
  operationID = random(1, 5);
  switch (operationID) {
    case 1:
      if (level >= 5) {
        num1 = random(50, 100);
        num2 = random(50, 100);
      } else if (level >= 8) {
        num1 = random(133, 433);
        num2 = random(422, 699);
      } else {
        num1 = random(1, 100);
        num2 = random(1, 100);
      }
      return '+';
    case 2:
      if (level >= 5) {
        num1 = random(50, 100);
        num2 = random(50, 100);
      } else if (level >= 8) {
        num1 = random(244, 699);
        num2 = random(142, 333);
      } else {
        num1 = random(1, 100);
        num2 = random(1, 100);
      }
      return '-';
    case 3:
      if (level >= 5 && level < 8) {
        num1 = random(2, 10);
        num2 = random(50, 100);
      } else if (level >= 8) {
        num1 = random(50, 150);
        num2 = random(50, 250);
      } else if (level < 5 && level >= 3) {
        num1 = random(2, 17);
        num2 = random(2, 25);
      } else {
        num1 = random(2, 10);
        num2 = random(2, 10);
      }
      return '*';
    case 4:
      switch (level) { // Random is bad for division because of decimals. Difficulty linearly scales up.
        case 1:
          num1 = 6;
          num2 = 3;
          break;
        case 2:
          num1 = 15;
          num2 = 3;
          break;
        case 3:
          num1 = 36;
          num2 = 6;
          break;
        case 4:
          num1 = 72;
          num2 = 6;
          break;
        case 5:
          num1 = 304;
          num2 = 8;
          break;
        case 6:
          num1 = 63;
          num2 = 9;
          break;
        case 7:
          num1 = 312;
          num2 = 6;
          break;
        case 8:
          num1 = 288;
          num2 = 6;
          break;
        case 9:
          num1 = 473;
          num2 = 11;
          break;
        case 10:
          num1 = 627;
          num2 = 11;
          break;

      }
      return '/';
  }
}

int getArithmeticProblemSolution(int num1, int num2, char operation) {
  int solution;
  switch (operation) {
    case '+':
      solution = num1 + num2;
      break;
    case '-':
      solution = num1 - num2;
      break;
    case '*':
      solution = num1 * num2;
      break;
    case '/':
      solution = num1 / num2;
      break;
    default:
      break;
  }

  return solution;
}


// 1 for Temperature
// 2 for Touch
// 3 for HoldButtons
// 4 for Potentiometer
void stuckInObstacle() { // Makes player stuck in an in-between obstacle
  switch (obstacleID) {
    case 1:
      temperatureObstacle();
      break;
    case 2:
      touchObstacle();
      break;
    case 3:
      holdObstacle();
      break;
    default:
      potentiometerObstacle();
      break;
  }

}

void temperatureObstacle() { // The code segment that starts receiving the temperature
  temperatureInput = analogRead(temperaturePin);
  newTemperature = temperatureInput;
  /*newTemperature = (double) temperatureInput / 1024;
    newTemperature = newTemperature * 5;
    newTemperature = newTemperature - 0.5;
    newTemperature = newTemperature * 100;*/
  if (!initialTempSet) {
    initialTemperature = newTemperature;
    initialTempSet = true;
  }

  if (newTemperature - temperature > 0) {
    lcd.print("RUB TO 5%");
    lcd.setCursor(0, 1);
    lcd.print("Change=" + String(newTemperature - initialTemperature) + "%");
    lcd.setCursor(0, 0);

    if (newTemperature - initialTemperature > 4) {
      currentlyInObstacle = false; // Reset to
      printedOnce = false; // Reset to print obstacle text
      temperature = 0;
      initialTempSet = false;
      lcd.clear();
      lcd.print("Obstacle Cleared");
      delay(1000);
      lcd.clear();
      firstObstacle = false;
    } else {
      temperature = newTemperature;
    }

  }
}

void potentiometerObstacle() { // Obstacle that checks if player rotated knob all the way
  potentioInput = analogRead(potentiometerPin);
  potentioOutput = map(potentioInput, 0, 1023, 0, 255);


  if (!secondPrintFlag) {
    if (potentioOutput >= 127) {
      potentioTarget = 0;
    } else if (potentioOutput <= 126) {
      potentioTarget = 255;
    } else {
      potentioTarget = potentioOutput / 2;
    }
    lcd.print("ROTATE to " + String((potentioTarget / 255.0) * 100) + "%");
    secondPrintFlag = true;
  }

  if (abs(potentioOutput - potentioTarget) > 0) {
    potentioInitial = potentioOutput;
    lcd.setCursor(0, 1);
    lcd.print(String((potentioOutput / 255.0) * 100) + "%");
    lcd.setCursor(0, 0);
  }

  if (potentioOutput == potentioTarget) {
    currentlyInObstacle = false; // Reset to
    printedOnce = false; // Reset to print obstacle text
    potentioInitial = -1;
    secondPrintFlag = false; // Ready to print second obstacle text
    lcd.clear();
    lcd.print("Obstacle Cleared");
    delay(1000);
    lcd.clear();
    firstObstacle = false;
  }



  delay(2);
}

void touchObstacle() { // Touch X amount of times to clear the obstacle
  if (!secondPrintFlag) {
    if (level < 3) {
      touchTarget = random(5, 20);
    } else if (level >= 3 && level <= 7) {
      touchTarget = random(15, 30);
    } else {
      touchTarget = random(10, 60); // Be lucky or lose.
    }

    lcd.clear();
    lcd.print("TOUCH " + String(touchTarget) + " TIMES");
    secondPrintFlag = true;

  }
  touchValue = digitalRead(touchPin);
  if (touchValue == HIGH) {
    delay(20); // To give enough time to read consecutive touches
    touchValue = digitalRead(touchPin);
    if (touchValue == LOW) {
      pressCounter++;
      lcd.setCursor(0, 1);
      lcd.print(String(touchTarget - pressCounter) + " left");
    }

  }

  if (pressCounter == touchTarget) {
    currentlyInObstacle = false; // Reset to
    printedOnce = false; // Reset to print obstacle text
    secondPrintFlag = false; // Ready to print second obstacle text
    pressCounter = 0;

    lcd.clear();
    lcd.print("Obstacle Cleared");
    firstObstacle = false;
  }

}

void holdObstacle() { // Obstacle that checks for holding 3 buttons for X amount of time
  if (!secondPrintFlag) {
    lcd.print("HOLD 3 BUTTONS");
    secondPrintFlag = true;
  }
  button1 = digitalRead(button1Pin);
  button2 = digitalRead(button2Pin);
  button3 = digitalRead(button3Pin);

  if (button1 == LOW && button2 == LOW && button3 == LOW) {
    lcd.clear();
    lcd.print("KEEP HOLDING");
    delay(random(5000, 10000)); // Hold for a random amount of time between 5s and 10s
    button1 = digitalRead(button1Pin);
    button2 = digitalRead(button2Pin);
    button3 = digitalRead(button3Pin);
    if (button1 == LOW && button2 == LOW && button3 == LOW) { // If still pressed, obstacle cleared
      lcd.clear();
      lcd.print("Obstacle Cleared!");
      delay(1500);
      currentlyInObstacle = false; // Reset to exit obstacle
      printedOnce = false; // Reset to print obstacle text
      secondPrintFlag = false; // Ready to print second obstacle text
      firstObstacle = false;
    } else {
      lcd.clear();
      lcd.print("Try again.");
      delay(1000);
      lcd.clear();
      lcd.print("HOLD 3 BUTTONS");
    }
  }
}

// MICROPHONE DOES NOT WORK
void microphoneObstacle() { // Obstacle that checks for a microphone sound
  if (!secondPrintFlag) {
    lcd.print("CLAP to MIC!!");
    secondPrintFlag = true;
  }
  micInput = digitalRead(micPin); // Reads from microphone
  if (micInput == LOW) { // Checks if sound is above threshold
    lcd.clear();
    lcd.print("CLAP DETECTED!");
    delay(1500);
  }



}

/*
  namespace Calculator{
  enum Operation{ADD, SUB, MUL, DIV, NOP};
  class SimpleCalculator
  {
  public:

    String input = "";
    Operation op = NOP;
    //Num num_state = NUM1;
    int num1, num2;
    bool next;
    void do_op(){
      switch(op){
        case ADD:
          num1 = num1+num2;
          break;
        case SUB:
          num1 = num1-num2;
          break;
        case MUL:
          num1 = num1*num2;
          break;
        case DIV:
          num1 = num1/num2;
          break;
      }
      op = NOP;
      num2 = 0;
      input = String(num1);
    }
    void clean(){
      num1 = num2 = 0;
      input = "";
      op = NOP;
    }
    void store(int which){
      switch(which){
        case 1:
          num1 = input.toInt();
          break;
        case 2:
          num2 = input.toInt();
          break;
      }
    }

  };

  }

  Calculator::SimpleCalculator my_calc;

  void perform_operation(Calculator::Operation Op){
  if(Op == Calculator::NOP){
    my_calc.store(2);
    lcd.clear();
    my_calc.do_op();
    lcd.print(my_calc.input);
    my_calc.next = true;
  }
  if (my_calc.op != Calculator::NOP){
    my_calc.store(2);
    lcd.clear();
    my_calc.do_op();
    lcd.print(my_calc.input);

    my_calc.op = Op;
    my_calc.next = true;
  }
  else{
    my_calc.store(1);
    my_calc.op = Op;
    my_calc.next = true;
  }

  }

  void num_press(char num){
  if(my_calc.next == true){
      lcd.clear();
      my_calc.input = "";
      my_calc.next = false;
  }
  lcd.write(num);
  my_calc.input += num;
  }
  // ----------------------------
*/
