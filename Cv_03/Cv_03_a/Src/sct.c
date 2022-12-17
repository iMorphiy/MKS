/*
 * sct.c
 *
 *  Created on: 12. 10. 2022
 *      Author: xalech00
 */
#include <stdint.h>
#include "stm32f030x8.h"
#include "sct.h"

#define sct_nla(x) do { if (x) GPIOB->BSRR = (1 << 5); else GPIOB->BRR = (1 << 5); } while (0);
#define sct_sdi(x) do { if (x) GPIOB->BSRR = (1 << 4); else GPIOB->BRR = (1 << 4); } while (0);
#define sct_clk(x) do { if (x) GPIOB->BSRR = (1 << 3); else GPIOB->BRR = (1 << 3); } while (0);
#define sct_noe(x) do { if (x) GPIOB->BSRR = (1 << 10); else GPIOB->BRR = (1 << 10); } while (0);

void sct_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // enable clock;
	GPIOB->MODER |= GPIO_MODER_MODER3_0; // pin PB3 to output;
	GPIOB->MODER |= GPIO_MODER_MODER4_0; // pin PB4 to output;
	GPIOB->MODER |= GPIO_MODER_MODER5_0; // pin PB5 to output;
	GPIOB->MODER |= GPIO_MODER_MODER10_0; // pin PB6 to output;
}
void sct_led(uint32_t value) {
	for (uint32_t i=0; i<32; i++){
		sct_sdi(value & 1);
		sct_clk(1); //clock pulse;
		sct_clk(0);
		value>>=1;
	}
	sct_nla(1); //latch bit pulse;
	sct_nla(0);
}
void sct_value(uint16_t value) {
	static const uint32_t reg_values[3][10] = {
	{
	//PCDE--------GFAB @ DIS1
	0b0111000000000111 << 16,
	0b0100000000000001 << 16,
	0b0011000000001011 << 16,
	0b0110000000001011 << 16,
	0b0100000000001101 << 16,
	0b0110000000001110 << 16,
	0b0111000000001110 << 16,
	0b0100000000000011 << 16,
	0b0111000000001111 << 16,
	0b0110000000001111 << 16,
	},
	{
	//----PCDEGFAB---- @ DIS2
	0b0000011101110000 << 0,
	0b0000010000010000 << 0,
	0b0000001110110000 << 0,
	0b0000011010110000 << 0,
	0b0000010011010000 << 0,
	0b0000011011100000 << 0,
	0b0000011111100000 << 0,
	0b0000010000110000 << 0,
	0b0000011111110000 << 0,
	0b0000011011110000 << 0,
	},
	{
	//PCDE--------GFAB @ DIS3
	0b0111000000000111 << 0,
	0b0100000000000001 << 0,
	0b0011000000001011 << 0,
	0b0110000000001011 << 0,
	0b0100000000001101 << 0,
	0b0110000000001110 << 0,
	0b0111000000001110 << 0,
	0b0100000000000011 << 0,
	0b0111000000001111 << 0,
	0b0110000000001111 << 0,
	},
	};
	uint32_t reg=0;
	reg |= reg_values[0][value / 100 % 10];
	reg |= reg_values[1][value / 10 % 10];
	reg |= reg_values[2][value / 1 % 10];
	sct_led(reg);

}


