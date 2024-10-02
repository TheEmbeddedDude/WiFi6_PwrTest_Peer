# WiFi6_PwrTest

## Tested with ESP-IDF v5.2.1

### Hardware to use
The following parts have been used to build a WiFi6 sensor node to measure the power used to report the sensor data using different technologies e.g. ESP-NOW, iTWT etc.
1. ESP32-C6-DevKitC-1
2. TSL2591 Light sensor --> For example the Adafruit TSL2591 High Dynamic Range Digital Light Sensor - STEMMA QT, Product ID: 1980
3. SHT40 or SHT45 Temperature and Humidity Sensor --> Can be found from different vendors
4. I am using external pull ups for the I2C communication that are turned on and off with the power switch (see below)
5. PowerSwitch to turn on and off the sensors. Use two mosfets (see schematics pdf)
6. ESP-NOW testing requires a 2nd ESP32 device. I am using a 2nd ESP32-C6-DevKitC-1

#### Measurement equipment
If you want to measure the power consumption you need to have a power profiler such as the Nordic Power Profiler KitII. 

##### How to use the code
Please use the SDK Configuration editor (using VSCode) or the menuconfig when using the command line, to configure the application.
See below the settings for each method

                                                                      |                                 Power saving methods
Menu                     | Parameter                                  | iTWT+AutoLightSleep | DeepSleep   | ESP-NOW_LightSleep                    | ESP-NOW_DeepSleep
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
GPIO configuration       | I2C sensor power enable/disable pin Number |        Pin nr.      |  Pin nr.    |   Pin nr.                             |   Pin nr.

I2C configuration        | SCL GPIO Number                            |        Pin nr.      |  Pin nr.    |   Pin nr.                             |   Pin nr.           
I2C configuration        | SDA GPIO Number                            |        Pin nr.      |  Pin nr.    |   Pin nr.                             |   Pin nr.           
I2C configuration        | I2C clock frequency, Hz                    |        100000       |  100000     |   100000                              |   100000

Power Test Configuration | Application power saving method            |   Auto Light Sleep  | Deep Sleep  |  Light Sleep with ESP-NOW. Not MQTT   |   Deep Sleep with ESP-NOW. Not MQTT
Power Test Configuration | App reporting interval in seconds          |                             User defined. Change to test different reporting intervals
Power Test Configuration | Wi-Fi Power save mode                      |    Recommended to use "Maximum modem power saving" for max power saving
Power Test Configuration | Maximum CPU frequency                      |                                             160Mhz            
Power Test Configuration | Minimum CPU frequency                      |                                              10Mhz

Wi-Fi Configuration      | WiFi SSID                                  |            User defined           |                    NA for ESP-NOW
Wi-Fi Configuration      | WiFi Password                              |            User defined           |                    NA for ESP-NOW
Wi-Fi Configuration      | Maximum retry                              |            Default is 6 retires   |                    NA for ESP-NOW
Wi-Fi Configuration      | WiFi Scan Method                           |            Default                |                    NA for ESP-NOW
Wi-Fi Configuration      | WiFi minimum rssi                          |            Default -127           |                    NA for ESP-NOW

WiFi Scan threshold      | WiFi Scan auth mode threshold              |            User defined           |                    NA for ESP-NOW
WiFi Scan threshold      | WiFi Connect AP Sort Method                |            User defined           |                    NA for ESP-NOW

IP Configuration         | Obtain IPv6 address                        |            User defined           |                    NA for ESP-NOW
IP Configuration         | enable static ip                           |            Recommeneded           |                    NA for ESP-NOW
IP Configuration         | Static IP address                          |            User defined           |                    NA for ESP-NOW            
IP Configuration         | Static netmask address                     |            User defined           |                    NA for ESP-NOW                 
IP Configuration         | Static gateway address                     |            User defined           |                    NA for ESP-NOW                 

