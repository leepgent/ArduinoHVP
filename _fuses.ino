//Function to set fuses
//See datasheet and http://www.rickety.us/2010/03/arduino-avr-high-voltage-serial-programmer/

#define SIG_BYTE_COUNT (3)
#define DEVICE_SIG_TINY85 (0x930B)
#define DEVICE_SIG_TINY13A (0x9007) // 0x90 -> 1kb flash; x07 -> attiny13A
#include "pins.hpp"

struct DeviceFuseInfo {
  uint16_t    device_sig;
  const char* device_name;
  uint8_t     lfuse_defaults; // Remember a set bit means 'unprogrammed' or off
  uint8_t     hfuse_defaults;
  uint8_t     hfuse_rstdisable_bit_num;
  uint8_t     has_extended_fuse;
  uint8_t     efuse_defaults;
};

static const DeviceFuseInfo DEVICES[] = {
  {
    .device_sig = DEVICE_SIG_TINY13A,
    .device_name  = "ATTiny13A",
    .lfuse_defaults = 0x6A, // SPIEN | CKDIV8 | SUT0 | CKSEL0 -> (int. RC osc @ 9.6MHz / 8)
    .hfuse_defaults = 0xFF,
    .hfuse_rstdisable_bit_num = 0,
    .has_extended_fuse = 0
  },
  {
    .device_sig = DEVICE_SIG_TINY85, // CF ATTINY85 Datasheet pp. 148
    .device_name  = "ATTiny85",
    .lfuse_defaults = 0x62, // CKDIV8 | SUT0 | CKSEL0 | CKSEL2 | CKSEL3 -> (int. RC osc @ 9.6MHz / 8)
    .hfuse_defaults = 0xDF, // SPIE3
    .hfuse_rstdisable_bit_num = 7,
    .has_extended_fuse = 1,
    .efuse_defaults = 0xff
  }
};
static const uint8_t DEVICES_COUNT = sizeof(DEVICES)/sizeof(DeviceFuseInfo);

static const struct DeviceFuseInfo* find_device(uint16_t deviceSig) {
  for (uint8_t i = 0; i < DEVICES_COUNT; ++i) {
    const DeviceFuseInfo* info = DEVICES + i;
    if (info->device_sig == deviceSig) return info;
  }
  return nullptr;
}


// Omits the first byte, which should always be 0x1E (which indicates an Atmel/Microchip device)
static uint16_t readSignature() {
  uint16_t sig = 0;
  byte val;
  for (int ii = 1; ii < 3; ii++) {
          writeHV(0x08, 0x4C);
          writeHV(  ii, 0x0C);
          writeHV(0x00, 0x68);
    val = writeHV(0x00, 0x6C);
    sig = (sig << 8) + val;
  }
  return sig;
}

void burnFuses(){
  const DeviceFuseInfo* device;

  if (!pmode) start_pmode();
  uint16_t sig = readSignature();
  device = find_device(sig);
  end_pmode();

  if (nullptr == device) {
    Serial.printf(F("Unknown device signature %4x! Quitting\n"), sig);
    for (;;)
    ;
  }
  Serial.printf(F("Device is %s (sig %04x)\n"), device->device_name, sig);
  //byte hfuse = device->hfuse_defaults;
  //byte lfuse = device->lfuse_defaults;
  Serial.printf(F("Default lfuse: %02X, hfuse: %02X\n"), device->lfuse_defaults, device->hfuse_defaults);

  if (!pmode) start_pmode();
  Fuses currentFuses = readFusesHV();
  end_pmode();

  Serial.println(F("Please select new fuse settings:"));
  Serial.println(F("0 - Default\n1 - Disable Reset\n2 - 8MHz Clock\n3 - Disable Reset, 8MHz Clock"));
  Serial.println(F("4 - EEPROM Preserve\n5 - EEPROM Preserve,Disable Reset\n6 - EEPROM Preserve,8MHz Clock\n7 - EEPROM Preserve,Disable Reset, 8MHz Clock"));
  Serial.println(F("9 - Read Only"));
  while (Serial.available()) Serial.read();
  while (!Serial.available()){};
  byte thisChar = Serial.read();
  Serial.println(F("Entering Programming Mode..."));
  if (!pmode) start_pmode();

  switch (thisChar) {
    case '9': // Read flash
      here = 0;
      for (byte i = 0; i < 32; i++){
        Serial.print("0x");
        writeHV(0b00000010, 0b01001100);  //Load "Read Flash" Command
        pmode = 3;
        Serial.println(flash_read(here), HEX);
        here++;
      }

    break;
    case '0': // set fuses to default
        //currentFuses.hfuse = device->hfuse_defaults;
       // currentFuses.lfuse = device->lfuse_defaults;
      Serial.printf(F("Setting lfuse: %4x, hfuse: %4x\n"), device->lfuse_defaults, device->hfuse_defaults);

      writeFusesHV(device->lfuse_defaults, device->hfuse_defaults);
      readFusesHV();
    break;

    default: // tweak existing fuses
    

    if (thisChar & 0b00000001) {
      // Set RESET DISABLE -> program appr. bit -> set to 0
      currentFuses.hfuse &= ~(1<<device->hfuse_rstdisable_bit_num);
    }
    if (thisChar & 0b00000010) {
      // set clock to int 8mhz osc
      currentFuses.lfuse |= 0b10000000; // TODO: NOT YET SUPPORTED FOR ATTINY13
    }
    if (thisChar & 0b00000100)
    {
      // preserve eeprom
      currentFuses.hfuse &= 0b11110111; // TODO: NOT YET SUPPORTED FOR ATTINY13
    }
    Serial.printf(F("Setting lfuse: %4x, hfuse: %4x\n"), currentFuses.lfuse, currentFuses.hfuse);

    writeFusesHV(currentFuses.lfuse, currentFuses.hfuse);
    readFusesHV();
    break;
  
  }
  
  Serial.println("Exiting Programming Mode");
  end_pmode();
  Serial.println("FINISHED");
}

void writeFusesHV(byte _l, byte _h){
  Serial.print("Writing hfuse = ");
  Serial.println(_h, BIN);
  writeHV(0x40, 0x4C);
  writeHV(_h, 0x2C);
  writeHV(0x00, 0x74);
  writeHV(0x00, 0x7C);
  //Write lfuse
  Serial.print("Writing lfuse = ");
  Serial.println(_l, BIN);
  writeHV(0x40, 0x4C);
  writeHV(_l, 0x2C);
  writeHV(0x00, 0x64);
  writeHV(0x00, 0x6C);
  Serial.println(); 
}


Fuses readFusesHV(){
  Fuses f;
  writeHV(0x04, 0x4C);
  writeHV(0x00, 0x7A);
  byte inData = writeHV(0x00, 0x7E);
  Serial.print(F("hfuse = "));
  Serial.print(inData, BIN);
  Serial.print(F(" - "));
  Serial.println(inData, HEX);
  f.hfuse = inData;
  
  writeHV(0x04, 0x4C);
  writeHV(0x00, 0x68);
  inData = writeHV(0x00, 0x6C);
  Serial.print(F("lfuse = "));
  Serial.print(inData, BIN);
Serial.print(F(" - "));
  Serial.println(inData, HEX);
  f.lfuse = inData;
  
  //Read efuse
  writeHV(0x04, 0x4C);
  writeHV(0x00, 0x6A);
  inData = writeHV(0x00, 0x6E);
  Serial.print(F("efuse = "));
  Serial.print(inData, BIN);
    Serial.print(F(" - "));
  Serial.println(inData, HEX);
  f.efuse = inData;

  Serial.println(); 
  return f;
}
