// setup 部分

void setup() {
  //Switch pin 初始化
  pinMode(switch_pin, INPUT);
  pinMode(USR_pin, INPUT);
  delay(100);

  // OLED 初始化
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();
  display.clearDisplay();

  SerialUSB.begin(9600);          // Set Terminal Baudrate: 9600bps   // 序列埠輸出
  Serial1.begin(9600);             // Set sigfox Baudrate:9600bps      // 對sigfox晶片下指令
  analogReadResolution(12);        // ADC改成12bit的讀值
  digitalWrite(RF_PWEN, HIGH);     // Set RF_PWEN to High, and Sigfox module power ON

  OLED_header("  >> Water Box <<  ");
  OLED_state(" >> Serial Port Check", _firwareVersion);
  display.display();
  display.clearDisplay();
  delay(1000);

  // 確認DS18B20模組並設定為準備狀態
  TempProcess(false);

  // SD card 初始化
  SerialUSB.print("初始化 SD card...");
  if (!SD.begin(SD_CS)) {
    SerialUSB.println("初始化失敗，請檢查SD卡是否插入");
  }
  SerialUSB.println("SD卡初始化完成");
  
  OLED_header("  >> Water Box <<  ");
  OLED_state(" >> SD check", _firwareVersion);
  display.display();
  display.clearDisplay();
  delay(1000);

  _config(true); // 讀取目前的設定
  
  SerialUSB.println("系統設定初始化完成");

  OLED_header("  >> Water Box <<  ");
  OLED_state(" >> Init DONE", _firwareVersion);
  display.display();
  display.clearDisplay();
  delay(1000);
}
