#include <string.h>
#include "ws2812b.h"

//------------------------------------------------------------
// Internal
//------------------------------------------------------------

#define WS2820_MIN(a, b)   ({ typeof(a) a1 = a; typeof(b) b1 = b; a1 < b1 ? a1 : b1; })

#ifdef WS2812B_USE_GAMMA_CORRECTION
#ifdef WS2812B_USE_PRECALCULATED_GAMMA_TABLE
static const uint8_t LEDGammaTable[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
		10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21,
		22, 23, 23, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38,
		38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58,
		59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 84,
		85, 86, 87, 88, 89, 91, 92, 93, 94, 95, 97, 98, 99, 100, 102, 103, 104, 105, 107, 108, 109, 111,
		112, 113, 115, 116, 117, 119, 120, 121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137,
		139, 140, 142, 143, 145, 146, 148, 149, 151, 152, 154, 155, 157, 158, 160, 162, 163, 165, 166,
		168, 170, 171, 173, 175, 176, 178, 180, 181, 183, 185, 186, 188, 190, 192, 193, 195, 197, 199,
		200, 202, 204, 206, 207, 209, 211, 213, 215, 217, 218, 220, 222, 224, 226, 228, 230, 232, 233,
		235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255 };
#endif
#endif

static inline uint8_t LEDGamma(uint8_t v)
{
#ifdef WS2812B_USE_GAMMA_CORRECTION
#ifdef WS2812B_USE_PRECALCULATED_GAMMA_TABLE
	return LEDGammaTable[v];
#else
	return (v * v + v) >> 8;
#endif
#else
	return v;
#endif
}


static void RGB2PWM(RGB_t *rgb, PWM_t *pwm)
{
	uint8_t r = LEDGamma(rgb->r);
	uint8_t g = LEDGamma(rgb->g);
	uint8_t b = LEDGamma(rgb->b);

	uint8_t mask = 128;

	for (unsigned int i = 0; i < 8; i++) {
		pwm->r[i] = r & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;
		pwm->g[i] = g & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;
		pwm->b[i] = b & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;

		mask >>= 1;
	}
}

static void SrcFilterNull(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
	memset(*pwm, 0, size * sizeof(PWM_t));
	*pwm += size;
}


static void SrcFilterRGB(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
	RGB_t *rgb = *src;
	PWM_t *p = *pwm;

	*count -= size;

	while (size--) {
		RGB2PWM(rgb++, p++);
	}

	*src = rgb;
	*pwm = p;
}

static void SrcFilterHSV(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
	HSV_t *hsv = *src;
	PWM_t *p = *pwm;

	*count -= size;

	while (size--) {
		RGB_t rgb;

		HSV2RGB(hsv++, &rgb);
		RGB2PWM(&rgb, p++);
	}

	*src = hsv;
	*pwm = p;
}


static void DMASend(ws2812b_t * obj, ws18b20_SrcFilter_t *filter, void *src, unsigned count)
{
	if (obj->DMABusy) {
		return;
	}

	obj->DMABusy = 1;

	obj->DMAFilter = filter;
	obj->DMASrc = src;
	obj->DMACount = count;

	PWM_t *pwm = obj->DMABuffer;
	PWM_t *end = &obj->DMABuffer[WS2812B_BUFFER_SIZE];

	// Start sequence
	SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

	// RGB PWM data
	obj->DMAFilter(&obj->DMASrc, &pwm, &obj->DMACount, WS2820_MIN(obj->DMACount, end - pwm));

	// Rest of buffer
	if (pwm < end) {
		SrcFilterNull(NULL, &pwm, NULL, end - pwm);
	}

	// Start transfer
	WS2812B_TIM_START_DMA(obj->tim, obj->ch, (uint32_t*)obj->DMABuffer, sizeof(obj->DMABuffer) / sizeof(WS2812B_TIMER_CCR_TYPE))
}

static void DMASendNext(ws2812b_t * obj, PWM_t *pwm, PWM_t *end)
{
	if (!obj->DMAFilter) {
		// Stop transfer
		WS2812B_TIM_STOP_DMA(obj->tim, obj->ch);
		WS2812B_SET_COMPARE(obj->tim, obj->ch, 0);

		obj->DMABusy = 0;
	} else if (!obj->DMACount) {
		// Rest of buffer
		SrcFilterNull(NULL, &pwm, NULL, end - pwm);

		obj->DMAFilter = NULL;
	} else {
		// RGB PWM data
		obj->DMAFilter(&obj->DMASrc, &pwm, &obj->DMACount, WS2820_MIN(obj->DMACount, end - pwm));

		// Rest of buffer
		if (pwm < end) {
			SrcFilterNull(NULL, &pwm, NULL, end - pwm);
		}
	}
}

void ws2812b_dma_handler(ws2812b_t * obj, uint8_t isHalf)
{
	if (isHalf) {
		DMASendNext(obj, obj->DMABuffer, &obj->DMABuffer[WS2812B_BUFFER_SIZE >> 1]);
	} else {
		DMASendNext(obj ,&obj->DMABuffer[WS2812B_BUFFER_SIZE >> 1], &obj->DMABuffer[WS2812B_BUFFER_SIZE]);
	}
}

//------------------------------------------------------------
// Interface
//------------------------------------------------------------

void ws2812b_Init(ws2812b_t * obj, WS2812B_TIMER_TYPE * tim, WS2812B_TIMER_CH_TYPE ch)
{
	if(!obj) {
		return;
	}

	WS2812B_TIM_STOP_DMA(tim, ch);
	WS2812B_SET_COMPARE(tim, ch, 0);

	obj->ch = ch;
	obj->tim = tim;
	obj->DMAFilter = NULL;
	obj->DMASrc = NULL;
	obj->DMACount = 0;
	obj->DMABusy = 0;
}

void ws2812b_SendRGB(ws2812b_t * obj, RGB_t *rgb, unsigned count)
{
	if(!obj) {
		return;
	}

	DMASend(obj, &SrcFilterRGB, rgb, count);
}

void ws2812b_SendHSV(ws2812b_t * obj, HSV_t *hsv, unsigned count)
{
	if(!obj) {
		return;
	}

	DMASend(obj, &SrcFilterHSV, hsv, count);
}
