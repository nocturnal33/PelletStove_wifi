> [!CAUTION]
> ADD AT YOUR OWN RISK! <br>
> THIS CONTROLS FIRE IN YOUR HOUSE!<br>
> IF YOU ARE NOT COMFORTABLE WITH ANY OF THIS, WALK AWAY

# Add wifi to your PelPro 130
* This has only been tested on a PelPro 130 *

## Purpose
The purpose of this is to be able to turn on the pellet stove from anywhere in the house. Maybe it's 2am and it's getting cold in the house. Go to the IP address, login, and turn it on. You cann turn it off, adjust heat level, all from the comfort of your bed.<br>
You still have to make sure you have Pellets loaded, the burn pot is clean, etc... <br>
This project assumes you are at home, near the PelPro if something were to go wrong - you will be there to monitor the pellet stove.

## Parts: 
- EMAX Servo ES08A (x1)
- ELEGOO ESP-32 (Or similar) (x1)
- 8mm x 2mm Round Magnets (x3)
- M2 X4 Hex screws (x5)
-   Forr Servo (x2)
-   for ESP32 (x3)

## 3D print the parts
- Servo Motor / ESP case
- Servo Spur gear
- PelPro Gear

## Flash ESP32
- Use your IP Address / Subnet / DNS
- Create your password or omit it if needed

## Solder or connect servo wires to ESP32
- SERVO  -  RED    -  3.3v
- SERVO  -  BROWN  -  Ground
- SERVO  -  ORANGE -  GPIO18

After 3D printing parts - super glue magnets in enclosure, screw in the ESP32 using M2 screws, screw in the Servo using M2 screws.

1. Remove the knob on the PelPro 130 and replace with printed gear.
2. Attach the magnetic base to the pellet stove
3. Attach the spur gear to the servo
4. Align the gears
5. With the Pellet stove OFF, go to the IP Address and test the alignment
6. Go to the IP address you selected in the .ino file and enter the password you chose
7. Test
8. Turn ON the pellet stove and enjoy

## Manually adjusting heat on the PP130
Now that the servo is attached to the pellet stove, it will be harder to manually turn on the pellet stove. Keep an old phone or tablet near the PelPro 130 and use that instead of manually adjusting, or, because it is magnetically attached, move it away from the spur gear and adjust. 
