
#include "libLED.h"

// TODO::: convert to new CSL interface.
static GPIO_Handle hGpio;

void ledInit()
{
	Uint32 	pinID;
	
	// open GPIO device
	hGpio = GPIO_open(GPIO_DEV0, 0);
	
	pinID = GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3;
	
	GPIO_reset(hGpio);
	GPIO_clear(hGpio);
	
	GPIO_pinEnable( hGpio, pinID );
	GPIO_pinDirection( hGpio, pinID, GPIO_OUTPUT );
	
	// For a start, turn off all LEDs
	GPIO_pinWrite( hGpio, pinID, 0xFFFFFFFF);
}

void ledLight( int lednum, Bool on)
{

	Uint32 pin;
	
	switch (lednum)
	{
	case 0:
		pin = GPIO_PIN0;
		break;
		
	case 1:
		pin = GPIO_PIN1;
		break;
		
	case 2:
		pin = GPIO_PIN2;
		break;
		
	case 3:
		pin = GPIO_PIN3;
		break;
	}
			
	if (on)
		GPIO_pinWrite( hGpio, pin, 0 );
	else
		GPIO_pinWrite( hGpio, pin, 1 );

}

void ledBlink()
{

	Int		pass_counter;
	Int		pulse_counter;
	Int		time;
	
	float	kitt;
	float	kitt_delta;
	
	Int		LED[4];
	int		led;
	
	// There are 10'000 ticks per second. We use a resolution of 0-100 for pulsing the LEDs
	// that leaves us with 100 Hz update rate.
	
	kitt = 0.0;
	kitt_delta = 4.0 * LED_KITT_SPEED / (LED_TICKSPERSECOND / LED_PULSE_RESOLUTION);
	
	while(1)
	{
		pass_counter++;
		
		// update kitt
		kitt += kitt_delta;
		if ( (kitt >= 3.0) || (kitt <= 0.0) )
			kitt_delta = -kitt_delta;
		
		//assertLED( (kitt >= 0.0) && (kitt <= 3.0));
		
		// update LEDs
		LED[0] = (int)(LED_MAX(0, (LED_KITT_RANGE - LED_DIST(kitt,0)) ) * LED_PULSE_RESOLUTION / LED_KITT_RANGE);
		LED[1] = (int)(LED_MAX(0, (LED_KITT_RANGE - LED_DIST(kitt,1)) ) * LED_PULSE_RESOLUTION / LED_KITT_RANGE);
		LED[2] = (int)(LED_MAX(0, (LED_KITT_RANGE - LED_DIST(kitt,2)) ) * LED_PULSE_RESOLUTION / LED_KITT_RANGE);
		LED[3] = (int)(LED_MAX(0, (LED_KITT_RANGE - LED_DIST(kitt,3)) ) * LED_PULSE_RESOLUTION / LED_KITT_RANGE);
		
		for ( pulse_counter = 0; pulse_counter <= LED_PULSE_RESOLUTION; pulse_counter++)
		{
			time = TSK_time();
			
			for (led=0; led<4; led++ )
			{
				if (LED[led] > pulse_counter)
					ledLight( led, TRUE );
				else
					ledLight( led, FALSE );
			}
						
			// sleep a tick, but double check, since DSP/BIOS doesn't quarantee to sleep
			// as long as intended.	
			TSK_sleep(1);			
			while ( time + 1 > TSK_time() );		
		}
	}

}


