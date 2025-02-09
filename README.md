# **🗳️ Election Box – Secure Fingerprint-Based Voting System**
## **🔹 Overview**
Election Box is a biometric-based electronic voting system that ensures a secure, transparent, and tamper-proof electoral process. Using **fingerprint authentication**, a **keypad interface**, and an **LCD display**, this system eliminates multiple voting fraud by allowing each registered voter to cast only one vote. Once a vote is cast, the fingerprint data is deleted, ensuring a **one-person, one-vote** policy.

## **🔹 Features**
✅ **Fingerprint Authentication** – Uses a biometric fingerprint sensor for secure voter identification.

✅ **Admin-Controlled System** – PIN-protected access for enrollment, voting, and result viewing.

✅ **Real-Time Vote Counting** – Displays live election results.

✅ **Secure & Tamper-Proof** – Deletes fingerprint after voting to prevent multiple votes.

✅ **User-Friendly Keypad Interface** – Simple navigation for both voters and administrators.

✅ **LCD Display** – Provides clear feedback and instructions.

✅ **Standalone System** – Works without the need for a network or external database.

✅ **Component Testing Scripts** – Separate test programs for fingerprint sensor, keypad, and LCD.

## **🔹 Hardware Used**
- **Adafruit Fingerprint Sensor** – Captures and verifies fingerprints.
- **4x4 Keypad** – Used for entering PINs and selecting options.
- **I2C LCD Display (HW-61)** – Provides real-time system updates.
- **Arduino (or compatible microcontroller)** – Processes voter authentication and results.

## **🔹 Required Libraries**
Make sure you have the following libraries installed in **Arduino IDE**:

#include <Adafruit\_Fingerprint.h>  // Fingerprint Sensor Library
#include <Keypad.h>                // Keypad Library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>      // LCD Display Library
How to Install:
Open Arduino IDE
Go to Sketch → Include Library → Manage Libraries
Search for the required library and click Install

🔹 How It Works
1️⃣ Admin Login – The system starts in admin mode, requiring a PIN (0000) for access.
2️⃣ Enrollment – Admin can register new voters by scanning and saving their fingerprints.
3️⃣ Voting – Voters authenticate using their fingerprint and cast their vote.
4️⃣ Vote Security – After voting, the system deletes the voter’s fingerprint to prevent re-voting.
5️⃣ Results Display – Admin can check election results at any time.

## **🔹 How It Works**
1️⃣ **Admin Login** – The system starts in admin mode, requiring a PIN (0000) for access.

2️⃣ **Enrollment** – Admin can register new voters by scanning and saving their fingerprints.

3️⃣ **Voting** – Voters authenticate using their fingerprint and cast their vote.

4️⃣ **Vote Security** – After voting, the system deletes the voter’s fingerprint to prevent re-voting.

5️⃣ **Results Display** – Admin can check election results at any time.

## **🔹 Testing the Components**
Before running the main voting system, test each hardware component using the provided test scripts:

✅ **Test the Fingerprint Sensor**

- Open test/FingerPrintTest/FingerPrintTest.ino in Arduino IDE
- Upload and check if fingerprints are recognized correctly

✅ **Test the Keypad**

- Open test/KeyPadTesting/KeyPadTesting.ino
- Press keys and monitor the serial output

✅ **Test the LCD Display**

- Open test/LCDTest/LCDTest.ino
- Ensure the LCD displays text properly

## **🔹 How to Upload & Run**
### **Clone the Repository**
git clone [https://github.com/ShahazadAbdulla/Election-Box.git
](https://github.com/ShahazadAbdulla/Election-Box.git)
### **Open in Arduino IDE**
1. Open **Arduino IDE**
1. Navigate to src/voting/voting.ino
1. Select the correct **board and COM port**
1. Click **Upload**
1. Test the system by following the LCD instructions

## **🔹 Project Motivation**
**Election Box** was developed to eliminate voting fraud and ensure a **secure, transparent, and efficient** electoral process. This system is ideal for:

✔️ **Student elections**

✔️ **Small-scale organizations**

✔️ **Secure digital polling environments**

🔒 **Biometric security, embedded systems, and real-world IoT applications—all in one project!**

⭐ **If you find this project useful, give it a star on GitHub!**

🔗 **GitHub Repository 🚀**

[Election-Box](https://github.com/ShahazadAbdulla/Election-Box)

