# Zephyr RTOS Project Mpu6050 Sensor


## Overall Overview

Some Technologies used in This project are zephyr rtos , mpu6050 sensor ,Wifi , Websockets , esp32  MCU , Backend server and frontend for visualization . 
In this Project data from mpu6050 sensor is read that is accelerometer data  . X , Y and Z which is used to detect rotation from reference X-axis.
This or related sensor can be used in variety of products including wearables devices , drones and any project involve motion detections or orientations etc which is intresting. 
After working with sensor and doing some configuration according to the requirements , For example you can change the sensitivity , power modes and a lot other parameters . That is what Custom code give you control of. Then tje data is sent over Wifi and used websockets which can transfer data in real time with minimum or no delay . I used first TCP/IP which was slow, used just for practiced( Actually TCP will create a new connection with every request which is overhead, no need of making connection and closing again and again. One connection and then transfer real time data and dont close it , here comes the websocket) . The data is received by the server which was connected to the frontend . The frontend used three js for visualization . It shows the rotation from X-axis which I got from accelerometer data only . It can be further optimized if combined with gyroscope data . So shortly that’s it. 
## Some Technical Information 

After Some Work with sensor like wiring and doing some configuration according to the requirements ,Configurations inlcudes,  For example you can change the sensitivity , power modes and a lot other parameters . That is what Custom code give you control of. Then the data is sent over Wifi and used websockets which can transfer data in real time with minimum or no delay . I used first TCP/IP which was slow, used just for practiced( Actually TCP will create a new connection with every request which is overhead, no need of making connection and closing again and again. One connection and then transfer real time data and dont close it ,so that is websocket and finally here comes the websocket) . The data is received by the server which was connected to the frontend . The frontend used three js for visualAfter working with sensor and doing some configuration according to the requirements , For example you can change the sensitivity , power modes and a lot other parameters . That is what Custom code give you control of. Then tje data is sent over Wifi and used websockets which can transfer data in real time with minimum or no delay . I used first TCP/IP which was slow, used just for practiced( Actually TCP will create a new connection with every request which is overhead, no need of making connection and closing again and again. One connection and then transfer real time data and dont close it , here comes the websocket) . The data is received by the server which was connected to the frontend . The frontend used three js for visualization . It showsAfter working with sensor and doing some configuration according to the requirements , For example you can change the sensitivity , power modes and a lot other parameters . That is what Custom code give you control of. Then tje data is sent over Wifi and used websockets which can transfer data in real time with minimum or no delay . I used first TCP/IP which was slow, used just for practiced( Actually TCP will create a new connection with every request which is overhead, no need of making connection and closing again and again. One connection and then transfer real time data and dont close it , here comes the websocket) . The data is received by the server which was connected to the frontend . The frontend used three js for visualization . It shows the rotation from X-axis which I got from accelerometer data only . It can be further optimized if combined with gyroscope data . So shortly that’s it.  the rotation from X-axis which I got from accelerometer data only . It can be further optimized if combined with gyroscope data . So shortly that’s it. ization . It shows the rotation from X-axis which I got from accelerometer data only . It can be further optimized if combined with gyroscope data . So shortly that’s it. 

After working with sensor and doing some configuration according to the requirements , For example you can change the sensitivity , power modes and a lot other parameters . That is what Custom code give you control of. Then tje data is sent over Wifi and used websockets which can transfer data in real time with minimum or no delay . I used first TCP/IP which was slow, used just for practiced( Actually TCP will create a new connection with every request which is overhead, no need of making connection and closing again and again. One connection and then transfer real time data and dont close it , here comes the websocket) . The data is received by the server which was connected to the frontend . The frontend used three js for visualization . It shows the rotation from X-axis which I got from accelerometer data only . It can be further optimized if combined with gyroscope data . So shortly that’s it. 


## How Project IS BUILD
So starting for connection/wiring , it is pretty simple .I used five pins Let me show you how it is done in short form.

When you leave the ADO pin unconnected or GND it, the I2C address is 0x68HEX; when you connect it to 3.3V, the I2C address changes to
0x69HEX

Everything is set . So setup a project and devicetree according to your project . Also make sure you enable the technologies you are using . In my
case I used wifi , i2c , networking , dhcpv4 and others.

You may visit the Code for Complete Project to See every bit of Details.

See All Projects

Furthermore First We need to write driver for mpu6050 sensor . Which is the same long discussion previously done in first project . APIs from
zephyr rtos used such as .

int ret = i2c_reg_read_byte(i2c_dev, I2C_DEV_ADDR, reg_adrr, &read_data);

int ret = i2c_reg_write_byte(i2c_dev, I2C_DEV_ADDR, reg_adrr, data);

and others . Like for Wifi connection from wifi_mgmt library and networking etc.

The APIs above used for reading or writing into the sensor. After working with sensor like connection and testing and doing some configuration
according to the requirements , For example with configuration you can change the sensitivity , power modes and a lot other parameters . That is
what Custom code give you control of. Then the data is sent over Wifi and used websockets which can transfer data in real time with minimum or
no delay . I used first TCP/IP which was slow, used just for practiced( Actually TCP will create a new connection with every request which is
overhead, no need of making connection and closing again and again. One connection and then transfer real time data and dont close it , here
comes the websocket) . The data is received by the server which was connected to the frontend . The frontend used three js for visualization . It
shows the rotation from X-axis which I got from accelerometer data only . It can be further optimized if combined with gyroscope data . So
shortly that’s it.
MPU6050

VCC
GND
SCL
SDA
ADO GND
SDA(D21)
SCL(D22)
GND
VCC
STM32
