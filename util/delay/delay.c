#include <stdint.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "delay.h"

void delay_setup(void)
{
	/* set up a microsecond free running timer for ... things... */
	rcc_periph_clock_enable(RCC_TIM9);
	/* microsecond counter */
	timer_set_prescaler(TIM9, 100000000 / 1000000 - 1);
	timer_set_period(TIM9, 0xffff);
	timer_one_shot_mode(TIM9);
}

void delay_us(uint16_t us)
{
	TIM_ARR(TIM9) = us;
	TIM_EGR(TIM9) = TIM_EGR_UG;
	TIM_CR1(TIM9) |= TIM_CR1_CEN;
	//timer_enable_counter(TIM6);
	while (TIM_CR1(TIM9) & TIM_CR1_CEN);
}

void delay_ms(uint16_t ms){
  for (uint16_t i = 0; i <= ms; i++)
  {
    delay_us(1000);
  }
  
}