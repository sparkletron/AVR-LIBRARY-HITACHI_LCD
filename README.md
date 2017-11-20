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
	struct s_lcd *p_lcd = NULL;
	
	p_lcd = initLCD(&PORTD, 2, 16, 0, 10);

	if(p_lcd == NULL) return 0;
	
	for(;;)
	{
		clearLCD(p_lcd);
		
		_delay_ms(1000);
		
		printLCD(p_lcd, "Hello World");
	}
}
```
