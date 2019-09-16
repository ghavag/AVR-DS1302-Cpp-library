#ifndef DS1302_h
#define DS1302_h

#include <stdio.h>
#include <avr/io.h>

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

/*
* Macros to convert the bcd values of the registers to normal integer variables.
* The code uses separate variables for the high byte and the low byte of the
* bcd, so these macros handle both bytes separately.
*/
#define bcd2bin(h,l) (((h)*10) + (l))
#define bcd2bin_b(x) ((((x & 0xF0) >> 4) * 10) + (x & 0x0F))
#define bin2bcd_h(x) ((x)/10)
#define bin2bcd_l(x) ((x)%10)
#define bin2bcd_b(x) ((((x)/10) << 4) + ((x)%10))

/*
* Structure for the first 8 registers. These 8 bytes can be read at once with
* the 'clock burst' command. Note that this structure contains an anonymous
* union. It might cause a problem on other compilers.
*/
typedef struct ds1302_struct
{
  uint8_t Seconds:4;      // low decimal digit 0-9
  uint8_t Seconds10:3;    // high decimal digit 0-5
  uint8_t CH:1;           // CH = Clock Halt
  uint8_t Minutes:4;
  uint8_t Minutes10:3;
  uint8_t reserved1:1;
  union
  {
    struct
    {
      uint8_t Hour:4;
      uint8_t Hour10:2;
      uint8_t reserved2:1;
      uint8_t hour_12_24:1; // 0 for 24 hour format
    } h24;
    struct
    {
      uint8_t Hour:4;
      uint8_t Hour10:1;
      uint8_t AM_PM:1;      // 0 for AM, 1 for PM
      uint8_t reserved2:1;
      uint8_t hour_12_24:1; // 1 for 12 hour format
    } h12;
  };
  uint8_t Date:4;           // Day of month, 1 = first day
  uint8_t Date10:2;
  uint8_t reserved3:2;
  uint8_t Month:4;          // Month, 1 = January
  uint8_t Month10:1;
  uint8_t reserved4:3;
  uint8_t Day:3;            // Day of week, 1 = first day (any day)
  uint8_t reserved5:5;
  uint8_t Year:4;           // Year, 0 = year 2000
  uint8_t Year10:4;
  uint8_t reserved6:7;
  uint8_t WP:1;             // WP = Write Protect
};

class DS1302 {
public:
  /*
  * Class constructor
  *
  * Params:
  *   *_port: Pointer to the AVR I/O port register to that the serial clock
  *           (SCLK), data (IO) or chip enable (CE) pin of the DS1302 are
  *           connected to. (For each of the three pins a different AVR port can
  *           be used.) Valid values are e. g. &PORTB, &PORTC, &PORTD and so on.
  *   *_ddr: Pointer to the AVR I/O port data direction register of the choosen
  *          AVR I/O port to that the SCLK, IO or CE pin of the DS1302 is
  *          connected to. Valid values are e. g. &DDRB, &DDRC, &DDRD and so on.
  *          The value must match the given port in the *_port register. I. e.
  *          if sclk_port is &PORTB, sclk_reg must be set to &DDRB.
  *   io_pin: Pointer to the AVR I/O pin register of the choosen AVR I/O port to
  *           that the IO pin of the DS1302 is connected to.
  *   *_pbn: Number of the pin of the choosen port to which the SCLK, IO or CE
  *          pin of the DS1302 is connected to. Valid values depending on the
  *          choosen AVR I/O port. E. g. if the corresponding *_port parameter
  *          is set to &PORTB, valid values are PB1, PB2, PB3 and so on.
  */
  DS1302(
    volatile uint8_t* sclk_port, volatile uint8_t* sclk_ddr, uint8_t sclk_pbn,
    volatile uint8_t* io_port, volatile uint8_t* io_ddr, volatile uint8_t* io_pin, uint8_t io_pbn,
    volatile uint8_t* ce_port, volatile uint8_t* ce_ddr, uint8_t ce_pbn
  );

  /*
  * Reads a single byte from the DS1302.
  *
  * Params:
  *   address: Address of the register to be read (see address register defines
  *            above).
  */
  uint8_t read(int address);

  /*
  * Writes a single byte to the DS1302.
  *
  * Params:
  *   address: Address of the register to be written (see address register
  *            defines above).
  *   data: Byte to be written to the addressed register.
  */
  void write(int address, uint8_t data);

  /*
  * Reads 8 bytes clock data at once (burst mode) from the DS1302.
  *
  * Params:
  *   p: Pointer to a ds1302_struct variable in which the data will be stored.
  */
  void clock_burst_read(uint8_t *p);

  /*
  * Writes 8 bytes clock data at once (burst mode) to the DS1302.
  *
  * Params:
  *   p: Pointer to a ds1302_struct variable that contains the data to be
  *      written to the DS1302.
  */
  void clock_burst_write(uint8_t *p);

private:
  volatile uint8_t* sclk_port;
  uint8_t sclk_pbn;
  volatile uint8_t* ce_port;
  uint8_t ce_pbn;
  volatile uint8_t* io_port;
  volatile uint8_t* io_ddr;
  volatile uint8_t* io_pin;
  uint8_t io_pbn;

  /*
  * Helper method to setup the start condition.
  *
  * An 'init' function is not used. But now the pinMode is set every time.
  * That's not a big deal, and it's valid. At startup, the pins of the ATmega
  * are high impedance. Since the DS1302 has pull-down resistors, the signals
  * are low (inactive) until the DS1302 is used.
  */
  void start(void);

  /*
  * Helper method to finish the communication.
  */
  void stop(void);

  /*
  * Helper method for reading a byte with bit toggle.
  *
  * This method assumes that the SCLK is still high.
  */
  uint8_t toggleread(void);

  /*
  * Helper method for writing a byte with bit toggle.
  *
  * Params:
  *   data: Byte to be written to the DS1302.
  *   release: Must be true (non-zero) if a read follows this write. (Basically
  *            every read transmission starts with a one byte write to the
  *            command register.) It will then release the I/O-line and will
  *            keep the SCLK high.
  */
  void togglewrite(uint8_t data, uint8_t release);
};

#endif // DS1302_h
