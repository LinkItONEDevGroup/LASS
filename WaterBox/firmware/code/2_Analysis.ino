// 這邊放分析水質的code(pH EC Temp)
/*
   計算array平均(超過5筆資料就會自動替除最大及最小值)
*/
double averge_array(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;

  if (number <= 0)
  {
    SerialUSB.println("Error number for the array to avraging!/n");
    return 0;
  }

  if (number < 5) { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  }
  else {            // 大於5個資料時的處理方式

    if (arr[0] < arr[1]) {        // 確認前一筆的資料比較小
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    } // end of if

    // 過濾最大級最小的兩筆資料，不納入計算
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {   // 低於下限值以下限值計算，並更新下限值
        amount += min;      // arr < min
        min = arr[i];
      } else {
        if (arr[i] > max) { // 高於上限值以上現值計算，並更新上限值
          amount += max;    // arr > max
          max = arr[i];
        } else {            // 未達上下現值，直接納入計算
          amount += arr[i]; // min<=arr<=max
        }
      } //if
    } //for

    avg = (double)amount / (number - 2);

  }//if

  return avg;
}

/*
   讀取pH 資料
*/
float _pH_mV(float slope = 1.0, float intercept = 0.0)
{
  int _sample_times = 40;
  int _dataArray[_sample_times];
  long _time = millis();        // 初始化採樣時間
  long _t = millis();           // 計時用
  int _i = 0;

  // 在800ms內，每20ms連續取樣放到array中
  while (_i < _sample_times) {
    if (millis() - _time > 20) {
      _dataArray[_i] = analogRead(pH_pin);
      _time += 20;  // 更新下一次採樣時間點
      _i++;
    }
    delay(10);
  }

  // 只有pH要刪除最大及最小值

  float _value = averge_array(_dataArray, _sample_times); // 把analogRead獨到的值取平均
  _value = 2 * map(_value, 0, ADC_range - 1, 0, ADC_voltage); // 還原成真正的訊號輸出電壓

  // 手動線性校正
  _value = _value * slope + intercept;

  _t = millis() - _t;   // 結算分析時間
  delay(1000 - _t);
  //  SerialUSB.println("pH分析時間 >> " + (String)_t + "(ms)");
  return _value;
}

/*
   讀取EC 資料
*/
int EC_value(float temperature = 25.0, float slope = 1.0, float intercept = 0.0)
{
  float _value;
  long _value_total = 0;
  long _sample_time = millis(); // 初始化採樣時間
  long _t = millis();           // 計時用

  // 在800ms內，每50ms連續取樣放到array中
  for (int _i = 0; _i < 16; _i++) {
    _value_total += analogRead(EC_pin);
    delay(50);
  }

  float _value_average = _value_total / 16;                                          // 求出平均電壓，輸出電壓(5V)為ADC電壓(2.5V)的兩倍
  float _average_voltage = 2 * map(_value_average, 0, ADC_range - 1, 0, ADC_voltage);  // 還原成真正的訊號輸出電壓

  // 溫度補償係數
  float _temp_coefficient = 1.0 + 0.0185 * (temperature - 25.0);      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
  float _coefficient_volatge = _average_voltage / _temp_coefficient;  // 電壓係數

  // 三個不同區間的導電度換算
  if (_coefficient_volatge < 150)       {
    SerialUSB.print("No solution!(");
    SerialUSB.print(_average_voltage);
    SerialUSB.println(")");
    _value = 6.84 * _coefficient_volatge - 200; // EC <= 1ms/cm (暫定)
    if (_value < 0) _value = 0;
  }
  else if (_coefficient_volatge > 3300) {
    SerialUSB.println("Out of the range!");
    _value = 5.3 * _coefficient_volatge + 2278; // 20ms/cm<EC (暫定)
  }
  else
  {
    if (_coefficient_volatge <= 448)        _value = 6.84 * _coefficient_volatge - 64.32; // 1ms/cm<EC<=3ms/cm
    else if (_coefficient_volatge <= 1457)  _value = 6.98 * _coefficient_volatge - 127;   // 3ms/cm<EC<=10ms/cm
    else                                 _value = 5.3 * _coefficient_volatge + 2278;   // 10ms/cm<EC<20ms/cm
  }

  // 手動線性校正
  _value = _value * slope + intercept;

  _t = millis() - _t;   // 結算分析時間
  delay(1000 - _t);
  //  SerialUSB.println("EC分析時間 >> " + (String)_t + "(ms)");


  return _value;
}

