# FTP and Web server 

This is the SD-Card example for esp-arduino 8266/ESP32/S2/S3/2C3/C6.
For the ESP32C6 you will need the Arduino Alpha 3 Release v3.0.0.
The SDCard must be FAT12 of FAT32, Max 32 GB 

You can find the instruction on youtube [https://www.youtube.com/watch?v=mrwjPOust5I&t=1s](https://www.youtube.com/watch?v=mrwjPOust5I&t=1s).

Copy the SDRoot to the root of your SD card so that index.htm is in the root.
The FTP username and password are in the password.txt file 
Load the software in the ESP, and now you can see all the content of the SD card and edit it by using the FTP connection.
To setup the WIFI the ESP will start up in AP ( access point )  mode connect to the ESP and setup the WIFI.
You can reset the WIFI password by pressing the boot key for 5 seconds.
Then the ESP will start in AP-mode 

For Filzilla you wil need to setup the Connection as Active 


![FileZilla settings](https://github.com/EmileSpecialProducts/FTP-WebServer/blob/main/FTP-WebServer/SdRoot/FilzilaSettings.png)

You can use the App Network Analyzer by Jiri Techet to find your ESP on the network 
https://play.google.com/store/apps/details?id=net.techet.netanalyzerlite.an&hl=en&pli=1
