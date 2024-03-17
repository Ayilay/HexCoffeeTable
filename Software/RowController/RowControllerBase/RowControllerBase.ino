#include <TinyWireS.h>
#include <FastLED.h>

extern "C" {
#include "commands.h"
}

// Row Controller has 8 cells/row, each has 12 LEDs
#define NUM_CELLS           ( 8 )
#define LEDS_PER_CELL       ( 12 )
#define NUM_LEDS            ( NUM_CELLS * LEDS_PER_CELL )

#define MAX_PAYLOAD         ( 24 )

//------------------------------------------------------------------------------
//  IO Declarations
//------------------------------------------------------------------------------
#define DATA_PIN 2

CRGB leds[NUM_LEDS];

volatile byte RowControllerData;

//------------------------------------------------------------------------------
//  ISR Structs and Forward Decls
//------------------------------------------------------------------------------

enum isrFlags {
    none_pending   = (uint8_t) (0),

    pend_i2c_recv  = (uint8_t) (1 << 0),
    pend_i2c_send  = (uint8_t) (1 << 1),
};

// ISR must execute QUICK.
// When ISR executes, we signal to app to do the "heavy processing"
// This is the "BODY_xxx" functions.
// The BODY_xx func must CLEAR the bit when done.
volatile enum isrFlags pendIsr;

#define ISR_PEND( WHICH )   ( pendIsr |= WHICH )
#define ISR_DONE( WHICH )   ( pendIsr &= ~(WHICH) )

void ISR_I2C_Recv( uint8_t );
void ISR_I2C_Send( void );

void BODY_I2C_Recv( void );
void BODY_I2C_Send( void );

//------------------------------------------------------------------------------
//  Private Helpers
//------------------------------------------------------------------------------

void DispatchPendingISR( void )
{
    switch( pendIsr ){
    default:
        // FUTURE: freak out
        break;
    case none_pending:
        // Nothing to do
        break;
    case pend_i2c_recv:
        BODY_I2C_Recv();
        break;
    case pend_i2c_send:
        BODY_I2C_Send();
        break;
    }
}

void DispatchCommand( struct cmd_ctx *ctx)
{
    switch( ctx->op ){
        #define X( NAME, OPCODE ) \
            case OPCODE: FN_CMD( NAME ) ( ctx ); break;  

        #include "cmd_table.h"
    }
}

//------------------------------------------------------------------------------
//  Main
//------------------------------------------------------------------------------

void setup() { 
    cli();
    FastLED.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);


    TinyWireS.begin( I2C_ADDR );
    TinyWireS.onReceive( ISR_I2C_Recv );
    TinyWireS.onRequest( ISR_I2C_Send );

    ISR_DONE( none_pending );
    sei();
}

void loop() 
{ 
    DispatchPendingISR();

    // TODO: Poll Cells for Status
}

//------------------------------------------------------------------------------
//  Interrupt Function Body
//------------------------------------------------------------------------------

void ISR_I2C_Recv( uint8_t arg )
{
    (void) arg;

    cli();                             // DISABLE IRQ 
    ISR_PEND( pend_i2c_recv );
}

void ISR_I2C_Send( void )
{
    cli();                             // DISABLE IRQ 
    ISR_PEND( pend_i2c_send );
}

void BODY_I2C_Recv( void )
{
    uint8_t payload[ MAX_PAYLOAD ];
    struct cmd_ctx ctx;

    ctx.payload = &payload[0];
    ctx.len = 0;

    ctx.op = TinyWireS.read();

    while( TinyWireS.available() > 0 )
    {
        ctx.payload[ ctx.len++ ] = TinyWireS.read();
    }

    DispatchCommand( &ctx );

    ISR_DONE( pend_i2c_recv );
    sei();                             // ENABLE IRQ 
}

void BODY_I2C_Send( void )
{
    TinyWireS.write( RowControllerData );

    ISR_DONE( pend_i2c_send );
    sei();                             // ENABLE IRQ 
}

