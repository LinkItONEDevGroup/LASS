#include "mbed.h"

// I2C addresses for LCD and RGB
#define LCD_ADDRESS     (0x7c)
#define RGB_ADDRESS     (0xc4)

#define RED_REG         0x04        
#define GREEN_REG       0x03        
#define BLUE_REG        0x02        

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET 0x20

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00

// flag for entry mode
#define LCD_ENTRYLEFT 0x02

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_2LINE 0x08
#define LCD_5x10DOTS 0x04

    
class Grove_LCD_RGB_Backlight
{
public:
    
    //Contructor
    Grove_LCD_RGB_Backlight(PinName sda, PinName scl); 
    

    
    //Set RGB Color of backglight
    void setRGB(char r, char g, char b);     
    
    //Initialize device
    void init();   
    
    //Turn on display
    void displayOn();
    
    //Clear all text from display
    void clear();
    
    //Print text to the lcd screen
    void print(char *str);
    
    //Move cursor to specified location
    void locate(char col, char row);

    
private:
    
    //Send command to display
    void sendCommand(char value);
    
    //Set register value
    void setReg(char addr, char val);
    
    //MBED I2C object used to transfer data to LCD
    I2C i2c;              
    
};