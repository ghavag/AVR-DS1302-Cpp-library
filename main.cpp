#include <stdio.h>
#include <util/delay.h>
#include <string.h>

#include "uart/uart.h"
#include "ds1302.h"

int main(void) {
  ds1302_struct rtc;

  /* UART initialization part */
  uart_init();
  stdout = &uart_output;
  stdin  = &uart_input;

  DS1302 ds1302 = DS1302(
    &PORTC, &DDRC, PC2,
    &PORTC, &DDRC, &PINC, PC1,
    &PORTC, &DDRC, PC0
  );

  /*
  * Un-comment the following define command to set date and time according to
  * the variables below.
  */
  #define SET_DATE_TIME
  #ifdef SET_DATE_TIME
  int seconds, minutes, hours, dayofweek, dayofmonth, month, year;

  // Hard coded date and time
  seconds    = 0;
  minutes    = 3;
  hours      = 13;
  dayofweek  = 1;  // Day of week, any day can be first, counts 1...7
  dayofmonth = 11; // Day of month, 1...31
  month      = 9;  // month 1...12
  year       = 2019;

  memset((char *) &rtc, 0, sizeof(rtc));

  rtc.Seconds    = bin2bcd_l(seconds);
  rtc.Seconds10  = bin2bcd_h(seconds);
  rtc.CH         = 0; // 1 for Clock Halt, 0 to run;
  rtc.Minutes    = bin2bcd_l(minutes);
  rtc.Minutes10  = bin2bcd_h(minutes);
  // To use the 12 hour format,
  // use it like these four lines:
  //    rtc.h12.Hour   = bin2bcd_l( hours);
  //    rtc.h12.Hour10 = bin2bcd_h( hours);
  //    rtc.h12.AM_PM  = 0;     // AM = 0
  //    rtc.h12.hour_12_24 = 1; // 1 for 24 hour format
  rtc.h24.Hour   = bin2bcd_l(hours);
  rtc.h24.Hour10 = bin2bcd_h(hours);
  rtc.h24.hour_12_24 = 0; // 0 for 24 hour format
  rtc.Date       = bin2bcd_l(dayofmonth);
  rtc.Date10     = bin2bcd_h(dayofmonth);
  rtc.Month      = bin2bcd_l(month);
  rtc.Month10    = bin2bcd_h(month);
  rtc.Day        = dayofweek;
  rtc.Year       = bin2bcd_l(year - 2000);
  rtc.Year10     = bin2bcd_h(year - 2000);
  rtc.WP = 0;

  // Write all clock data at once (burst mode).
  ds1302.clock_burst_write((uint8_t *) &rtc);
  #endif // SET_DATE_TIME

  /*** Example for read() method ***/
  // Demonstrate read() method by reading the date
  printf("Read date using read() method: YYYY/MM/DD = ");
  uint8_t data;

  data = ds1302.read(DS1302_YEAR);
  printf("%d/", 2000 + bcd2bin_b(data));

  data = ds1302.read(DS1302_MONTH);
  printf("%d/", bcd2bin_b(data));

  data = ds1302.read(DS1302_DATE);
  printf("%d\n", bcd2bin_b(data));
  /*********************************/

  /* Main loop starts here */
  while(1) {
    ds1302.clock_burst_read((uint8_t *) &rtc);

    printf("Time = %02d:%02d:%02d, ", \
      bcd2bin(rtc.h24.Hour10, rtc.h24.Hour), \
      bcd2bin(rtc.Minutes10, rtc.Minutes), \
      bcd2bin(rtc.Seconds10, rtc.Seconds));

    printf("Date(day of month) = %d, Month = %d, " \
      "Day(day of week) = %d, Year = %d\n", \
      bcd2bin(rtc.Date10, rtc.Date), \
      bcd2bin(rtc.Month10, rtc.Month), \
      rtc.Day, \
      2000 + bcd2bin( rtc.Year10, rtc.Year));

    _delay_ms(1000);
  }
}
