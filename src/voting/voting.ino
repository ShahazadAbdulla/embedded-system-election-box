#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <EEPROM.h>  // EEPROM for storing voter data

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
#define GREEN_LED_PIN 11     // Define the Green LED pin (for successful voting)

int userID = 0;
int votes[2] = {0, 0}; // Only two parties now
bool votedUsers[128] = {false}; // Track users who have voted

// Push buttons for voting (Party A & B)
#define PARTY_A_BUTTON 10
#define PARTY_B_BUTTON 12

#define EEPROM_VOTER_START 0  // Stores voting status per user (1 byte per ID)
#define EEPROM_VOTES_A 200     // Stores votes for Party A
#define EEPROM_VOTES_B 400     // Stores votes for Party B

void setup() {
  Serial.begin(57600); // Fingerprint sensor on Hardware Serial1
  delay(3000);  

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
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint OK");
  } else {
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
  lcd.print("C:Result D:Clear");

  char choice = waitForKeypress();

  if (choice == 'A') {
    enrollFingerprint();
  } else if (choice == 'B') {
    vote();
  } else if (choice == 'C') {
    displayResults();
  } else if (choice == 'D') {
    clearElectionData();
  }
}

// ADMIN LOGIN
void authenticateAdmin() {
  int attempts = 0;
  while (attempts < 11) {
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

void clearElectionData() {
  lcd.clear();
  lcd.print("Clearing Data...");
  delay(2000);

  // Clear EEPROM (Reset stored voter data)
  for (int i = 0; i < 128; i++) {
    EEPROM.update(EEPROM_VOTER_START + i, 2);
  }
  EEPROM.put(EEPROM_VOTES_A, 0);
  EEPROM.put(EEPROM_VOTES_B, 0);

  // Clear Fingerprint Database
  finger.emptyDatabase();

  // Reset vote counts and voted user tracking
  votes[0] = 0;
  votes[1] = 0;
  memset(votedUsers, false, sizeof(votedUsers));

  
  // Ensure LCD stays on properly
  lcd.clear();
  lcd.print("Data Cleared!");
  delay(3000);  // Increased delay to prevent LCD flickering
}

void enrollFingerprint() {
  int enteredID = 0;

  while (true) {  
    lcd.clear();
    lcd.print("Enter ID:");

    String idInput = "";
    while (true) {
      char c = waitForKeypress();
      if (c >= '0' && c <= '9') {
        idInput += c;
        lcd.print(c);
      } else if (c == 'D') {
        break;
      }
    }

    enteredID = idInput.toInt();
    if (enteredID >= 1 && enteredID <= 127) {
      if (finger.loadModel(enteredID) == FINGERPRINT_OK) {
        lcd.clear();
        lcd.print("ID Taken!");
        delay(2000);
        return;
      }
      break;
    } else {
      lcd.clear();
      lcd.print("Invalid ID!");
      delay(2000);
    }
  }

  userID = enteredID;
  if (captureFingerprint(userID)) {
    EEPROM.write(EEPROM_VOTER_START + userID, 0);
    lcd.clear();
    lcd.print("Enroll Success!");
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay(2000);
    digitalWrite(GREEN_LED_PIN, LOW);
  } else {
    lcd.clear();
    lcd.print("Enroll Failed!");
    for (int i = 0; i < 1; i++) {
      digitalWrite(BUZZER_LED_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_LED_PIN, LOW);
      delay(500);
    }
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

  int attempts = 4;  // Allow 5 attempts to recognize the fingerprint
  int voterID = -1;

  while (attempts > 0) {
    if ((voterID = matchFingerprint()) > 0) {
      break;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Place Finger...");
    delay(2000);  // Wait for 2 seconds before retrying
    attempts--;
  }

  if (voterID == -1) {
    
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

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press A or B");

  while (true) {
    if (digitalRead(PARTY_A_BUTTON) == LOW) {
      EEPROM.get(EEPROM_VOTES_B, votes[1]);
      EEPROM.get(EEPROM_VOTES_A, votes[0]);
      votes[0]++;
      EEPROM.put(EEPROM_VOTES_A, votes[0]);
      EEPROM.put(EEPROM_VOTES_B, votes[1]);
      break;
    } else if (digitalRead(PARTY_B_BUTTON) == LOW) {
      EEPROM.get(EEPROM_VOTES_B, votes[1]);
      votes[1]++;
      EEPROM.put(EEPROM_VOTES_B, votes[1]);
      break;
    }
  }

  EEPROM.write(EEPROM_VOTER_START + voterID, 1); // Mark as voted
  votedUsers[voterID] = true;
  
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

  int votesA = 0, votesB = 0;
  EEPROM.get(EEPROM_VOTES_A, votesA);
  EEPROM.get(EEPROM_VOTES_B, votesB);

  int totalVoters = 0, votedUsers = 0;
  for (int i = 1; i <= 127; i++) {
    byte status = EEPROM.read(EEPROM_VOTER_START + i);
    if (status == 0 || status == 1) totalVoters++; 
    if (status == 1) votedUsers++; 
  }

  int pendingVoters = totalVoters - votedUsers;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enrl:");
  lcd.print(totalVoters);
  lcd.setCursor(8, 0);
  lcd.print("Vot:");
  lcd.print(votedUsers);
  lcd.setCursor(0, 1);
  lcd.print("Pend:");
  lcd.print(pendingVoters);
  delay(4000);

  lcd.clear();
  lcd.print("A:");
  lcd.print(votesA);
  lcd.setCursor(8, 0);
  lcd.print("B:");
  lcd.print(votesB);
  delay(4000);

}

// CAPTURE FINGERPRINT (Enhanced with Retry & Proper Storage)
bool captureFingerprint(int id) {
  int p;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger...");

  // Step 1: Capture first fingerprint scan (keep trying until successful)
  while (true) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      p = finger.image2Tz(1);
      if (p == FINGERPRINT_OK) break;  // Proceed to the next step
    }
    delay(1000);  // Wait 1 second before retrying
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wait...");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Again...");

  // Step 2: Capture second fingerprint scan (keep trying until successful)
  while (true) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      p = finger.image2Tz(2);
      if (p == FINGERPRINT_OK) break;  // Proceed to model creation
    }
    delay(1000);  // Wait 1 second before retrying
  }

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
