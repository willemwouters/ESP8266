#include "espmissingincludes.h"
#include <stdarg.h>
void user_rf_pre_init(void)
{
}


int os_printf( const char *format , ... ){
    void *arg = __builtin_apply_args();
    void *ret = __builtin_apply((void*)os_printf_plus, arg, 100);
    __builtin_return(ret);
}

