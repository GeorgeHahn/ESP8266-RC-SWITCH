# ESP8266-RC-SWITCH
Control [cheap](http://amzn.to/1Or6jne) [rc switches](http://amzn.to/1sjnX8n) over MQTT using an ESP8266. Requires an ESP8266* and any [433MHz ASK transmitter](http://amzn.to/1rJhtir). 

\* I prefer the [WeMos D1 Mini](http://www.wemos.cc/Products/d1_mini.html)

# Wiring diagram

    ESP8266 5V  : RF module VCC
    ESP8266 GND : RF module GND
    ESP8266 D1  : RF module DATA
