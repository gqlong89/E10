#ifndef __BSWDRV_HC595_H__
#define __BSWDRV_HC595_H__


#include <stdint.h>


#define MOSIO_HIGH()   				gpio_bit_set(GPIOA, GPIO_PIN_12)
#define MOSIO_LOW()   				gpio_bit_reset(GPIOA, GPIO_PIN_12)

#define SRCLK_HIGH()   				gpio_bit_set(GPIOA, GPIO_PIN_12)
#define SRCLK_LOW()   				gpio_bit_reset(GPIOA, GPIO_PIN_12)

#define RCLK_HIGH()   				gpio_bit_set(GPIOA, GPIO_PIN_12)
#define RCLK_LOW()   				gpio_bit_reset(GPIOA, GPIO_PIN_12)




extern void HC595SendData(unsigned char SendVal);

#endif


