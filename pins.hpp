#ifndef PINS_H
#define PINS_H

static const uint8_t SDI = 11;
static const uint8_t SII = 12;
static const uint8_t SDO = 13;
static const uint8_t HVSP_SCL = A4;
static const uint8_t RST = 10;
static const uint8_t VCC = A3;

static const uint8_t HVMODE_LED_PIN = 5;
static const uint8_t SELECT_HV_MODE_PIN = A0;
static const uint8_t LED_HB = 9;
static const uint8_t LED_ERR = 8;
static const uint8_t LED_PMODE = 7;

struct Fuses {
  uint8_t hfuse; // high
  uint8_t lfuse; // low
  uint8_t efuse; // extended
};

#endif // PINS_H
