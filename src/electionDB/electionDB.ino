#include <EEPROM.h>

const int totalVoters = 128;  // Adjust based on the max voter slots

void setup() {
    Serial.begin(9600);
    Serial.println("Type 'database' to retrieve voter data.");
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');  // Read input from Serial Monitor
        command.trim();  // Remove any extra whitespace or newline characters

        if (command.equalsIgnoreCase("database")) {
           int enrolledVoters = 0, votedUsersCount = 0, pendingUsersCount = 0;

           for (int i = 1; i <= 127; i++) {
               byte status = EEPROM.read(i);  // Read status from EEPROM for each user

               // Only print if status is 0 or 1
               if (status == 0 || status == 1) {
                   enrolledVoters++;  // Increment total enrolled voters

                   String votedStatus = (status == 1) ? "Yes" : "No";
                   
                   // Print user ID and voting status
                   Serial.print("User ID: ");
                   Serial.print(i);
                   Serial.print(" Voted: ");
                   Serial.println(votedStatus);
                   
                   // Increment votedUsersCount or pendingUsersCount
                   if (status == 1) {
                       votedUsersCount++;
                   } else {
                       pendingUsersCount++;
                   }
               }
           }

           // Read results stored in EEPROM at addresses 200 and 400
           byte resultA = EEPROM.read(200);
           byte resultB = EEPROM.read(400);

           // Output the results
           Serial.print("A: ");
           Serial.println(resultA);
           Serial.print("B: ");
           Serial.println(resultB);

           // Output the enrolled, voted, and pending users count
           Serial.print("Enrolled Voters: ");
           Serial.println(enrolledVoters);
           Serial.print("Voted: ");
           Serial.println(votedUsersCount);
           Serial.print("Pending Voters: ");
           Serial.println(pendingUsersCount);
        }
    }
}
