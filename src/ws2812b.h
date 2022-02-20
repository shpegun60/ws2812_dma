#ifndef __WS2812B_H
#define __WS2812B_H

#include <stdint.h>
#include "color_bitmap.h"
#include "ws2812b_conf.h"

#if defined(__ICCARM__)
__packed struct PWM
#else
struct __attribute__((packed)) PWM
#endif
{
	WS2812B_TIMER_CCR_TYPE g[8], r[8], b[8];
};

typedef struct PWM PWM_t;
typedef void (ws18b20_SrcFilter_t)(void **, PWM_t **, unsigned *, unsigned);


typedef struct {
	ws18b20_SrcFilter_t *DMAFilter;
	void *DMASrc;
	unsigned DMACount;
	PWM_t DMABuffer[WS2812B_BUFFER_SIZE];
	volatile int DMABusy;

	WS2812B_TIMER_TYPE * tim;
	WS2812B_TIMER_CH_TYPE ch;
} ws2812b_t;


void ws2812b_Init(ws2812b_t * obj, WS2812B_TIMER_TYPE * tim, WS2812B_TIMER_CH_TYPE ch);

static inline int ws2812b_IsReady(ws2812b_t * obj) {
	return !obj->DMABusy;
}


void ws2812b_SendRGB(ws2812b_t * obj, RGB_t *rgb, unsigned count);
void ws2812b_SendHSV(ws2812b_t * obj, HSV_t *hsv, unsigned count);
void ws2812b_dma_handler(ws2812b_t * obj, uint8_t isHalf);

#endif //__WS2812B_H
