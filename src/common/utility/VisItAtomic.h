#ifndef VISIT_ATOMIC_H
#define VISIT_ATOMIC_H

// Should add the cstdatomic values too.
//#include <cstdatomic>

// Could be overridden below.
#define VISIT_ATOMIC_INLINE inline

// Type of value used for the atomic functions.
#if   defined(VISIT_ATOMIC_USE_GCC_ATOMIC) || defined(VISIT_ATOMIC_USE_ICC_ATOMIC)
    typedef volatile int VISIT_ATOMIC_TYPE;
    typedef int VISIT_ATOMIC_AMOUNT;

#elif defined(VISIT_ATOMIC_USE_WIN32_ATOMIC)
    #include <windows.h>
    #include <intrin.h>

    #pragma intrinsic (_InterlockedIncrement)
    #define InterlockedIncrement _InterlockedIncrement
    #pragma intrinsic (_InterlockedDecrement)
    #define InterlockedDecrement _InterlockedDecrement
    #pragma intrinsic(_InterlockedAdd)
    #define InterlockedAdd _InterlockedAdd

    typedef volatile LONG VISIT_ATOMIC_TYPE;
    typedef LONG VISIT_ATOMIC_AMOUNT;

#elif defined(VISIT_ATOMIC_USE_MAC32_ATOMIC)
    #include <libkern/OSAtomic.h>

    typedef volatile int32_t VISIT_ATOMIC_TYPE;
    typedef int32_t VISIT_ATOMIC_AMOUNT;

#elif defined(VISIT_ATOMIC_USE_MIPOSPRO_ATOMIC)
    typedef volatile unsigned int VISIT_ATOMIC_TYPE;
    typedef int VISIT_ATOMIC_AMOUNT;

#elif defined(VISIT_ATOMIC_USE_SUN_ATOMIC)
    typedef volatile uint_t ATOMIC_TYPE;
    typedef int ATOMIC_AMOUNT;

#elif defined(VISIT_ATOMIC_USE_MUTEX)
    #include "vtkCriticalSection.h"

    typedef int VISIT_ATOMIC_TYPE;
    typedef int VISIT_ATOMIC_AMOUNT;

#else
    typedef int VISIT_ATOMIC_TYPE;
    typedef int VISIT_ATOMIC_AMOUNT;
#endif

#if defined(VISIT_ATOMIC_NO_INLINE)
    #define VISIT_ATOMIC_INLINE
#endif

// ****************************************************************************
//  Method: AtomicInc
//
//  Arguments:
//      _value  The value to be incremented by 1.
//
//  Purpose:
//      Performs an atomic increment operation on the specified value.
//
//  Programmer: David Camp
//  Creation:   March 6, 2013
//
// ****************************************************************************

static VISIT_ATOMIC_INLINE VISIT_ATOMIC_TYPE AtomicInc(VISIT_ATOMIC_TYPE &_value)
{
#if   defined(VISIT_ATOMIC_USE_GCC_ATOMIC) || defined(VISIT_ATOMIC_USE_ICC_ATOMIC)
    return( __sync_add_and_fetch(&_value, 1) );

#elif defined(VISIT_ATOMIC_USE_WIN32_ATOMIC)
    return( InterlockedIncrement(&_value) );

#elif defined(VISIT_ATOMIC_USE_MAC32_ATOMIC)
    return( OSAtomicIncrement32(&_value) );

#elif defined(VISIT_ATOMIC_USE_MIPOSPRO_ATOMIC)
    return( __add_and_fetch(&_value, 1) );

#elif defined(VISIT_ATOMIC_USE_SUN_ATOMIC)
    return( atomic_inc_uint_nv(&_value) );

#elif defined(VISIT_ATOMIC_USE_MUTEX)
    static vtkSimpleCriticalSection AtomicIncCritSec;

    AtomicIncCritSec.Lock();
    ++_value;
    AtomicIncCritSec.Unlock();

    return( _value );

#else
    return( ++_value );
#endif
}

// ****************************************************************************
//  Method: AtomicAdd
//
//  Arguments:
//      _value  The value to be incremented by the _amount variable.
//      _amount The amount to increment the value by.
//
//  Purpose:
//      Performs an atomic addition operation on the specified value.
//
//  Programmer: David Camp
//  Creation:   March 6, 2013
//
// ****************************************************************************

