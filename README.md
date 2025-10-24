> [!WARNING]
> ADD AT YOUR OWN RISK!
> THIS CONTROLS FIRE IN YOUR HOUSE!
> IF YOU ARE NOT COMFORTABLE WITH ANY OF THIS, WALK AWAY

# Add wifi to your pelpro 130
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
