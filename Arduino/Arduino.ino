#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#include <SoftwareSerial.h>

const char number_of_rows = 4;
const char number_of_columns = 4;
const int ledPin = A0;    //Led
const int servoPin = 11;  //servo
const int buzzer = 12;  //buzzer

int attempts = 3;
int lockSecs = 10;
int mins = 0;
int secs = 0;
String status,error;
boolean isWarning = false;

char row_pins[number_of_rows] = {13, 10, 9, 8};
char column_pins[number_of_columns] = {7, 6, 5, 4};
char key_array[number_of_rows][number_of_columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

boolean doorState = false;
int defaultPassword = 9999;

Keypad k = Keypad(makeKeymap(key_array), row_pins , column_pins, number_of_rows, number_of_columns);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo;
SoftwareSerial mySerial(2, 3);

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzer, OUTPUT);

  lcd.begin(16, 2);
  lcd.backlight();

  myservo.attach(servoPin);
  myservo.write(180);
}

void loop() {
  if (!doorState) {
    lcd.setCursor(0, 0);
    lcd.print("Smart Lock - LMT");
  }
 
  char key = k.getKey();
  if (!doorState) {
      if (Serial.available()) 
      { // Kiểm tra nếu có dữ liệu từ ESP8266
      String receivedData = Serial.readString(); // Đọc dữ liệu từ ESP8266
      Serial.print("Received from ESP8266: ");
      Serial.println(receivedData); // In dữ liệu nhận được lên Serial Monitor
      if (receivedData.startsWith("Door Status: ")) {
        status = receivedData.substring(13);
        status.trim();
        Serial.println(status);
        Serial.println(doorState);
        if(status.equals("Closed") && doorState){
          Serial.println("-->Closed");
          closeDoor();
        }
        if(status.equals("Opened") && doorState==0){
        Serial.println("-->Opened");
        openDoor();
        delay(5000);
        closeDoor();
        doorState = false;
      }
    }
  }
  if (key == 'A'){
    

  }
  if (key == 'C') {
    changePassword();
  }
  if (key == 'D') {
    checkPass();
  }

  } else if (doorState) {
    delay(5000);
    closeDoor();
  }
}


void checkFaceId()
{
  
}
void changePassword() {
  lcd.clear();
  int oldPass = enterPass("Enter old pass:");
  
  if (oldPass == defaultPassword) {
    lcd.clear();
    int newPass1 = enterPass("Enter new pass:");
    int newPass2 = enterPass("Confirm new pass:");
    
  Serial.println(newPass1);
  Serial.println(newPass2);
  
    if (newPass1 == newPass2) {
      if (oldPass == defaultPassword) {
        defaultPassword = newPass1;      
      }
      lcd.clear();
      lcd.print("Password Changed");
      delay(2000);
    } else {
      lcd.clear();
      lcd.print("Mismatch! Try");
      lcd.setCursor(0, 1);
      lcd.print("again");
      delay(2000);
    }
  } else {
    lcd.clear();
    lcd.print("Wrong old pass");
    delay(2000);
  }
  lcd.clear();
}


void checkPass() {
  if (attempts == 0) {
    attempts = 1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Try again in");
    for (secs = lockSecs; secs >= 0; secs--) {
      lcd.setCursor(0, 1);
      lcd.print(secs);
      delay(1000);
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }

    if (isWarning == true) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Disable Unlock!");
      shortBuzzer();
      lockSecs = lockSecs * 2;
      if (lockSecs > 59) {
        lockSecs = 59;
      }
    }

    lcd.clear();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);

    int enteredPass = enterPass("Enter Password:");

    if (enteredPass == defaultPassword) {
      openDoor();    
      attempts = 3;
      isWarning = false;
      lockSecs = 10;
    } else {
      wrongPass();
      if (attempts == 0) {
        isWarning = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Disable Unlock!");
        shortBuzzer();
      }
      delay(1500);
      lcd.clear();
    }
  }
}


//Type password
int enterPass(String msg) {
  int password = 0;
  int count = 0;
  char key;
  lcd.clear();
  lcd.print(msg);
  do {
    key = k.getKey();
    if (key >= '0' && key <= '9') {
      password = password * 10 + (key - '0');
      lcd.setCursor(count, 1);
      lcd.print("*");
      count++;
    } else if (key == '*') {
      password = 0;
      count = 0;
      lcd.clear();
      lcd.print(msg);
    } else if (key == 'C' && count > 0) {
      count--;
      password /= 10;
      lcd.setCursor(count, 1);
      lcd.print(" ");
      lcd.setCursor(count, 1);
    }
  } while (count != 4);
  return password;
}

//Open door
void openDoor() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Correct Password");
  digitalWrite(ledPin, HIGH);
  myservo.write(0); //servo
  longBuzzer();
  lcd.setCursor(2, 1);
  lcd.print("Door Opened");
  doorState = true;
}
//Close door
void closeDoor() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Door Closed");
  digitalWrite(ledPin, LOW);
  myservo.write(180); //servo
  longBuzzer();
  doorState = false;
  delay(1000);
  lcd.clear();
}

//Wrong password
void wrongPass() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wrong password");
  //buzzer
  shortBuzzer();
  lcd.setCursor(3, 1);
  lcd.print("Try Again");
  delay(500);
  lcd.clear();
  attempts--;
}

void longBuzzer() {
  tone(buzzer, 1000);
  delay(1000);
  noTone(buzzer);
}

void shortBuzzer() {
  tone(buzzer, 1000);
  digitalWrite(ledPin, HIGH);
  delay(500);
  noTone(buzzer);
  digitalWrite(ledPin, LOW);
  delay(100);

  tone(buzzer, 1000);
  digitalWrite(ledPin, HIGH);
  delay(500);
  noTone(buzzer);
  digitalWrite(ledPin, LOW);
  delay(100);

  tone(buzzer, 1000);
  digitalWrite(ledPin, HIGH);
  delay(500);
  noTone(buzzer);
  digitalWrite(ledPin, LOW);

}