iTWT Configuration       | iTWT enabled                               |  True               |  false      | false                                 |  false
iTWT Configuration       | trigger-enabled                            |  True (default)     |  NA         | NA                                    |  NA         
iTWT Configuration       | announced                                  |  True (default)     |  NA         | NA                                    |  NA
iTWT Configuration       | itwt minimum wake duration                 |  User defined       |  NA         | NA                                    |  NA               
iTWT Configuration       | itwt wake interval exponent                |  User defined       |  NA         | NA                                    |  NA                
iTWT Configuration       | itwt wake interval mantissa                |  User defined       |  NA         | NA                                    |  NA                
iTWT Configuration       | itwt connection id                         |  0 (default)        |  NA         | NA                                    |  NA     
iTWT Configuration       | TWT setup timeout in µSec                  |  5000 (default)     |  NA         | NA                                    |  NA               
iTWT Configuration       | iTWT ASUS AP bug workaround enabled        |  false (default)    |  NA         | NA                                    |  NA                                             
iTWT Configuration       | ASUS AP deauthenticates interval in sec    |  300sec (Default)   |  NA         | NA                                    |  NA

ESP-NOW application data | ESP-NOW enabled                            |   disable           |  disable    |   Enable                              | Enable
ESP-NOW application data | ESPNOW primary master key                  |       NA            | NA          | User defined                          | User defined               
ESP-NOW application data | ESP-NOW local master key                   |       NA            | NA          | User defined                          | User defined               
ESP-NOW application data | Channel                                    |       NA            | NA          | User defined                          | User defined                   
ESP-NOW application data | MAC address (hex) of peer device we -      |       NA            | NA          | Insert mac from 2nd ESP32             | Insert mac from 2nd ESP32                                                
                         | report the sensor data to                  |                      
                   
Backend Configuration    | MQTT Broker IP address                     |            User defined           |                    NA for ESP-NOW
Backend Configuration    | MQTT broker port                           |            User defined           |                    NA for ESP-NOW             
Backend Configuration    | User name to access MQTT broker            |            User defined           |                    NA for ESP-NOW                            
Backend Configuration    | User password for MQTT broker access       |            User defined           |                    NA for ESP-NOW                                 
Backend Configuration    | Device ID for this device                  |            User defined           |                    NA for ESP-NOW                      
Backend Configuration    | Device location                            |            User defined           |                    NA for ESP-NOW            
Backend Configuration    | Quality of Service (QoS) for messages -    |            User defined           |                    NA for ESP-NOW                                    
                         | to be send                                 |       
Sleep Config             | Power down flash in light sleep when -     | Enable              | NA          |  Enable                               | NA
                         | there is no SPIRAM                         |                     |
Sleep Config             | light sleep GPIO reset workaround          | Enable              | NA          |  Enable                               | NA

Power Management         | Support for power management               | Enable              | Enable      |  Enable                               | Enable
Power Management         | Enable dynamic frequency scaling -         | Enable              | Enable      |  Enable                               | Enable
                         | (DFS) at startup                           |                     |
Power Management         | Disable all GPIO when chip at sleep        | Enable              | Enable      |  Enable                               | Enable
Power Management         | Power down CPU in light sleep              | Enable              | NA          |  Enable                               | NA
Power Management         | Power down Digital Peripheral in -         | Enable              | NA          |  Enable                               | NA  
                         | light sleep (EXPERIMENTAL)                 |                     | 

PHY                      | Power down MAC and baseband of Wi-Fi -     | Enable              | NA          |  NA                                   | NA      
                         | and Bluetooth when PHY is disabled         |                     |                

Wi-Fi                    | Power Management for station -             | Enable              | Enable      |                    NA for ESP-NOW                    
                         | at disconnected                            |                     |

ESP System Settings      | CPU frequency                              |                     |          User defined

FreeRTOS-->Kernel        | configUSE_TICKLESS_IDLE                    | Enable              | NA          |  Enable                               | NA
LWIP                     | DHCP coarse timer interval(s)              | >10 (see workaround)| NA          |             Not releveant for ESP-NOW
LWIP                     | GARP timer interval(seconds)               | 60                  | 60          |             Not releveant for ESP-NOW 