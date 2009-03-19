#pragma once

#ifndef UVFBASIC_H
#define UVFBASIC_H

#ifndef UVFVERSION
  #define UVFVERSION 1
#else
  #if UVFVERSION != 1
    #pragma message("[UVFBasic.h] Version mismatch.\n")
    >> VERSION MISMATCH <<
  #endif
#endif

#include "../../StdTuvokDefines.h"
#define UVF_INVALID UINT64_INVALID

#include "LargeRAWFile.h"
class DataBlock;


#endif // UVFBASIC_H
