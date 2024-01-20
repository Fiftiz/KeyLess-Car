# KeyLess-Car for ESP32

What is a RKS system

RKS as the Remote keyless system. It's a proximity system that is triggered if a key is within a certain distance.
Widely used in modern vehicles. It's very convenient to Entry your car when you have RKS system.
You do not need take out of your key, just walk to your car with the key in your pocket, then you can open the door.

 ![image](https://github.com/fryefryefrye/Open-Source-RKS/raw/master/img/pke_car.jpg)

# Use your iPhone as a Tag.

iPhone:

When IOS is enabling with continuity service, it will always sending BLE advertising to let other IOS device to know. But the Bluetooth MAC Address is random and changed every 15 minutes.
With my another project, you can decode the random address and determined if this random address belongs to your phone.

# Get IRK of your iPhone
If the IRK of an IOS device is known, the random Bluetooth address can be determined if it belongs to this device.

1- Use an ESP32 board and download with “esp32_get_irk” project and built to esp32. It will start a BLE service.

2- Use your iPhone install with “LightBlue” APP, find the “ESP_BLE_SECURITY” service, and connect it, the IRK will be print out.
![image](https://github.com/Fiftiz/KeyLess-Car/assets/51287497/75a2ad95-5e8d-4e47-abc9-2e1615555771)


# Hardware used:
iPhone; ESP32 Board
