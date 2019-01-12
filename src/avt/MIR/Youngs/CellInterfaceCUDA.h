// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France


#ifndef __CUDA_NEC_H


#ifndef __CUDACC__ // compiling with host compiler (gcc, icc, etc.)

#ifndef FUNC_DECL
#define FUNC_DECL static inline
#endif

#ifndef KERNEL_DECL
#define KERNEL_DECL /* exported function */
#endif

#ifndef CONSTANT_DECL
#define CONSTANT_DECL static const
#endif

#ifndef REAL_PRECISION
#define REAL_PRECISION 64 // defaults to 64 bits floating point
#endif

#else // compiling with cuda

#ifndef FUNC_DECL
#define FUNC_DECL __device__
#endif

#ifndef KERNEL_DECL
#define KERNEL_DECL __global__
#endif

#ifndef CONSTANT_DECL
#define CONSTANT_DECL __constant__
#endif

#ifndef REAL_PRECISION
#define REAL_PRECISION 32 // defaults to 32 bits floating point
#endif

#endif // __CUDACC__


#include "CellInterfaceTypes.h"      // definit REAL en float, double ou long double
#include "CellInterfaceMath.h"

#endif

