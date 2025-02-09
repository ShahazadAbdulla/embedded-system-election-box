#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <SoftwareSerial.h>

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address 0x27, 16 cols, 2 rows

// Fingerprint Sensor Setup (Hardware Serial on 0 & 1)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial);

// Keypad Setup
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}  // 'D' is Enter
};
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#define BUZZER_LED_PIN 13    // Define the buzzer and LED pin (shared)
#define GREEN_LED_PIN 12     // Define the Green LED pin (for successful voting)

int userID = 0;
int votes[2] = {0, 0}; // Only two parties now
bool votedUsers[128] = {false}; // Track users who have voted

// Push buttons for voting (Party A & B)
#define PARTY_A_BUTTON 10
#define PARTY_B_BUTTON 11

void setup() {
  Serial.begin(9600);
  Serial.begin(57600); // Fingerprint sensor on Hardware Serial1

  pinMode(PARTY_A_BUTTON, INPUT_PULLUP);
  pinMode(PARTY_B_BUTTON, INPUT_PULLUP);

  pinMode(BUZZER_LED_PIN, OUTPUT); 
  pinMode(GREEN_LED_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Election Box");
  delay(2000);
  lcd.clear();

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor detected!");
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint OK");
  } else {
    Serial.println("Sensor NOT found!");
    lcd.setCursor(0, 0);
    lcd.print("Sensor ERROR!");
    while (1);
  }

  authenticateAdmin();
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A:Enroll B:Vote");
  lcd.setCursor(0, 1);
  lcd.print("C:Results D:Exit");

  Serial.println("\nChoose: A (Enroll), B (Vote), C (Results), D (Exit)");
  char choice = waitForKeypress();

  if (choice == 'A') {
    enrollFingerprint();
  } else if (choice == 'B') {
    vote();
  } else if (choice == 'C') {
    displayResults();
  } else if (choice == 'D') {
    Serial.println("Exiting...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Goodbye!");
    delay(2000);
    return;
  }
}

// ADMIN LOGIN
void authenticateAdmin() {
  int attempts = 0;
  while (attempts < 3) {
    Serial.println("\nEnter PIN & Press D:");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter PIN:");

    String pin = "";
    while (pin.length() < 4) {
      char c = waitForKeypress();
      if (c >= '0' && c <= '9') {
        pin += c;
        Serial.print("*");
        lcd.setCursor(pin.length(), 1);
        lcd.print("*");  // Mask PIN
      }
    }

    if (pin == "0000") {
      Serial.println("\nAccess Granted.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Granted!");
      delay(2000);
      return;
    } else {
      Serial.println("\nIncorrect PIN!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Incorrect PIN!");
      delay(2000);
    }
    attempts++;
  }

  Serial.println("Too many failed attempts!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Too many tries!");
  delay(2000);
}

// ENROLL FINGERPRINT
void enrollFingerprint() {
  int enteredID = 0;

  while (true) {  
    Serial.println("\nEnter ID (1-127) & D:");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter ID:");

    String idInput = "";
    while (true) {
      char c = waitForKeypress();
      if (c >= '0' && c <= '9') {
        idInput += c;
        Serial.print(c);
        lcd.setCursor(idInput.length(), 1);
        lcd.print(c);
      } else if (c == 'D') {
        break;
      }
    }

    enteredID = idInput.toInt();
    if (enteredID >= 1 && enteredID <= 127) {
      Serial.print("\nSaving to ID: ");
      Serial.println(enteredID);
      break;
    } else {
      Serial.println("\nInvalid ID!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Invalid ID!");
      delay(2000);
    }
  }

  userID = enteredID;
  Serial.println("Place finger...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger...");

  if (captureFingerprint(userID)) {
    Serial.println("Fingerprints Matched! Saved.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enroll Success!");
    delay(2000);
  } else {
    Serial.println("Enrollment Failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enroll Failed!");
    delay(2000);
  }
}

// VOTING FUNCTION (WITH 3 CHANCES & WAITING FOR FINGERPRINT)

void vote() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Verification");
  delay(1500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger");
  delay(1000);

  int attempts = 5;  // Allow 5 attempts to recognize the fingerprint
  int voterID = -1;

  while (attempts > 0) {
    if ((voterID = matchFingerprint()) > 0) {
      break;
    }
    Serial.println("Fingerprint NOT recognized! Try again.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Try Again...");
    delay(2000);  // Wait for 2 seconds before retrying
    attempts--;
  }

  if (voterID == -1) {
    Serial.println("No attempts left. Voting right lost.");
    
    // Trigger buzzer and red LED blinking if fingerprint is not recognized after 5 attempts
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_LED_PIN, HIGH);  // Turn on buzzer and LED
      delay(1000);                          // Wait for 500ms
      digitalWrite(BUZZER_LED_PIN, LOW);   // Turn off buzzer and LED
      delay(1000);                          // Wait for 500ms
    }

    // Inform the user of the failed attempts
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No Tries Left!");
    delay(2000);
    
    // Return to main menu
    return;  
  }

  if (votedUsers[voterID]) {
    Serial.println("User already voted!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Already Voted!");
    
    // Trigger buzzer and red LED blinking if the user already voted
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_LED_PIN, HIGH);  // Turn on buzzer and LED
      delay(500);                          // Wait for 500ms
      digitalWrite(BUZZER_LED_PIN, LOW);   // Turn off buzzer and LED
      delay(500);                          // Wait for 500ms
    } 
    return;
  }

  Serial.println("Fingerprint matched! Vote now.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press A or B");

  while (true) {
    if (digitalRead(PARTY_A_BUTTON) == LOW) {
      votes[0]++;
      break;
    } else if (digitalRead(PARTY_B_BUTTON) == LOW) {
      votes[1]++;
      break;
    }
  }

  votedUsers[voterID] = true;
  Serial.println("Vote registered!");
  
  // Turn on the Green LED for successful voting
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vote Success!");
  digitalWrite(GREEN_LED_PIN, HIGH);  
  delay(2000); // Keep the green LED on for 2 seconds to indicate success
  digitalWrite(GREEN_LED_PIN, LOW);  // Turn off the green LED after 2 seconds

}



// DISPLAY RESULTS
void displayResults() {
  Serial.println("\n--- RESULTS ---");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A:");
  lcd.print(votes[0]);
  lcd.setCursor(8, 0);
  lcd.print("B:");
  lcd.print(votes[1]);
  delay(5000);
}

// CAPTURE FINGERPRINT (Enhanced with Retry & Proper Storage)
bool captureFingerprint(int id) {
  int p;

  Serial.println("Waiting for finger...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger...");

  // Step 1: Capture first fingerprint scan
  while ((p = finger.getImage()) != FINGERPRINT_OK);
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return false;

  Serial.println("Remove finger...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Remove Finger...");
  delay(2000);

  Serial.println("Place the same finger again...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Again...");

  // Step 2: Capture second fingerprint scan
  while ((p = finger.getImage()) != FINGERPRINT_OK);
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return false;

  // Create the fingerprint model
  if (finger.createModel() != FINGERPRINT_OK) return false;

  // Store the fingerprint at the given ID
  if (finger.storeModel(id) != FINGERPRINT_OK) return false;

  return true;
}

int matchFingerprint() {
  if (finger.getImage() != FINGERPRINT_OK) return -1;
  if (finger.image2Tz(1) != FINGERPRINT_OK) return -1;
  return (finger.fingerFastSearch() == FINGERPRINT_OK) ? finger.fingerID : -1;
}

char waitForKeypress() {
  char key;
  while (!(key = keypad.getKey()));
  return key;
}
