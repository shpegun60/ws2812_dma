#ifndef __WS2812B_CONF_H
#define __WS2812B_CONF_H

#include <main.h>


// cross platform -----------------------------------------------------------------------------------------------------

#define WS2812B_TIMER_TYPE 		TIM_HandleTypeDef
#define WS2812B_TIMER_CH_TYPE 	uint32_t
#define WS2812B_TIMER_CCR_TYPE 	uint16_t
#define WS2812B_SET_COMPARE(timer, channel, compare) 	__HAL_TIM_SET_COMPARE((timer), (channel), (compare))
#define WS2812B_GET_COMPARE(timer, channel) 		 	__HAL_TIM_GET_COMPARE((timer), (channel))

#define WS2812B_TIM_STOP_DMA(timer, channel) 							HAL_TIM_PWM_Stop_DMA((timer), (channel))
#define WS2812B_TIM_START_DMA(timer, channel, buffer, size) 			HAL_TIM_PWM_Start_DMA((timer), (channel), (buffer), (size));
//---------------------------------------------------------------------------------------------------------------------

#define WS2812B_USE_GAMMA_CORRECTION
#define WS2812B_USE_PRECALCULATED_GAMMA_TABLE

#define WS2812B_BUFFER_SIZE     64
#define WS2812B_START_SIZE      2


#define WS2812B_PULSE_HIGH      76
#define WS2812B_PULSE_LOW       30

#endif //__WS2812B_CONF_H
