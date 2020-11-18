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
int level = 1; // Keep track of what level the player is in

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
int touchPin = 12;
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

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Setup
void setup() {
  // put your setup code here, to run once:
  pinMode(micPin, INPUT);
  pinMode(touchPin, INPUT);
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  randomSeed(A3); // To randomize the numbers every different runtime
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  generateRandomProblem();
  while (currentlyInObstacle) {
    if (!printedOnce) {
      lcd.print("Obstacle Alert");
      delay(1000);
      lcd.clear();
      printedOnce = true;
    }
    stuckInObstacle();
  }
  delay(10);
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
  int num1 = 4;
  int num2 = 4;
  char operation = '+';
  String problemText = String(num1) + operation + String(num2) + " = ?";
  int solution;

  if (!printedOnce) {
    lcd.clear();
    lcd.print("LEVEL " + String(level));
    delay(1000);
    lcd.clear();
    lcd.print("4+4 = ?");
    printedOnce = true;
  }

  if (key) {
    if (key == '*') {
      lcd.clear();
      lcd.print(problemText);
      keyInput = ""; // Clears the input
      lcd.noCursor();
    } else if (key == '#') {
      solution = getArithmeticProblemSolution(num1, num2, operation);
      Serial.print(solution);
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

    } else {
      lcd.setCursor(0, 1);
      lcd.cursor();
      keyInput += key;
      lcd.print(keyInput);
    }

  }


}

char getRandomOperation(char operation) {
  switch (operation) {
    case '+': break;
    default: break;
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

void stuckInObstacle() { // Makes player stuck in an in-between obstacle
  switch (level) {
    default:
      holdObstacle();
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
  Serial.print(newTemperature - temperature);

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
    Serial.print("im here");
    secondPrintFlag = true;
  }

  if (abs(potentioOutput - potentioTarget) > 0) {
    potentioInitial = potentioOutput;
    lcd.setCursor(0, 1);
    lcd.print(String((potentioOutput / 255.0) * 100) + "%");
    lcd.setCursor(0, 0);
    Serial.println(potentioInitial);
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
  }



  Serial.println(potentioOutput);
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
  Serial.println(micInput, DEC);
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
