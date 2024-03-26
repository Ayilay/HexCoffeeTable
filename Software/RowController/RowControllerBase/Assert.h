#ifndef PROJECJT_ASSERT_H
#define PROJECJT_ASSERT_H

// Simple assert, no return code; For basic param checking
#define ASSERT_RET_VOID( COND ) \
    if( ! ( COND ) )    return;

#endif
