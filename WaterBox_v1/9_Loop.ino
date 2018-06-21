/*
   usr 按鈕檢查
   超過5秒返回 true : 儲存資料
   否則返回false:     狀態 + 1
*/
int chapter = 0;                          //  大項目：0~1(校正設定,時間設定)
int item = 0;                             //  子項目：0~3(準備,參數1設定,參數2設定,參數3設定)
bool config_state = true;                 //  true時只能顯示目前設定，flase時可以改設定
int _YY, _year_1, _year_2, _MM, _DD, _HH, _mm; //  時間日期調整用

void loop() {
  bool _switch_mode = digitalRead(switch_pin);

  DateTime now = rtc.now();          // 取得目前時間
  int _year = now.year();
  int _month = now.month();
  int _day = now.day();
  String _days = daysOfTheWeek[now.dayOfTheWeek()];
  int _hour = now.hour();
  int _minute = now.minute();
  bool _SD_save = now.unixtime() % SD_save_interval == 0;     //  5分鐘存檔一次
  bool _upload = now.unixtime() % Upload_interval == 0;       // 20分鐘上傳一次
  // Upload_interval

  if (_switch_mode) // 分析模式
  {
    SerialUSB.println("分析模式");

    float temperature = temp_value();
    int EC = EC_value(temperature, EC_slop, EC_intercept);
    float pH_mV = _pH_mV(pH_slop, pH_intercept);                             // 實際輸出電壓(mV)，校正用來校正電壓
    float pH_Value = 3.5 * pH_mV * 0.001;                                   // 分壓(5V->2.5V)後，用電壓換算成pH (斜率3.5的出處待確認)
    float DO, ORP;                                                        // 預留但不會用到

    OLED_header("  >> Water Box <<  ");
    OLED_content((String)_year + "/" + convert_2digits(_month) + "/" + convert_2digits(_day), 1);
    OLED_content("<< " + _days + " >>", 1);
    OLED_content("T: " + (String)temperature, 2);
    display.display();
    display.clearDisplay();
    delay(1000);

    OLED_header("  >> Water Box <<  ");
    OLED_content(convert_2digits(_hour) + ":" + convert_2digits(_minute), 1);
    OLED_content("EC: " + (String)EC + " us", 2);
    OLED_content("pH: " + (String)pH_Value, 2);
    OLED_content("pH(mV): " + (String)pH_mV, 1);
    display.display();
    display.clearDisplay();
    delay(500);

    String _Msg = "";

    if (_SD_save)  {
      OLED_header("  >> Water Box <<  ");
      OLED_content(convert_2digits(_hour) + ":" + convert_2digits(_minute), 1);
      OLED_content("Save Data", 2);
      display.display();
      display.clearDisplay();
      delay(500);
      save2SD(_year, _month, _day, _hour, _minute, temperature, pH_Value, EC, pH_mV);
      delay(500);
    }
    else delay(1000);

    if (_upload) {                 // 上傳前先用JSON打包訊息
      SerialUSB.print("上傳資料");

      /*
         int App_ID = 0x6;                       // App ID 給LASS後端辨識用
        int Temperature_filed[2] = {1,7};         // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
        int pH_filed[2] = {1,8};                 // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
        int EC_filed[2] = {1,18};                // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
        int DO_filed[2] = {0,7};                 // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
        int ORP_filed[2] = {0,11};               // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
        const int Filed_length_limit = 52;             // Filed length limit (在設定時檢查是否超過長度，16*4(sigfox) - 4(app id)- 8(FiledDefinition) =52)
      */

      long long _buffer[2] = {App_ID, 0};  // 打包資料用buffer  (第一個)
      int _packets_size[2] = {12, 0};      // 封包大小初始值為12bits，後面再依測項增加長度

      byte _DataFiled = 0b1;             // byte 為8 bits剛好可以塞_DataFiled
      _DataFiled = _DataFiled << 1;
      _DataFiled |= Temperature_filed[0];// add Temperature filed
      _DataFiled = _DataFiled << 1;
      _DataFiled |= pH_filed[0];        // add pH filed
      _DataFiled = _DataFiled << 1;
      _DataFiled |= EC_filed[0];        // add EC filed
      _DataFiled = _DataFiled << 1;
      _DataFiled |= DO_filed[0];        // add DO filed
      _DataFiled = _DataFiled << 1;
      _DataFiled |= ORP_filed[0];       // add ORP filed
      _DataFiled = _DataFiled << 2;     // add last 2 reserved filed

      SerialUSB.println("_DataFiled: " + (String)_DataFiled);   // 確認打包OK
      SerialUSB.println(_DataFiled, BIN);

      _buffer[0] = _buffer[0] << 8;
      _buffer[0] |= _DataFiled;                // add Data Filed

      SerialUSB.print("第1段封包");
      _printOut1(_buffer[0], sizeof(_buffer[0]));  // 0000 0000 0000 0000 0000 0110 1111 0000
      SerialUSB.print("第2段封包");
      _printOut1(_buffer[1], sizeof(_buffer[1]));  // 0000 0000 0000 0000 0000 0000 0000 0000

      if (Temperature_filed[0]) {
        int _temperature_temp = int(temperature) + 200;                     // -20.0~80.0 修正成 0-100
        _bale(_temperature_temp, Temperature_filed, _buffer, _packets_size);   // 打包資料
        SerialUSB.print("第1段封包");
        _printOut1(_buffer[0], sizeof(_buffer[0]));  
        // 0000 0000 0000 0110 1111 0000 1110 0110
        // 0000 0000 0000 0110 1111 0000 1110 0110 
        // 0110 1111 0000 1110 0110 0010 1111 0000
      } else SerialUSB.println("無安裝溫度模組");
      if (pH_filed[0]) {
        int _pH_temp = int(pH_Value * 10);                                     // -20.0~80.0 修正成 0-100
        _bale(_pH_temp, pH_filed, _buffer, _packets_size);                  // 打包資料
      } else SerialUSB.println("無安裝pH模組");
      if (EC_filed[0]) {
        int _EC_temp = int(EC);                                          // -20.0~80.0 修正成 0-100
        _bale(_EC_temp, EC_filed, _buffer, _packets_size);                  // 打包資料
      } else SerialUSB.println("無安裝EC模組");
      if (DO_filed[0]) {
        int _DO_temp = int(DO * 10);                                      // 0.0~10.0 修正成 0-100
        _bale(_DO_temp, DO_filed, _buffer, _packets_size);                   // 打包資料
      } else SerialUSB.println("無安裝DO模組");
      if (ORP_filed[0]) {
        int _ORP_temp = int(ORP) + 1000;                                  // -20.0~80.0 修正成 0-100
        _bale(_ORP_temp, ORP_filed, _buffer, _packets_size);                // 打包資料
      } else SerialUSB.println("無安裝ORP模組");

      _buffer[0] = _buffer[0] << ( 32 - _packets_size[0] );                 // 把資料往左補滿，空格再右
      _buffer[1] = _buffer[1] << ( 32 - _packets_size[1] );                 // 把資料往左補滿，空格再右

      SerialUSB.print("第1段封包(" + (String)(32 - _packets_size[0]) + ")\t");
      _printOut1(_buffer[0], sizeof(_buffer[0]));                           // 0000 0000 0000 0011 0111 1000 1110 1000
      SerialUSB.print("第2段封包(" + (String)(32 - _packets_size[1]) + ")\t");
      _printOut1(_buffer[1], sizeof(_buffer[1]));                           // 0000 0000 0000 0000 0000 0000 0000 0000

      String _str_balle = _bale2Hex(_buffer);
      send_to_sigfox(_str_balle);
    }
    SerialUSB.println((String)_year + "/" + convert_2digits(_month) + "/" + convert_2digits(_day) + "\t<<" + _days + ">>");
    SerialUSB.print("pH: ");
    SerialUSB.print(pH_Value);
    SerialUSB.print("(");
    SerialUSB.print(pH_mV);
    SerialUSB.print(")");
    SerialUSB.print("\tEC: ");
    SerialUSB.print(EC);
    SerialUSB.print("\t溫度: ");
    SerialUSB.print(temperature);
    SerialUSB.println("\n\r");

  } // end of if (_switch_mode)
  else {  // 設定模式
    unsigned long printTime = millis();                            // Serial印出時間計時器
    float _time = pull_time(USR_pin, 3);                           // 設定按鈕計時器
    String _state_str, _config_str;                                // Serial輸出用
    String _state_OLDE_1, _state_OLDE_2, _state_str_OLED;          // OLED 顯示用

    if (printTime % print_interval <= print_interval * 0.05) {
      SerialUSB.println("設定模式");
    }

    if (_time > 0 && _time < 1) {       // 切換子項目
      if (item == 3) item = 0;          // 到最後一項時跳回第一項
      else item += 1;                   // 跳到下一項
    }
    else if (_time > 1 && _time < 2) {  // 切換大項目
      if (chapter == 1) chapter = 0;    // 到最後一項時跳回第一項
      else chapter += 1;                // 跳到下一項
    }
    else if (_time >= 3) {              // 切換設定顯示/調整
      if (config_state) config_state = false;                          // 切換到設定調整狀態
      else  {                                                          // 切回設定顯示狀態
        config_state = true;
        _config(false);                                                // 儲存目前的設定
        if (_YY != 0 && _MM != 0 && _DD != 0 && _HH != 0 && _mm != 0)  rtc.adjust(DateTime(_YY, _MM, _DD, _HH, _mm, 0)); // 設定時間
      }
    }
    if (printTime % print_interval <= print_interval * 0.05) {
      if (config_state) {
        SerialUSB.println("顯示目前設定");
      }
      else {
        SerialUSB.println("調整設定");
      }
    }
    // 顯示目前的項目
    switch (item) {
      case 0:
        if (chapter) {
          _state_str = "時間設定頁面";
          _state_str_OLED = "Setting Time";
        }
        else {
          _state_str = "參數設定頁面";
          _state_str_OLED = "Setting Parameters";
        }
        break;
      case 1:
        if (chapter) {          // 年份的設定
          if (config_state) {   // 顯示年分
            _state_str = "系統年分";
            _config_str = (String) _year;
            _state_str_OLED = "This Year";
            _state_OLDE_1 = (String)_year;
          }
          else {                // 用可變電阻調整目前年分
            _state_str = "設定年分";
            int _year_1 = (int)_analog_convert(Rotary_Pin_1, 9);
            int _year_2 = (int)_analog_convert(Rotary_Pin_2, 9);
            _YY = 2000 + _year_1 * 10 + _year_2;
            _config_str = (String) _YY;
            _state_str_OLED = "Setting Year";
            _state_OLDE_1 = (String)_YY;
          }
        }
        else {                  // pH斜率截距的設定
          if (config_state) {   // 顯示目前設定
            _state_str = "pH 目前的斜率截距";
            _config_str = "pH slope:" + (String)pH_slop + "\t pH intercept：" + (String)pH_intercept;
            _state_str_OLED = "pH's config";
            _state_OLDE_1 = "pH's slop: " + (String)pH_slop;
            _state_OLDE_2 = "pH's intercept: " + (String)pH_intercept;
          }
          else {                // 用可變電阻設定參數
            _state_str = "設定pH斜率截距";
            pH_slop = 0.8 + _analog_convert(Rotary_Pin_1, 400) * 0.001;
            pH_intercept = -0.5 + _analog_convert(Rotary_Pin_2, 100) * 0.01;
            _config_str = "pH slope:" + (String)pH_slop + "\t pH intercept：" + (String)pH_intercept;
            _state_str_OLED = "Setting pH's config";
            _state_OLDE_1 = "pH's slop: " + (String)pH_slop;
            _state_OLDE_2 = "pH's intercept: " + (String)pH_intercept;
          }
        }
        break;
      case 2:
        if (chapter) {          // 日期的設定
          if (config_state) {   // 顯示目前日期
            _state_str = "日期顯示";
            _config_str = convert_2digits(_month) + "/" + convert_2digits(_day);
            _state_str_OLED = "Date Now";
            _state_OLDE_1 = "Month: " + convert_2digits(_month);
            _state_OLDE_2 = "Day: " + convert_2digits(_day);
          }
          else {                // 用可變電阻調整日期
            _state_str = "日期設定(MM/DD)";
            _MM = (int)_analog_convert(Rotary_Pin_1, 11) + 1;
            _DD = (int)_analog_convert(Rotary_Pin_2, 30) + 1;
            _config_str = convert_2digits(_MM) + "/" + convert_2digits(_DD);
            _state_str_OLED = "Setting Date";
            _state_OLDE_1 = "Month: " + convert_2digits(_MM);
            _state_OLDE_2 = "Day: " + convert_2digits(_DD);
          }
        }
        else {                  // EC斜率截距的設定
          if (config_state) {   // 顯示目前設定
            _state_str = "EC目前的斜率截距";
            _config_str = "EC斜率:" + (String)EC_slop + "\t EC截距：" + (String)EC_intercept;
            _state_str_OLED = "EC's config";
            _state_OLDE_1 = "EC's slop: " + (String)EC_slop;
            _state_OLDE_2 = "EC's intercept: " + (String)EC_intercept;
          }
          else {                // 用可變電阻設定參數
            _state_str = "設定EC斜率截距";
            EC_slop = 0.8 + _analog_convert(Rotary_Pin_1, 400) * 0.001;
            EC_intercept = _analog_convert(Rotary_Pin_2, 1000) - 500;
            _config_str = "EC斜率:" + (String)EC_slop + "\t EC截距：" + (String)EC_intercept;
            _state_str_OLED = "Setting EC's config";
            _state_OLDE_1 = "EC's slop: " + (String)EC_slop;
            _state_OLDE_2 = "EC's intercept: " + (String)EC_intercept;
          }
        }
        break;
      case 3:
        if (chapter) {          // 時間的設定
          if (config_state) {   // 顯示目前時間
            _state_str = "時間顯示";
            _config_str = convert_2digits(_hour) + ":" + convert_2digits(_minute);
            _state_str_OLED = "Time Now";
            _state_OLDE_1 = "Hours Now: " + (String)_hour;
            _state_OLDE_2 = "Minutes Now: " + (String)_minute;
          }
          else {                // 用可變電阻調整目前時間
            _state_str = "時間設定(HH:MM)";
            _HH = (int)_analog_convert(Rotary_Pin_1, 23);
            _mm = (int)_analog_convert(Rotary_Pin_2, 59) + 1;
            _config_str = convert_2digits(_HH) + ":" + convert_2digits(_mm);
            _state_str_OLED = "SettingTime(HH:MM)";
            _state_OLDE_1 = "Hours: " + (String)_HH;
            _state_OLDE_2 = "Minutes: " + (String)_mm;
          }
        }
        else {                  // 偵測警報設定
          if (config_state) {
            _state_str = "上下限顯示";
            _config_str = "pH上下限:±" + (String)pH_alarm + "\t EC上限：" + (String)EC_alarm;
            _state_str_OLED = "Show the limit";
            _state_OLDE_1 = "pH's limit: " + (String)(7 - pH_alarm) + "-" + (String)(7 + pH_alarm);
            _state_OLDE_2 = "EC's limit: " + (String)EC_alarm;
          }
          else {
            _state_str = "上下限設定";
            pH_alarm = _analog_convert(Rotary_Pin_1, 20) * 0.1;
            EC_alarm = _analog_convert(Rotary_Pin_2, 1000);
            _config_str = "pH上下限:7.00±" + (String)pH_alarm + "\t EC上限：" + (String)EC_alarm;
            _state_str_OLED = "Setting limit";
            _state_OLDE_1 = "pH's limit: " + (String)(7 - pH_alarm) + "-" + (String)(7 + pH_alarm);
            _state_OLDE_2 = "EC's limit: " + (String)EC_alarm;
          }
        }
        break;
    }

    if (printTime % print_interval <= print_interval * 0.05 ) {
      SerialUSB.println("目前頁面:" + (String)chapter + "-" + (String)item);
      SerialUSB.println("\t " + _state_str);
      SerialUSB.println("\t 目前設定：" + _config_str);
      SerialUSB.println("*******************************\n\r\n\r ");
    }

    OLED_header("  >> Water Box <<  ");
    OLED_content((String)_year + "/" + convert_2digits(_month) + "/" + convert_2digits(_day) + " " + convert_2digits(_hour) + ":" + convert_2digits(_minute), 1);
    OLED_content(_state_str_OLED, 1);
    OLED_state(_state_OLDE_1, _state_OLDE_2);
    OLED_content(_firwareVersion, 1);
    display.display();
    display.clearDisplay();
  } //end of 設定模式
}
