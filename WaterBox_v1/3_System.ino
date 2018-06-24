// 缺Alarm, LoRa, MQTT, 列出所有page

/*
   格式轉換(int變成2位數的String)
*/
String convert_2digits(int i)
{
  String number;
  if (i < 10) number = "0" + (String)i;
  else number = (String)i;
  return number;
}

/*
   I/O 的部份
*/
// SD卡
void save2SD(int Y, int M, int D, int h, int m, float t, float ph, float ec, float pH_mV)
{

  String file = String(Y) + convert_2digits(M) + convert_2digits(D) + ".csv";
  String Header = "Date,Time,Tempture(C),pH,EC,pH_mV";
  String data = String(Y) + "/" + convert_2digits(M) + "/" + convert_2digits(D) + "," + \
                convert_2digits(h) + ":" + convert_2digits(m) + "," + \
                String(t) + "," + String(ph) + "," + String(ec)  + String(pH_mV);

  SerialUSB.println("寫入SD卡中");

  if (SD.exists(file)) {
    File myFile = SD.open(file, FILE_WRITE);   // open file
    if (myFile) {
      myFile.println(data);
      myFile.close();
    }
    else {
      SerialUSB.println("SD卡檔案開啟錯誤 " + file);
    }
  } else {
    File myFile = SD.open(file, FILE_WRITE);   // open a new file and add the table header:
    if (myFile) {
      myFile.println(Header);
      myFile.println(data);
      myFile.close();
    }
    else {
      SerialUSB.println("SD卡檔案建立錯誤 " + file);
    }
  }
}


/*
   EEPROM 存取設定資料
*/

bool EEPROM_write( int deviceaddress, char* data, int _page, int _length)
{
  if (_length > 30) {
    SerialUSB.println("Too many bites of data, writing fail....");
    return false;
  }
  else {
    int address = _page * 32;
    SerialUSB.println("writing " + (String)_length + " data at address " + (String)address + " .....");
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(address >> 8)); // MSB
    Wire.write((int)(address & 0xFF)); // LSB
    for ( int c = 0; c < _length; c++)
      Wire.write(data[c]);
    Wire.endTransmission();
    return true;
    SerialUSB.println("DONE..");
  }
}

// maybe let's not read more than 30 or 32 bytes at a time!
String EEPROM_read( int deviceaddress, int _page, int _length)
{
  int address = _page * 32;
  char str;
  String read_buffer = "";

  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(address >> 8)); // MSB
  Wire.write((int)(address & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress, _length);

  for ( int c = 0; c < _length; c++ ) {
    if (Wire.available()) {
      str = Wire.read();
      read_buffer += (char)str;
    }
  }
  return read_buffer;
}

void sigfox_atcommand_rx()                                       // sigfox only
{
  char temp;
  while (Serial1.available() <= 0);
  while (Serial1.available() > 0)
  {
    temp = Serial1.read();
    SerialUSB.print(temp);
    delay(10);
  }
}

void sigfox_atcommand_tx(char *wBuffer)          // sigfox only
{
  Serial1.print(wBuffer);                        // of, just Send Data to backend
  sigfox_atcommand_rx();                                       // Get feedback and show on Terminal function
}

void send_to_sigfox(String _str) {
  String _buffer_str = "AT$SF=" + _str;
  char _buffer[_str.length()+2];
  _buffer_str.toCharArray(_buffer, _buffer_str.length()+2);
  
  SerialUSB.println(_buffer_str);
  SerialUSB.println(_buffer);

  sigfox_atcommand_tx("AT$RCZ?\r");          //sigfox AT Command: get RCZ
  sigfox_atcommand_tx("AT$ID?\r");           //sigfox AT Command: Get ID
  sigfox_atcommand_tx("AT$PAC?\r");          //sigfox AT Command: Get PAC
  delay(100);
  sigfox_atcommand_tx(_buffer);
  sigfox_atcommand_tx("\r");                // Send Data to backend function

}

/*
   把bit資料印出來確認
*/
void _printOut1(long long c, int _size) {
  int _bits = _size * 4 - 1;
  for (_bits; _bits > -1; _bits--) {
    if (c & (1 << _bits)) {
      SerialUSB.print ("1");
    }
    else {
      SerialUSB.print ("0");
    }
  }
  SerialUSB.println();
}




/*
   封包打包
   第1個變數：要打包的數據(int)
   第2個變數：數據特徵的array
   第3個變數：封包的array
   第4個變數：封包長度的array
*/
void _bale(int _value, int *item, long long *_package, int*_size) {
  if (_size[0] + item[1] <= 32)    // 確認buffer 1 還有空間
  {
    _size[0] += item[1];
    _package[0] = _package[0] << item[1];
    _package[0] |= _value;
    SerialUSB.println("寫入第一段");
  }
  else {
    _size[1] += item[1];
    _package[1] = _package[1] << item[1];
    _package[1] |= _value;
    SerialUSB.println("寫入第二段");
  }
}

/*
   把轉換成String
*/
String _bale2Hex(long long *_package) {
  String _str;

  for (int _i = 0; _i < 2; _i++) {
    int _to_long = _package[_i];
    String _HEX_str = String(_to_long, HEX);
     
    int _j = _HEX_str.length();                       // 取得目前封包的大小(bytes)
    for (_j; _j < 8; _j++) _HEX_str = "0" + _HEX_str; // 剩餘空間補0
    SerialUSB.println(_HEX_str);
    _str += _HEX_str;
    delay(100);
  }
  
  SerialUSB.println(_str);
  return _str;
}

/*
   設定存取用
   _state 為false時，寫入設定；反之讀取設定
*/

void _config(bool _state = true) {
  String _buffer;
  char _char_buffer[30];

  if (_state) SerialUSB.println("讀取設定....");
  else        SerialUSB.println("開始更新設定");


  for (int _i = 0; _i < 6; _i++) {
    if (_state) {
      _buffer = EEPROM_read( EEPROM_deviceaddress, _i, 10);
      SerialUSB.print(_buffer);
    }

    switch (_i) {
      case 0:
        if (_state) pH_slop = _buffer.toFloat();
        else _buffer = (String)pH_slop;
        break;
      case 1:
        if (_state) pH_intercept = _buffer.toFloat();
        else       _buffer = (String)pH_intercept;
        break;
      case 2:
        if (_state) EC_slop = _buffer.toFloat();
        else       _buffer = (String)EC_slop;
        break;
      case 3:
        if (_state) EC_intercept = _buffer.toFloat();
        else       _buffer = (String)EC_intercept;
        break;
      case 4:
        if (_state) pH_alarm = _buffer.toFloat();
        else       _buffer = (String)pH_alarm;
        break;
      case 5:
        if (_state) EC_alarm = _buffer.toFloat();
        else       _buffer = (String)EC_alarm;
        break;
    }// end of switch

    if (!_state) {
      _buffer.toCharArray(_char_buffer, _buffer.length() + 1);

      SerialUSB.print("寫入資料");
      SerialUSB.println(_char_buffer);
      EEPROM_write(EEPROM_deviceaddress, _char_buffer, _i, _buffer.length() + 1);
      delay(100);
    }
    delay(100);
  }
  SerialUSB.println("設定完成");
}


/*
   異常警報設定(TODO)
*/
bool alarm_check(float pH, int EC)
{
  bool _pH_alarm = false, _EC_alarm = false;
  if (pH < 7 - pH_alarm || pH > 7 + pH_alarm)  _pH_alarm = true;
  if (EC > EC_alarm)  _EC_alarm = true;
  return _pH_alarm || _EC_alarm;
}

