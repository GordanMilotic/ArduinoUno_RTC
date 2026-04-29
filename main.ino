#include <LiquidCrystal.h>

#define CLK 2
#define DAT 3
#define CE  4

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void ds_writeByte(uint8_t val) {
  pinMode(DAT, OUTPUT);
  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK, LOW);
    digitalWrite(DAT, val & 0x01);
    delayMicroseconds(1);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(1);
    val >>= 1;
  }
}

uint8_t ds_readByte() {
  pinMode(DAT, INPUT);
  uint8_t val = 0;
  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK, LOW);
    delayMicroseconds(1);
    if (digitalRead(DAT)) val |= (1 << i);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(1);
  }
  digitalWrite(CLK, LOW);
  return val;
}

void ds_write(uint8_t reg, uint8_t val) {
  digitalWrite(CLK, LOW);
  digitalWrite(CE, LOW);
  delayMicroseconds(4);
  digitalWrite(CE, HIGH);
  delayMicroseconds(1);
  ds_writeByte(reg);
  ds_writeByte(val);
  digitalWrite(CE, LOW);
}

uint8_t ds_read(uint8_t reg) {
  digitalWrite(CLK, LOW);
  digitalWrite(CE, LOW);
  delayMicroseconds(4);
  digitalWrite(CE, HIGH);
  delayMicroseconds(1);
  ds_writeByte(reg);
  uint8_t val = ds_readByte();
  digitalWrite(CE, LOW);
  return val;
}

uint8_t bcd2dec(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }
uint8_t dec2bcd(uint8_t d) { return ((d / 10) << 4) | (d % 10); }

void ds_burstWrite() {
  digitalWrite(CLK, LOW);
  digitalWrite(CE, LOW);
  delayMicroseconds(4);
  digitalWrite(CE, HIGH);
  delayMicroseconds(1);

  ds_writeByte(0xBE);        //burst wr
  ds_writeByte(dec2bcd(0));   //sekunde
  ds_writeByte(dec2bcd(17));  //minute
  ds_writeByte(dec2bcd(14)); //sati
  ds_writeByte(dec2bcd(29)); //dan
  ds_writeByte(dec2bcd(4));  //mjesec
  ds_writeByte(0x04);        //dan u tjednu
  ds_writeByte(dec2bcd(26)); //godina
  ds_writeByte(0x00);        

  digitalWrite(CE, LOW);
}

void ds_init() {
  pinMode(CLK, OUTPUT);
  pinMode(CE, OUTPUT);
  pinMode(DAT, OUTPUT);
  digitalWrite(CE, LOW);
  digitalWrite(CLK, LOW);
  delay(10);
  ds_write(0x8E, 0x00);
  delay(5);
}

void setup() {
  Serial.begin(9600);
  lcd.begin(8, 2);
  ds_init();
  delay(10);
  ds_burstWrite(); // kom
  delay(100);
}

void loop() {
  uint8_t s  = bcd2dec(ds_read(0x81) & 0x7F);
  uint8_t m  = bcd2dec(ds_read(0x83));
  uint8_t h  = bcd2dec(ds_read(0x85));
  uint8_t d  = bcd2dec(ds_read(0x87));
  uint8_t mo = bcd2dec(ds_read(0x89));
  
  //printanje na lcd
  lcd.setCursor(0, 0);
  if (h < 10) lcd.print("0"); lcd.print(h);
  lcd.print(":");
  if (m < 10) lcd.print("0"); lcd.print(m);
  lcd.print(":");
  if (s < 10) lcd.print("0"); lcd.print(s);

  lcd.setCursor(0, 1);
  if (d < 10) lcd.print("0"); lcd.print(d);
  lcd.print(".");
  if (mo < 10) lcd.print("0"); lcd.print(mo);
  lcd.print(".");
  uint8_t y = bcd2dec(ds_read(0x8D));
  if (y < 10) lcd.print("0"); lcd.print(y);

  delay(1000);
}