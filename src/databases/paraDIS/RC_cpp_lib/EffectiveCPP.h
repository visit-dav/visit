/* tricks from Effective C++ class */ 

#ifndef EFFECTIVE_CPP_H
#define EFFECTIVE_CPP_H

/*!
  This used to contain template operators such as the following: 
  template <class T>
  const T& operator *(const T&other) {}
  
  however, this caused problems for any class that had its own operator *() for example.  So I moved them to where I specifically needed them, which was in the Point class, it turns out. 
*/ 
namespace RC_Math {
  
}

#endif
