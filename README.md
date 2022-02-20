## Nonblocking continious ws2812 handler

```c
#include "ws2812b.h"

#define NUMBER_OF_LEDS 24

typedef struct {
	RGB_t colors [NUMBER_OF_LEDS]; // or HSV_t
	ws2812b_t ws2812b_obj;
} WS2812_Handle;

extern TIM_HandleTypeDef htim1;
WS2812_Handle ws2812;

int main () {

	ws2812b_Init(&ws2812->ws2812b_obj, &htim1, TIM_CHANNEL_4);
	
	//white color load
	for(uint32_t i = 0; i < NUMBER_OF_LEDS; ++i) {
		ws2812->colors[i].r = 0xFF;
		ws2812->colors[i].g = 0xFF;
		ws2812->colors[i].b = 0xFF;
	}
	
	while(1) {
		if(ws2812b_IsReady(&ws2812->ws2812b_obj))  {
			ws2812b_SendRGB(&ws2812->ws2812b_obj, ws2812->colors, NUMBER_OF_LEDS);
		}
	}
}


// callbacks overwriting
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance == ws2812.ws2812b_obj.tim->Instance) {
		ws2812b_dma_handler(&ws2812.ws2812b_obj, 0);
	}
}

void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance == ws2812.ws2812b_obj.tim->Instance) {
		ws2812b_dma_handler(&ws2812.ws2812b_obj, 1);
	}
}


```