/*
   TempProcess(bool ch)
   ch 為 true時讀取溫度，反之檢查溫模組並進入準備狀態
*/
float TempProcess(bool ch)
{
  //returns the temperature from one DS18B20 in DEG Celsius
  static byte data[12];
  static byte addr[8];
  static float TemperatureSum;
  if (!ch) {
    if ( !ds.search(addr)) {
      SerialUSB.println("no more sensors on chain, reset search!");
      ds.reset_search();
      return 0;
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      SerialUSB.println("CRC is not valid!");
      return 0;
    }
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
      SerialUSB.print("Device is not recognized!");
      return 0;
    }
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1); // start conversion, with parasite power on at the end
  }
  else {
    byte present = ds.reset();
    ds.select(addr);
    ds.write(0xBE); // Read Scratchpad
    for (int i = 0; i < 9; i++) { // we need 9 bytes
      data[i] = ds.read();
    }
    ds.reset_search();
    byte MSB = data[1];
    byte LSB = data[0];
    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    // 原本OneWire函式庫中有另外進行位數的檢查，這邊忽略

    TemperatureSum = tempRead / 16; // 攝氏溫度的計算
  }
  return TemperatureSum;
}

/*
   讀取溫度資料
  ch=0,let the DS18B20 start the convert;ch=1,MCU read the current temperature from the DS18B20.
*/
float temp_value() {

  float _temp  = TempProcess(true);  // read the current temperature from the  DS18B20
  TempProcess(false);                // after the reading,start the convert for next reading
  // 讓DS18B20進入準備狀態(檢查模組)
  return _temp;
}

/*
  把特定的ADC腳位電壓，換算成0-val的數值後返回float格式
  rotary_button(指定的ADC pin, 要轉換的數值)
*/
float _analog_convert(uint8_t pin, int _val)
{
  int  _read = analogRead(pin);
  float _value = map(_read, ADC_offset, r_ADC_range - 1, 0, _val);
  return _value;
}

/*
   計算某pin上拉一次花多少秒(0.1)
   並設定上限秒數超過自動跳出回傳
*/
float pull_time(uint8_t pin, int _limit) {
  bool _virutal_button = false;             // 紀錄按鈕狀態用
  int _while_i = 0;                         // 中斷while用
  unsigned long _push_time = 0;             // 按下時的時間(暫存)
  float _duration_time = 0;                 // 總經歷秒數
  bool _btn_state = digitalRead(pin);       // 確認按鈕有沒有被按下

  for (int _i = 0 ; _i < 2; _i++) {         //  跑兩遍，第一遍用來確認按鈕狀態，第二遍用來結算時間
    if (_btn_state) {                       // 按鈕被按下的話，確認虛擬按鈕的狀態
      _virutal_button = true;               // 按下虛擬按鈕
      _push_time = millis();                // 紀錄按下開始時間(millisecond)

      while (_btn_state) {                                                 // 當按鈕被按下時進入while迴圈，超過_limit秒數(*10ms)後自動中斷
        if (_while_i < _limit * 100)  _btn_state = digitalRead(pin);        // 用while停住程式，並持續更新按鈕狀態
        else {
          _btn_state = false;                                              // 跳出前先把按鈕狀態關掉，避免再次進入while
        }
        delay(10);
        _while_i++;
      } // end of while (按鈕確認)

    }
    else {                           // 按鈕彈起時，結算按住時間
      if (_virutal_button) {         // 如果是按鈕彈起後還沒更新虛擬按鈕，結算時間
        _duration_time = (millis() - _push_time) * 0.001;
      }
    }
  }
  return _duration_time;
}
