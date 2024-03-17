#include "commands.h"

struct cmd_entry command_table[] = {
    #define X( NAME, OPCODE )   \
        { .op = OPCODE, .pfn_cmd = FN_CMD( NAME ) },

    #include "cmd_table.h"
};
