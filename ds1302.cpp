#include "ds1302.h"
#include <util/delay.h>

/*
* Register names.
*
* Since the highest bit is always '1', the registers start at 0x80 If the
* register is read, the lowest bit should be '1'.
*/
#define DS1302_SECONDS           0x80
#define DS1302_MINUTES           0x82
#define DS1302_HOURS             0x84
#define DS1302_DATE              0x86
#define DS1302_MONTH             0x88
#define DS1302_DAY               0x8A
#define DS1302_YEAR              0x8C
#define DS1302_ENABLE            0x8E
#define DS1302_TRICKLE           0x90
#define DS1302_CLOCK_BURST       0xBE
#define DS1302_CLOCK_BURST_WRITE 0xBE
#define DS1302_CLOCK_BURST_READ  0xBF
#define DS1302_RAMSTART          0xC0
#define DS1302_RAMEND            0xFC
#define DS1302_RAM_BURST         0xFE
#define DS1302_RAM_BURST_WRITE   0xFE
#define DS1302_RAM_BURST_READ    0xFF

DS1302::DS1302(
  volatile uint8_t* sclk_port, volatile uint8_t* sclk_ddr, uint8_t sclk_pbn,
  volatile uint8_t* io_port, volatile uint8_t* io_ddr, volatile uint8_t* io_pin, uint8_t io_pbn,
  volatile uint8_t* ce_port, volatile uint8_t* ce_ddr, uint8_t ce_pbn
) {
  this->sclk_port = sclk_port;
  this->sclk_pbn = sclk_pbn;
  this->ce_port = ce_port;
  this->ce_pbn = ce_pbn;
  this->io_port = io_port;
  this->io_ddr = io_ddr;
  this->io_pin = io_pin;
  this->io_pbn = io_pbn;

  *sclk_ddr |= _BV(sclk_pbn); // Configure pin as output
  *sclk_port &= ~(_BV(sclk_pbn)); // Set initial state to low

  *ce_ddr |= _BV(ce_pbn);
  *ce_port &= ~(_BV(ce_pbn));
}

void DS1302::clock_burst_read(uint8_t *p) {
  int i;

  start();

  /*
  * Instead of the address, the CLOCK_BURST_READ command is issued the I/O-line
  * is released for the data
  */
  togglewrite(DS1302_CLOCK_BURST_READ, true);

  for(i=0; i<8; i++) {
    *p++ = toggleread();
  }

  stop();
}

void DS1302::togglewrite(uint8_t data, uint8_t release) {
  int i;

  for( i = 0; i <= 7; i++) {
    // set a bit of the data on the I/O-line
    //digitalWrite(DS1302_IO_PIN, bitRead(data, i));
    if ((data >> i) & 0x01) {
      *io_port |= _BV(io_pbn);
    } else {
      *io_port &= ~(_BV(io_pbn));
    }

    _delay_us(1); // tDC = 200ns

    // clock up, data is read by DS1302
    *sclk_port |= _BV(sclk_pbn); //digitalWrite( DS1302_SCLK_PIN, HIGH);
    _delay_us(1); // tCH = 1000ns, tCDH = 800ns

    if(release && i == 7) {
      /*
      * If this write is followed by a read, the I/O-line should be released
      * after the last bit, before the clock line is made low. This is according
      * the datasheet. I have seen other programs that don't release the
      * I/O-line at this moment, and that could cause a shortcut spike on the
      * I/O-line.
      */
      *io_ddr &= ~(_BV(io_pbn)); //pinMode( DS1302_IO_PIN, INPUT);
      *io_port &= ~(_BV(io_pbn));

      // For Arduino 1.0.3, removing the pull-up is no longer needed.
      // Setting the pin as 'INPUT' will already remove the pull-up.
      // digitalWrite (DS1302_IO, LOW); // remove any pull-up
    } else {
      *sclk_port &= ~(_BV(sclk_pbn)); //digitalWrite( DS1302_SCLK_PIN, LOW);
      _delay_us(1); // tCL=1000ns, tCDD=800ns
    }
  }
}

uint8_t DS1302::toggleread(void) {
  uint8_t i, data;

  data = 0;

  for(i = 0; i <= 7; i++) {
    /*
    * Issue a clock pulse for the next databit. If the 'togglewrite' function
    * was used before this function, the SCLK is already high.
    */
    *sclk_port |= _BV(sclk_pbn); //digitalWrite( DS1302_SCLK_PIN, HIGH);
    _delay_us(1); //delayMicroseconds( 1);

    // Clock down, data is ready after some time.
    *sclk_port &= ~(_BV(sclk_pbn)); //digitalWrite( DS1302_SCLK_PIN, LOW);
    _delay_us(1); //delayMicroseconds( 1);        // tCL=1000ns, tCDD=800ns

    // read bit, and set it in place in 'data' variable
    //bitWrite( data, i, digitalRead( DS1302_IO_PIN));
    if (*io_pin & _BV(io_pbn)) data |= (1UL << i);
  }

  return data;
}

void DS1302::start(void) {
  /*digitalWrite( DS1302_CE_PIN, LOW); // default, not enabled
  pinMode( DS1302_CE_PIN, OUTPUT);

  digitalWrite( DS1302_SCLK_PIN, LOW); // default, clock low
  pinMode( DS1302_SCLK_PIN, OUTPUT);*/

  *io_ddr |= _BV(io_pbn); //pinMode( DS1302_IO_PIN, OUTPUT);

  *ce_port |= _BV(ce_pbn); // start the session
  _delay_us(4); // tCC = 4us
}

void DS1302::stop(void) {
  *ce_port &= ~(_BV(ce_pbn));

  _delay_us(4); // tCWH = 4us
}
