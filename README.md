# ESP8266-RC-SWITCH
Control [cheap rc switches](http://amzn.to/1Or6jne) over MQTT using an ESP8266. Requires an ESP8266* and any [433MHz ASK transmitter](http://amzn.to/1rJhtir). 

\* I prefer the [WeMos D1 Mini](http://www.wemos.cc/Products/d1_mini.html)

# Wiring diagram

    ESP8266 5V  : RF module VCC
    ESP8266 GND : RF module GND
    ESP8266 D1  : RF module DATA

# Protocol
By default, this accepts messages on the `rfsend` MQTT topic. It expects strings composed of '0', '1', and 'F'. These strings should be twelve characters long - the first eight values are the ID and the last four are the state. The ID can be any eight characters; the state should be `0110` for ON and `0101` for OFF. For example, for device `1F0F11FF`, `1F0F11FF0110` is on and `1F0F11FF0101` is off.