static VISIT_ATOMIC_INLINE VISIT_ATOMIC_TYPE AtomicAdd(VISIT_ATOMIC_TYPE &_value, VISIT_ATOMIC_AMOUNT _amount)
{
#if   defined(VISIT_ATOMIC_USE_GCC_ATOMIC)
    return( __sync_add_and_fetch(&_value, _amount) );

#elif defined(VISIT_ATOMIC_USE_WIN32_ATOMIC)
    return( InterlockedAdd(&_value, _amount) );

#elif defined(VISIT_ATOMIC_USE_MAC32_ATOMIC)
    return( OSAtomicAdd32(_amount, &_value) );

#elif defined(VISIT_ATOMIC_USE_MIPOSPRO_BUILTINS)
    return( __add_and_fetch(&_value, _amount) );

#elif defined(VISIT_ATOMIC_USE_SUN_ATOMIC)
    return( atomic_add_int_nv(&_value, _amount) );

#elif defined(VISIT_ATOMIC_USE_MUTEX)
    static vtkSimpleCriticalSection AtomicIncCritSec;

    AtomicIncCritSec.Lock();
    _value += _amount;
    AtomicIncCritSec.Unlock();

    return( _value );
#else
    return( _value += _amount );
#endif
}


// ****************************************************************************
//  Method: AtomicDec
//
//  Arguments:
//      _value  The value to be decremented by 1.
//
//  Purpose:
//      Performs an atomic subtraction operation on the specified value.
//
//  Programmer: David Camp
//  Creation:   March 6, 2013
//
// ****************************************************************************

static VISIT_ATOMIC_INLINE VISIT_ATOMIC_TYPE AtomicDec(VISIT_ATOMIC_TYPE &_value)
{
#if   defined(VISIT_ATOMIC_USE_GCC_ATOMIC) || defined(VISIT_ATOMIC_USE_ICC_ATOMIC)
    return( __sync_sub_and_fetch(&_value, 1) );

#elif defined(VISIT_ATOMIC_USE_WIN32_ATOMIC)
    return( InterlockedDecrement(&_value) );

#elif defined(VISIT_ATOMIC_USE_MAC32_ATOMIC)
    return( OSAtomicDecrement32(&_value) );

#elif defined(VISIT_ATOMIC_USE_MIPOSPRO_ATOMIC)
    return( __sub_and_fetch(&_value, 1) );

#elif defined(VISIT_ATOMIC_USE_SUN_ATOMIC)
    return( atomic_dec_uint_nv(&_value) );

#elif defined(VISIT_ATOMIC_USE_MUTEX)
    static vtkSimpleCriticalSection AtomicIncCritSec;

    AtomicIncCritSec.Lock();
    --_value;
    AtomicIncCritSec.Unlock();

    return( _value );

#else
    return( --_value );
#endif
}

// ****************************************************************************
//  Method: AtomicSub
//
//  Arguments:
//      _value  The value to be decremented by the _amount variable.
//      _amount The amount to decrement the value by.
//
//  Purpose:
//      Performs an atomic subtraction operation on the specified value.
//
//  Programmer: David Camp
//  Creation:   March 6, 2013
//
// ****************************************************************************

static VISIT_ATOMIC_INLINE VISIT_ATOMIC_TYPE AtomicSub(VISIT_ATOMIC_TYPE &_value, VISIT_ATOMIC_AMOUNT _amount)
{
#if   defined(VISIT_ATOMIC_USE_GCC_ATOMIC) || defined(VISIT_ATOMIC_USE_ICC_ATOMIC)
    return( __sync_sub_and_fetch(&_value, _amount) );

#elif defined(VISIT_ATOMIC_USE_WIN32_ATOMIC)
    return( InterlockedAdd(&_value, (-1 * _amount) );

#elif defined(VISIT_ATOMIC_USE_MAC32_ATOMIC)
    return( OSAtomicAdd32((-1 * _amount), &_value) );

#elif defined(VISIT_ATOMIC_USE_MIPOSPRO_BUILTINS)
    return( __add_and_fetch(&_value, (-1 *_amount) );

#elif defined(VISIT_ATOMIC_USE_SUN_ATOMIC)
    return( atomic_sub_int_nv(&_value, _amount) );

#elif defined(VISIT_ATOMIC_USE_MUTEX)
    static vtkSimpleCriticalSection AtomicIncCritSec;

    AtomicIncCritSec.Lock();
    _value -= _amount;
    AtomicIncCritSec.Unlock();

    return( _value );
#else
    return( _value -= _amount );
#endif
}


#endif // VISIT_ATOMIC_H
