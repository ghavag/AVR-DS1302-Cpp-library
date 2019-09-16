#include "ds1302.h"
#include <util/delay.h>

/*
* Defines for the bits, to be able to change between bit number and binary
* definition.
*/
#define DS1302_D0 0
#define DS1302_D1 1
#define DS1302_D2 2
#define DS1302_D3 3
#define DS1302_D4 4
#define DS1302_D5 5
#define DS1302_D6 6
#define DS1302_D7 7

// Bit for reading (bit in address)
#define DS1302_READBIT DS1302_D0 // READBIT=1: read instruction

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

uint8_t DS1302::read(int address) {
  uint8_t data;

  address |= (1UL << DS1302_READBIT); // set lowest bit (read bit) in address

  start();
  togglewrite(address, true); // the I/O-line is released for the data
  data = toggleread();
  stop();

  return data;
}

void DS1302::write(int address, uint8_t data) {
  address &= ~(_BV(DS1302_READBIT)); // clear lowest bit (read bit) in address

  start();
  togglewrite(address, false); // don't release the I/O-line
  togglewrite(data, false); // don't release the I/O-line
  stop();
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

void DS1302::clock_burst_write(uint8_t *p) {
  int i;

  start();

  /*
  * Instead of the address, the CLOCK_BURST_WRITE command is issued. The
  * I/O-line is not released
  */
  togglewrite(DS1302_CLOCK_BURST_WRITE, false);

  for( i=0; i<8; i++) {
    togglewrite(*p++, false); // the I/O-line is not released
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
