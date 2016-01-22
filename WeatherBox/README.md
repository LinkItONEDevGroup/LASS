## Arduino

### Library:
* DHT https://github.com/adafruit/DHT-sensor-library
* BH1750 https://github.com/claws/BH1750
* Si1145 https://github.com/adafruit/Adafruit_SI1145_Library
* TinyGPS https://github.com/mikalhart/TinyGPS
* ArduinoJson https://github.com/bblanchon/ArduinoJson

### References:
* [G3 PM2.5 感測器](http://www.icshop.com.tw/product_info.php/products_id/20460)
* [APRS气象站传感器套件](http://wiki.dfrobot.com.cn/index.php/(SKU:SEN0186)_APRS%E6%B0%94%E8%B1%A1%E7%AB%99%E4%BC%A0%E6%84%9F%E5%99%A8%E5%A5%97%E4%BB%B6)

## Raspberry Pi

### 設定開機自動執行程式

安裝 Supervisor

    # apt-get install supervisor

設定 Supervisor
```bash
# vi /etc/supervisor/supervisord.conf
[program:thingspeak]
command=/usr/bin/python /home/pi/thingspeak.py
```
