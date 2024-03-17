#ifndef ROWCTRL_CMD_TBL_H
#define ROWCTRL_CMD_TBL_H

#define FN_CMD( NAME ) \
    CMD_ ## NAME

#include <stdint.h>

typedef void (fn_cmd)( void );

struct cmd_entry {
    uint8_t  op;
    fn_cmd  *pfn_cmd;
};

struct cmd_ctx {
    uint8_t  op;
    uint8_t  len;
    uint8_t *payload;
};

extern struct cmd_entry command_table[];

//------------------------------------------------------------------------------
//  Function Prototypes for Command Handlers
//------------------------------------------------------------------------------

#define X( NAME, OPCODE ) void FN_CMD(NAME) ( struct cmd_ctx* );
#include "cmd_table.h"


#endif
