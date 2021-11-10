#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

/**
	 * Initialize the timers used for delays.
	 */
	void delay_setup(void);

	/**
	 * busy wait for a number of usecs.
	 * @param us number of usecs to delay.
	 */
	void delay_us(uint16_t us);
  
  /**
   * busy wait for number of msecs.
   * 
   * @param ms number of ms to delay
   */
  void delay_ms(uint16_t ms);

  #endif