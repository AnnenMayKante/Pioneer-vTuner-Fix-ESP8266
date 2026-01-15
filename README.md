# Pioneer vTuner Fix (ESP8266)

vTuner (internet radio system) has not been working on Pioneer receivers for some time. Here is a method to fix this issue using an ESP8266.

## Requirements

| Component | Image |
| :--- | :---: |
| **ESP8266** | <img src="https://github.com/user-attachments/assets/9dee4826-adc9-4b4e-97b7-e224063bc89a" width="150"> |
| **Data Cable** | <img src="https://github.com/user-attachments/assets/e37f970c-40cb-4681-b5b3-6768db6b64a9" width="150"> |
| **PC** | (Arduino IDE Installed) |

## How to use
1. Open the `sketch_apr2a.ino` file.
2. Change the `your_ssid` and `your_pass` fields.
3. Flash the code to the card.
4. Find the IP address after your card is connected to Wi-Fi.
5. Change the **Primary DNS** in the Pioneer settings to the IP address if your card.

Now enjoy listening to internet radio without any problems
