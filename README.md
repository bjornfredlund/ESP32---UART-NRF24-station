# ESP32 - UART/NRF24 station
---------
Project used to pass  messages via UART from desktop wirelessly to aonther ESP32 using NRF24-module. Second part of two-part project: [first part](https://github.com/bjornfredlund/ESP32---RC-interface).

Uses [esp-idf-mirf](https://github.com/nopnop2002/esp-idf-mirf) for NRF24-module communication.

### Modules used:
- NRF24LO1


### Connections:
  USB --> ESP32 UART
 |nRF24L01||ESP32|
|:-:|:-:|:-:|
|MISO|--|GPIO19|
|SCK|--|GPIO18|
|MOSI|--|GPIO23|
|CE|--|GPIO16|
|CSN|--|GPIO17|
|GND|--|GND|
|VCC|--|3.3V|


