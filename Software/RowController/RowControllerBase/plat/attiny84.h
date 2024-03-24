#ifndef HEX_ROW_PLAT_TINY84_H
#define HEX_ROW_PLAT_TINY84_H

//------------------------------------------------------------------------------
//  IO Declarations
//------------------------------------------------------------------------------

/*  ATTiny85 Pin Map
 *
 *  Port/Pin  Arduino    Alt Fn
 *   PA0        0
 *   PA1        1
 *   PA2        2
 *   PA3        3
 *   PA4        4       SCL (USI-I2C)
 *   PA5        5 
 *   PA6        6       SDA (USI-I2C)
 */

#define I2C_SDA  6
#define I2C_SCL  4

#define LED_PIN  3


#endif
