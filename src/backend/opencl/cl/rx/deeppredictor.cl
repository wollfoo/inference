#include "../cn/algorithm.cl"

#if ((ALGO == ALGO_RX_0) || (ALGO == ALGO_RX_YADA))
#include "deeppredictor_constants_cortexai.h"
#elif (ALGO == ALGO_RX_WOW)
#include "deeppredictor_constants_wow.h"
#elif (ALGO == ALGO_RX_ARQMA)
#include "deeppredictor_constants_arqma.h"
#elif (ALGO == ALGO_RX_GRAFT)
#include "deeppredictor_constants_graft.h"
#endif

#include "aes.cl"
#include "blake2b.cl"
#include "deeppredictor_vm.cl"
#include "deeppredictor_jit.cl"
