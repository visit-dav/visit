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

#define UVF_INVALID (std::numeric_limits<UINT64>::max())
#include "LargeRAWFile.h"
class DataBlock;

#endif // UVFBASIC_H
