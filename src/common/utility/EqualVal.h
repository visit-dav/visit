#ifndef EQUALVAL_H
#define EQUALVAL_H

#include <vector>

// ****************************************************************************
//  Class:  EqualVal 
//
//  Purpose:
//    Comparison functions
//
//  Programmer:  Mark C. Miller 
//  Creation:    06May03 
//
// ****************************************************************************
template <class T>
struct EqualVal
{
    inline static bool EqualScalar(void *a1,void *a2);
    inline static bool EqualArray (void *a1,void *a2, int l);
    inline static bool EqualVector(void *a1,void *a2);
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//                               Inline Methods
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Methods:  EqualVal<T>::Equal*
//
//  Purpose:
//    Methods for comparison.
//
//  Arguments:
//    a1         the pointer to the first operand of comparison
//    a2         the pointer to the second operand of comparison 
//    l          (where applicable) the length of the array
//
//  Programmer:  Mark C. Miller (cut/paste/modify from Jeremey's stuff)
//  Creation:    06May03 
//
// ****************************************************************************
template <class T>
bool
EqualVal<T>::EqualScalar(void *a1_, void *a2_)
{
    T *a1  = (T*)a1_;
    T *a2  = (T*)a2_;
    if (*a1 == *a2)
       return true;
    else
       return false;
}

template <class T>
bool
EqualVal<T>::EqualArray(void *a1_, void *a2_, int l)
{
    if (a1_ == a2_)
       return true;
    T *a1  = (T*)a1_;
    T *a2  = (T*)a2_;
    for (int i=0; i<l; i++)
    {
       if (a1[i] != a2[i])
          return false;
    }
    return true;
}

template <class T>
bool
EqualVal<T>::EqualVector(void *a1_, void *a2_)
{
    if (a1_ == a2_)
       return true;
    std::vector<T> &a1  = *(std::vector<T>*)a1_;
    std::vector<T> &a2  = *(std::vector<T>*)a2_;
    int l1 = a1.size();
    int l2 = a2.size();
    if (l1 != l2)
        return false;
    else
    {
        for (int i=0; i<l1; i++)
        {
           if (a1[i] != a2[i])
              return false;
        }
    }
    return true;
}

// ****************************************************************************
//  Method:  EqualVal<AttributeGroup*>::EqualVector
//
//  Purpose:
//    Specialized method for equality of Attribute Group Vectors.
//
//  Arguments:
//    a1         the pointer to the first  operand of comparsion 
//    a2         the pointer to the second operand of comparison 
//
//  Programmer:  Mark C. Miller 
//  Creation:    06May03 
//
// ****************************************************************************
template<>
bool
EqualVal<AttributeGroup*>::EqualVector(void *a1_, void *a2_)
{
    if (a1_ == a2_)
       return true;
    AttributeGroupVector &a1 = *(AttributeGroupVector*)a1_;
    AttributeGroupVector &a2 = *(AttributeGroupVector*)a2_;
    int l1 = a1.size();
    int l2 = a2.size();
    if (l1 != l2)
       return false;
    for (int i=0; i<l1; i++)
    {
        if (!a1[i]->EqualTo(a2[i]))
           return false;
    }
    return true;
}

#endif
