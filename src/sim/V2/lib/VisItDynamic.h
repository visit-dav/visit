#ifndef VISIT_DYNAMIC_H
#define VISIT_DYNAMIC_H
#include "SimV2Tracing.h"

#define VISIT_DYNAMIC_EXECUTE(FUNC, FUNCTYPE, FUNCVAR, EXEC_CB) \
    int retval = VISIT_ERROR; \
    LIBSIM_API_ENTER(VisIt_##FUNC);\
    {\
        FUNCVAR = (FUNCTYPE)visit_get_runtime_function("simv2_"#FUNC);\
        if(cb != NULL)\
        { \
            retval = EXEC_CB;\
            if(retval == VISIT_ERROR) \
            { \
                LIBSIM_MESSAGE("simv2_" #FUNC " returned VISIT_ERROR"); \
            } \
            else \
            { \
                LIBSIM_MESSAGE("simv2_" #FUNC " returned VISIT_OKAY"); \
            } \
        } \
    }\
    LIBSIM_API_LEAVE(VisIt_##FUNC); \
    return retval;

void *visit_get_runtime_function(const char *name);

#endif
