/*

Insanity Radio - ArduGPIO

*/
String commandString;
int minDigitalPin;
int maxDigitalPin;
int minAnalogPin;
int maxAnalogPin;
bool commandIssued;
bool digitalStates[50];
int analogValues[25];
void setup() { 
  Serial.begin(19200);
  minDigitalPin = 2;
  maxDigitalPin = 13;
  minAnalogPin = 0;
  maxAnalogPin = 5;
  for(int i=minDigitalPin;i<=maxDigitalPin;i++) {
    digitalStates[i] = false;
  }
  for(int i=minAnalogPin;i<=maxAnalogPin;i++) {
    analogValues[i] = 0;
  }
  commandIssued = false;
  commandString = "";
  commandString.reserve(200);
  Serial.println("ArduGPIO 1.0 Ready");
  Serial.println("Available commands: PINMODE PINNUM IN|OUT, SETPIN PINNUM HIGH|LOW, GETPIN PINNUM, AREAD PINNUM");
  Serial.print("Available digital pins: ");
  Serial.print(minDigitalPin);
  Serial.print(" to ");
  Serial.print(maxDigitalPin);
  Serial.print(", available analog pins: ");
  Serial.print(minAnalogPin);
  Serial.print(" to ");
  Serial.print(maxAnalogPin);
  Serial.print("\n");
  Serial.println("Pin change notifications: EDGE PINNUM RISING|FALLING HIGH|LOW (latter is new state), ACHANGE PINNUM VALUE");
}
boolean pinInRange(int pin, boolean digital) {
  if (digital) {
    if (pin < maxDigitalPin && pin > minDigitalPin) {
      return true;
    } else {
      return false;
    }
  } else {
    if (pin < maxAnalogPin && pin > minAnalogPin) {
      return true;
    } else {
      return false;
    }
  }
}
void loop() {
  // Scan all pins, anything changes, report it.
  for(int i=minDigitalPin;i<=maxDigitalPin;i++) {
    boolean val = digitalRead(i);
    if (digitalStates[i] != val) {
      Serial.print("EDGE ");
      Serial.print(i);
      if (digitalStates[i]) {
        Serial.print(" FALLING");
      } else {
        Serial.print(" RISING");
      }
      if (val) {
        Serial.print(" HIGH\n");
      } else {
        Serial.print(" LOW\n");
      }
      digitalStates[i] = val;
    }
    delayMicroseconds(15);
  }
  for(int i=minAnalogPin;i<=maxAnalogPin;i++) {
    int val = analogRead(i);
    if (val > (analogValues[i]+100) || val < (analogValues[i]-100)) {
      Serial.print("ACHANGE ");
      Serial.print(i);
      Serial.print(" ");
      Serial.print(val);
      Serial.print("\n");
      analogValues[i] = val;
    }
    delayMicroseconds(15);
  }
  // Handle any commands
  if (commandIssued) {
    // Handle the command.
    // PINMODE PINNUM IN|OUT - sets selected digital pin to be an input or output
    // SETPIN PINNUM HIGH|LOW - sets selected digital pin high or low
    // GETPIN PINNUM - gets the selected digital pin's state (high/low)    
    // AREAD PINNUM - gets the selected analogue pin's raw value
    Serial.print("Got command: ");
    Serial.print(commandString);
    if (commandString.startsWith("PINMODE")) {
      int firstSpace = commandString.indexOf(" ");
      int secondSpace = commandString.indexOf(" ",firstSpace+1);
      char buf[25];
      commandString.substring(firstSpace+1,secondSpace).toCharArray(buf, sizeof(buf));
      int pinNumber = atoi(buf);
      if (pinInRange(pinNumber, true)) {
        String modeToSet = commandString.substring(secondSpace+1,commandString.length());
        Serial.print("PINMODE ");
        Serial.print(pinNumber);
        if (modeToSet.startsWith("IN")) {
          pinMode(pinNumber, INPUT);
          Serial.print(" INPUT\n");
        } else {
          pinMode(pinNumber, OUTPUT);
          Serial.print(" OUTPUT\n");
        }
      } else {
        Serial.println("RANGEERROR");
      }
    } else if (commandString.startsWith("SETPIN")) {
      int firstSpace = commandString.indexOf(" ");
      int secondSpace = commandString.indexOf(" ",firstSpace+1);
      char buf[25];
      commandString.substring(firstSpace+1,secondSpace).toCharArray(buf, sizeof(buf));
      int pinNumber = atoi(buf);
      if (pinInRange(pinNumber, true)) {
        String stateToSet = commandString.substring(secondSpace+1,commandString.length());
        Serial.print("SETPIN ");
        Serial.print(pinNumber);
        if (stateToSet.startsWith("HIGH")) {
          digitalWrite(pinNumber, HIGH);
          Serial.print(" HIGH\n");
        } else {
          digitalWrite(pinNumber, LOW);
          Serial.print(" LOW\n");
        }
      } else {
        Serial.println("RANGEERROR");
      }
    } else if (commandString.startsWith("GETPIN")) {
      int firstSpace = commandString.indexOf(" ");
      char buf[25];
      commandString.substring(firstSpace+1,commandString.length()).toCharArray(buf, sizeof(buf));
      int pinNumber = atoi(buf);
      if (pinInRange(pinNumber, true)) {
        Serial.print("GETPIN ");
        Serial.print(pinNumber);
        if (digitalRead(pinNumber) == HIGH) {
          Serial.print(" HIGH\n");
        } else {
          Serial.print(" LOW\n");
        }
      } else {
        Serial.println("RANGEERROR");
      }
    } else if (commandString.startsWith("AREAD")) {
      int firstSpace = commandString.indexOf(" ");
      char buf[25];
      commandString.substring(firstSpace+1,commandString.length()).toCharArray(buf, sizeof(buf));
      int pinNumber = atoi(buf);
      if (pinInRange(pinNumber, false)) {
        Serial.print("AREAD ");
        Serial.print(pinNumber);
        Serial.print(" ");
        Serial.print(analogRead(pinNumber));
        Serial.print("\n");
      } else {
        Serial.println("RANGEERROR");
      }
    } else {
      Serial.println("UNKNOWNCMD");
    }
    // 
    // Reset the command issued flag and input command.
    commandIssued = false;
    commandString = "";
  }
}
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    commandString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      commandIssued = true;
    } 
  }
}
