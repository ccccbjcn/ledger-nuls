#ifndef NULS_RAM_VARIABLES_H
#define NULS_RAM_VARIABLES_H

#include "os.h"
#include "cx.h"
#include "nuls_context.h"


extern request_context_t reqContext;
extern transaction_context_t txContext;

extern cx_ecfp_public_key_t public_key;
extern cx_ecfp_private_key_t private_key;

#endif
