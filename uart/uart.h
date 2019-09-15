#ifndef _uart_h
#define _uart_h

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void uart_putchar(char c, FILE *stream);
char uart_getchar(FILE *stream);

void uart_init(void);

/* http://www.ermicro.com/blog/?p=325 */

FILE uart_output;
FILE uart_input;

#ifdef __cplusplus
}
#endif

#endif /* _uart_h */
