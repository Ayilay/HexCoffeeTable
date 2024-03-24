#ifndef HEX_ROW_PLAT_TINY85_H
#define HEX_ROW_PLAT_TINY85_H

//------------------------------------------------------------------------------
//  IO Declarations
//------------------------------------------------------------------------------

/*  ATTiny85 Pin Map
 *
 *  Port/Pin  Arduino    Alt Fn
 *   PB0        0        SDA (USI-I2C)
 *   PB1        1
 *   PB2        2 / A1   SCL (USI-I2C)
 *   PB3        3 / A3  
 *   PB4        4 / A2   
 *   PB5        5 / A0   RESET
 */

#define I2C_SDA  0
#define I2C_SCL  2

#define LED_PIN  3

#endif
