#if defined( ARDUINO_AVR_ATTINYX5 )
    #include "plat/attiny85.h"
#elif defined( ARDUINO_AVR_ATTINYX4 )
    #include "plat/attiny84.h"
#else
#error Unsupported Platform; We ONLY support ATTiny85 and ATTiny84
#endif
