#include <stdio.h>
#include <util/delay.h>

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

  /*rtc.h24.Hour10 = 2;
  rtc.h24.Hour = 3;
  rtc.Minutes10 = 5;
  rtc.Minutes = 7;*/

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
