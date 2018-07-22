// OLED 用的程式
void OLED_header(String _str) // 顯示表頭
{
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(_str);
}

void OLED_content(String _str, int _size) {
  display.setTextSize(_size);
  display.setTextColor(WHITE);
  display.println(_str);
}

void OLED_state(String _str1, String _str2) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(_str1);
  display.println(_str2);
}
