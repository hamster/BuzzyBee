
#define F_CPU 20000000

#include <atmel_start.h>
#include <util/delay.h>

extern volatile uint8_t measurement_done_touch;
extern volatile uint8_t LED_PWM[2];
extern volatile uint16_t timeTicks;

// Logarithmic brightness levels
const uint8_t pwm_log[] = {30,43,59,78,102,137,196,255};
	
typedef enum {
	MODE_TWINKLE,
	MODE_BOUNCE,
	MODE_RANDOM
} RUNMODE;

int main(void){
	
	uint8_t key_status = 0;
	
	system_init();
	touch_init();
	
	cpu_irq_enable(); /* Global Interrupt Enable */
	
	LED_LEFT_set_level(true);
	_delay_ms(200);
	LED_RIGHT_set_level(true);
	
	VIBE_set_level(true);
	_delay_ms(50);
	VIBE_set_level(false);
	_delay_ms(50);
	VIBE_set_level(true);
	_delay_ms(50);
	VIBE_set_level(false);	
	_delay_ms(50);
	VIBE_set_level(true);
	_delay_ms(50);
	VIBE_set_level(false);
	_delay_ms(50);
	
	TIMER_0_init();
	
	LED_PWM[0] = 14;
	LED_PWM[1] = 14;
	
	uint8_t brightnessPosition = 0;
	uint8_t twinkleLED = 0;
	bool directionUp = true;
	bool twinkling = false;
	uint8_t randomLED = 0;
	
	uint8_t counter = 0;
	
	RUNMODE mode = MODE_TWINKLE;
	
	bool touched = false;
	bool buzzed = false;
	

	/* Replace with your application code */
	while (1) {
		
		
		if(!touched && !buzzed){
			
			if(mode == MODE_TWINKLE){
				if(!twinkling){
					// Not twinkling, wait a random time before starting a twinkle
					if((rand() % 500) == 0){
						// Time to twinkle
						twinkling = true;
						// Pick a new LED
						twinkleLED = rand() % 2;
					}
				}
				
				if(((timeTicks % 50) < 2) && twinkling){
					if(directionUp){
						// Go up in brightness
						LED_PWM[twinkleLED] = pwm_log[brightnessPosition++];
						if(brightnessPosition == 8){
							directionUp = false;
						}
					}
					else{
						// Go down in brightness
						LED_PWM[twinkleLED] = pwm_log[--brightnessPosition];
						if(brightnessPosition == 0){
							directionUp = true;
							twinkling = false;
							LED_PWM[0] = 14;
							LED_PWM[1] = 14;
						}
					}
				}
			}
			
			if(mode == MODE_BOUNCE){
				if(counter == 75){
					counter = 0;
					if(randomLED == 0){
						LED_PWM[0] = 14;
						LED_PWM[1] = 128;
						randomLED = 1;
					}
					else{
						LED_PWM[0] = 128;
						LED_PWM[1] = 14;
						randomLED = 0;
					}
					
				}
				else{
					counter++;
				}
			}
			
			if(mode == MODE_RANDOM){
				if(counter == 150){
					counter = 0;
					
					LED_PWM[0] = rand() % 128;
					LED_PWM[1] = rand() % 128;
				}
				else{
					counter++;
				}
				
			}
			
		}
		else if(!buzzed){
			
			// Mode is about to change, light up all the LEDs
			for(uint8_t i = 0; i < 2; i++){
				LED_PWM[i] = 184;
			}
			
		}		
		
		touch_process();
		if (measurement_done_touch == 1) {
			key_status = get_sensor_state(0) & KEY_TOUCHED_MASK;
			if (0u != key_status) {
				touched = true;
				_delay_ms(5);
			} else {
				if(touched){
					switch(mode){
						case MODE_TWINKLE:
							// Move to bounce mode
							twinkleLED = 0;
							directionUp = true;
							mode = MODE_BOUNCE;
							break;
						case MODE_BOUNCE:
							// Move to off mode
							mode = MODE_RANDOM;
							break;
						case MODE_RANDOM:
							// Move to twinkle mode
							LED_PWM[0] = 14;
							LED_PWM[1] = 14;
							twinkleLED = 0;
							directionUp = true;
							brightnessPosition = 0;
							mode = MODE_TWINKLE;
							break;
					}

					touched = false;
				}
			}
			
			// Vibrate
			key_status = get_sensor_state(1) & KEY_TOUCHED_MASK;
			if (0u != key_status) {
				VIBE_set_level(true);
				LED_PWM[0] = 255;
				LED_PWM[1] = 255;
				buzzed = true;
			} else {
				if(buzzed){
					VIBE_set_level(false);
					LED_PWM[0] = 14;
					LED_PWM[1] = 14;
					buzzed = false;
				}
			}			
		}	
		
		_delay_ms(1);	
		
	}
}
