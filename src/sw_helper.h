#pragma once

#if defined(ENABLE_SW_DEBUG)
#define SW_BKPT(bkname) \
int bk_##bkname = 1; \
while(bk_##bkname) {}
#endif