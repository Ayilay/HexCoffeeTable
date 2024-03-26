#include <TinyWireS.h>
#include <FastLED.h>

extern "C" {
#include "commands.h"
#include "PlatIO.h"
#include "Assert.h"
}

#define NUM_CELLS       ( 8 )
#define LEDS_PER_CELL   ( 12 )
#define NUM_LEDS        ( NUM_CELLS * LEDS_PER_CELL )

#define MAX_PAYLOAD     ( 24 )

CRGB leds[NUM_LEDS];

volatile byte RowControllerData;

enum {
    NO_FORCE = 0,
    FORCE    = 1,
};

enum updateMode_e {
    UPDATE_IMMEDIATELY,
    UPDATE_WAIT,
};

struct state_t {
    enum updateMode_e updateMode;
} State = {
    .updateMode = UPDATE_IMMEDIATELY,
};

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

#define ISR_PEND( WHICH )   ( pendIsr |=  (WHICH) )
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
    FastLED.addLeds<WS2812,LED_PIN,RGB>(leds,NUM_LEDS);


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

//------------------------------------------------------------------------------
//  COMMAND Definitions
//------------------------------------------------------------------------------

void CMD_SetRow3B ( struct cmd_ctx *pCtx )
{
    ASSERT_RET_VOID( pCtx );
    ASSERT_RET_VOID( pCtx->len == 24 );

    // whichCell in range [ 0, 8 )
    for( uint8_t whichCell = 0; whichCell < NUM_CELLS; whichCell++ )
    {
        uint8_t r, g, b;
        r = pCtx->payload[ (whichCell*3) + 0 ];
        g = pCtx->payload[ (whichCell*3) + 1 ];
        b = pCtx->payload[ (whichCell*3) + 2 ];

        // ledInCell in range [ 0, 12 )
        for( uint8_t ledInCell = 0; ledInCell < LEDS_PER_CELL; ledInCell++)
        {
            // offset in range [ 0, 96 )
            uint8_t offset;
            offset  = whichCell * LEDS_PER_CELL;
            offset += ledInCell;
            
            leds[ offset ] = CRGB( r, g, b );
        }
    }

    UpdateWork( 0 );
}

void CMD_SetCell3B ( struct cmd_ctx *pCtx )
{
    ASSERT_RET_VOID( pCtx );
    ASSERT_RET_VOID( pCtx->len == 4 );

    uint8_t whichCell = pCtx->payload[ 0 ];

    uint8_t r, g, b;
    r = pCtx->payload[ 1 ];
    g = pCtx->payload[ 2 ];
    b = pCtx->payload[ 3 ];

    for( uint8_t ledInCell = 0; ledInCell < LEDS_PER_CELL; ledInCell++ )
    {
        uint8_t offset;
        offset  = whichCell * LEDS_PER_CELL;
        offset += ledInCell;

        leds[ offset ] = CRGB( r, g, b );
    }

    UpdateWork( NO_FORCE );
}

void CMD_ConfigUpdate ( struct cmd_ctx *pCtx )
{
    ASSERT_RET_VOID( pCtx );
    ASSERT_RET_VOID( pCtx->len == 1 );

    #define MODE_UPDATE_IMMEDIATE  0 
    #define MODE_UPDATE_WAIT       1 
    #define MODE_UPDATE_NOW        2 

    uint8_t sub_cmd = pCtx->payload[ 0 ];
    switch( sub_cmd ) 
    {
        case MODE_UPDATE_IMMEDIATE:
            State.updateMode = UPDATE_IMMEDIATELY;
            break;

        case MODE_UPDATE_WAIT:
            State.updateMode = UPDATE_WAIT;
            break;

        case MODE_UPDATE_NOW:
            UpdateWork( FORCE );
            break;

        default:
            return;
    }
}


//------------------------------------------------------------------------------
//  COMMAND Helpers
//------------------------------------------------------------------------------

/** Update the LEDs (if applicable) by driving the LED pin.
 *  
 *  \param force    If 1, then forceupdate. Otherwise, use configured setting
 */
void UpdateWork( int force )
{
    if( force || State.updateMode == UPDATE_IMMEDIATELY ) 
    {
        FastLED.show();
    }
    else 
    {
        return;
    }
}
