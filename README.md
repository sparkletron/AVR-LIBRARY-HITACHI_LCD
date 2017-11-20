# Hitachi LCD Library

Allows control over the 4 bit bus of the hitachi based LCD screens.

## Example

```c
#include <inttypes.h>
#include <avr/common.h>
#include <avr/io.h>
#include <util/delay.h>
#include "hitachiLcd.h"

int main(void)
{
	initLCD(&PORTD, 2, 16, 0, 10);

	for(;;)
	{
		clearLCD();
		
		_delay_ms(1000);
		
		printLCD("Hello World");
	}
}
```
