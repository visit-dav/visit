#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "EffectiveCPP.h" // gives us free operator - and operator +
#include <iostream>
#include <vector>
#include <iomanip> 
#include <istream>
#include "RCDebugStream.h"

namespace rclib {
  using namespace std;
  template <class T>
    struct Point {
      /*!
        default constructor
      */ 
      Point() {
          int i=3; 
          while (i--) mXYZ[i] = 0; 
        }
      /*! 
        construct from three elements
      */ 
      Point(T t1, T t2, T t3) {
        mXYZ[0] = t1; 
        mXYZ[1] = t2; 
        mXYZ[2] = t3; 
      }
      /*! 
        Yet another constructor
      */ 
      Point(T t[3]) { int i=3;   while (i--) mXYZ[i] = t[i];  }      
      /*! 
        Yet another constructor -- e.g. to allow Point<float>(100).  Hopefully, does not cause problems with accidental construction per Effective C++.  
      */ 
      Point(T t) {
        int i=3; while (i--) mXYZ[i] = t; 
      } 
      /*! 
        Copy constructor
      */ 
      Point(const Point<T>&p) {
        *this = p; 
      }
      /*!
        Accessor -- assumes xyz is allocated
      */ 
      void Get(float *xyz) const {
        int i=3; while (i--) {
          xyz[i] = mXYZ[i]; 
        }
        return;
      }

      /*!
        Assignment
      */ 
      const Point<T> &operator = (const Point<T> &rhs) {
        if (&rhs != this) {
          int i=3; 
          while (i--) mXYZ[i] =rhs.mXYZ[i]; 
        }
        return *this; 
      }
      /*!
        non-const index
      */ 
        T& operator [](int i)  {
          return mXYZ[i]; 
        }
      /*! 
        const indexing
      */ 
      const T& operator [](int i) const {
        const T *loc = mXYZ + i; 
        return *loc; 
      }
      /*!
        returns true if any of this->mXYZ is greater than corresponding element of other.mXYZ
        else returns false.  This is useful for bounds testing and other operations. 
      */ 
      bool Exceeds(const Point<T> &other) const {
        int i=0; while (i<3) {
          if (mXYZ[i] > other.mXYZ[i]) return true; 
          ++i; 
        }
        return false;
      }
      /*! 
        Compares magnitudes to determine less-than
        \return true or false
      */ 
      bool operator <(const Point<T> &rhs) const {
        T product1=T(0), product2=T(0); 
        int i=3;  while (i--) {
          product1 += mXYZ[i] * mXYZ[i]; 
          product2 += rhs.mXYZ[i] * rhs.mXYZ[i]; 
        }
        return product1 < product2;         
      }
      /*!
        Attempt to normalize the vector to magnitude of 1.0
      */ 
      void Normalize(void) {
        T magnitude = sqrt(mXYZ[0] * mXYZ[0] + mXYZ[1] * mXYZ[1] + mXYZ[2] * mXYZ[2]); 
        if (magnitude > 0) {
          mXYZ[0] /= magnitude;
          mXYZ[1] /= magnitude;
          mXYZ[2] /= magnitude;
        }
        return; 
      }
     /*! 
        Compute magnitude 
      */ 
        T Magnitude(void) const {
        T sum=T(0);
        int i=3;  while (i--) {
          sum += mXYZ[i] * mXYZ[i]; 
        }
        return sqrt(sum);        
      }
     /*! 
        Compares magnitudes to determine equality.  
      */ 
        bool SameMagnitude(const Point<T> &rhs) const {        
        T sum1=T(0), sum2=T(0);
        int i=3;  while (i--) {
          sum1 += mXYZ[i] * mXYZ[i]; 
          sum2 += rhs.mXYZ[i] * rhs.mXYZ[i]; 
        }
        return sum1 == sum2;         
      }

      /*!
        Only equal if they have the same XYZ values
      */ 
      bool operator == (const Point<T>&rhs) const {
        int i=3; while (i--) {
          if (mXYZ[i] != rhs.mXYZ[i]) return false;  
        } 
        return true; 
      }

      /*!
        True if nonzero element exists
      */ 
      bool operator ! () const {
        int i=3; while (i--) {
          if (mXYZ[i] != 0) return false;  
        } 
        return true; 
      }

      /*!
        Negation
      */ 
      const Point<T> operator -() {
        return Point<T>(-mXYZ[0], -mXYZ[1], -mXYZ[2]); 
      }
      
      /*!
        autodecrement
      */ 
      const Point<T>&operator -=(const Point<T> &rhs) {
        int i=3;  while (i--) mXYZ[i] -= rhs.mXYZ[i]; 
        return *this; 
      }
      /*! 
        auto-increment
      */ 
      const Point<T>&operator +=(const Point<T> &rhs) {
        int i=3;  while (i--) mXYZ[i] += rhs.mXYZ[i]; 
        return *this; 
      }
      /*!
        auto-multiply (dot product)
      */
      const Point<T>&operator *=(const Point<T> &rhs) {
        int i=3;  while (i--) mXYZ[i] *= rhs.mXYZ[i]; 
        return *this; 
      }
      /*!
        auto-multiply (scaling)
      */
      const Point<T>&operator *=(const T &rhs) {
        int i=3;  while (i--) mXYZ[i] *= rhs; 
        return *this; 
      }
      /*!
        auto-division
      */ 
      const Point<T>&operator /=(const Point<T> &rhs) {
        int i=3;  while (i--) mXYZ[i] /= rhs.mXYZ[i]; 
        return *this; 
      }

      /*!
        Cross product for vectors
      */ 
      template <class W> 
      friend const Point<W>Cross(const Point<W> &lhs, const Point<W> &rhs);

      /*! 
        convert to string "(x, y, z)" (no end line)
      */ 
      const std::string Stringify(void) const {
        return std::string("(") + doubleToString(mXYZ[0]) + ", " +  doubleToString(mXYZ[1]) + ", " + doubleToString(mXYZ[2]) +")"; 
      }
      /*!
        dump to ASCII file for later retrieval
      */ 
      const std::ostream &UnformattedDump(const std::ostream &stream){
        return stream << mXYZ[0] << " "  << mXYZ[1] << " " << mXYZ[2]; 
      }
      /*!
        retrieve the point from ASCII UnformattedDump
      */ 
      const std::ostream &ReadFromStream(const std::istream &stream){
        return stream >> mXYZ[0] >> mXYZ[1] >> mXYZ[2]; 
      } 
       
      /*!
        The meat of the matter -- kept public for ease of use.  
      */ 
      T mXYZ[3]; 
    }; // end template struct Point
  template <class T> 
    const Point<T> operator +(const Point<T>&lhs, const Point<T>&rhs) {
    Point<T> result(lhs); 
    return result += rhs; 
  }
  
  template <class T> 
    const Point<T> operator -(const Point<T>&lhs, const Point<T>&rhs) {
    Point<T> result(lhs); 
    return result -= rhs; 
  }
  
  template <class T> 
    const Point<T> operator *(const Point<T>&lhs, const Point<T>&rhs) {
    Point<T> result(lhs); 
    return result *= rhs; 
  }
  
  template <class T> 
    const Point<T> operator *(const T &lhs, const Point<T>&rhs) {
    Point<T> result(rhs); 
    return result * lhs; 
  }

  template <class T> 
    const Point<T> operator *(const Point<T> &lhs, const T &rhs) {
    Point<T> result(lhs); 
    return result *= rhs; 
  }
  
  template <class T> 
    const Point<T> operator /(const Point<T>&lhs, const Point<T>&rhs) {
    Point<T> result(lhs); 
    return result /= rhs; 
  }

 template <class T> 
    bool operator > (const Point<T>& lhs, const Point<T> &rhs) {
   return !(lhs < rhs) && !(lhs.SameMagnitude(rhs)); 
  } 
  
  template <class T> 
    bool operator != (const Point<T>&lhs, const Point<T>&rhs) {
    return !(lhs == rhs); 
  }
 
  /*!
    Cross product for vectors
  */ 
  template <class T>
  const Point<T> Cross(const Point<T> &lhs, const Point<T> &rhs){
    Point<T> result
      (lhs.mXYZ[1] * rhs.mXYZ[2] - lhs.mXYZ[2] * rhs.mXYZ[1], 
       lhs.mXYZ[2] * rhs.mXYZ[0] - lhs.mXYZ[0] * rhs.mXYZ[2],  
       lhs.mXYZ[0] * rhs.mXYZ[1] - lhs.mXYZ[1] * rhs.mXYZ[0]); 
    return result; 
  }

  /*!
    InBounds returns true if p1 is in the space with given min/max extents
    \param p1 the point to test
    \param min the minimum of the subspace
    
  */ 
  template <class T>
  bool InBounds(const Point<T>p1, 
                const Point<T>min, const Point<T>max){
    if (min.Exceeds(p1) || p1.Exceeds(max)) return false;         
    return true; 
  }


  // ========================================================================
  // ***********************************************************************

  // POINT ROTATION:  The following functions take a vector of Points and rotate them from "oldOrientation" to "newOrientation".  For example, if oldOrientation = (0 0 1) and newOrientation = (0 1 0), this means that any point <x y z> in pointsToRotate will be rotated by the same set of rotations around the origin that would move (0 0 1) to (0 1 0), which boils down to a 270° rotation around the X axis. 
  // -----------------------------------------------------------------------

  // =======================================================================
  // RESTRICTED CASE (FAST): If you are just switching from orientations along axes, it's fast to do so without any matrix math, and that's what this does.  Returns false if the switch could not be performed, true if it could.  
  template <class T>
    bool AxisSwitch(Point<T> &oldOrientation, Point<T> &newOrientation, 
                    vector<Point<T> > &pointsToRotate) {
    cerr << "AxisSwitch" <<endl;
    int axisNum=3, oldAxis = -1, newAxis = -1; 
    while (axisNum--) {
      if (oldOrientation[axisNum] != 0) {
        if (oldAxis != -1) {
          //    debug2 << "AxisSwitch: oldOrientation is not aligned with an axis" << endl; 
          return false;
        }
        oldAxis = axisNum; 
      }
      if (newOrientation[axisNum] != 0) {
        if (newAxis != -1)  {
          rcdebug2 << "AxisSwitch: newOrientation is not aligned with an axis" << endl; 
          return false; 
        }
        newAxis = axisNum;
      }
    }
    if (newAxis == -1 || oldAxis == -1) {
      rcdebug2 << "AxisSwitch: One or both orientations do not align with an axis" << endl; 
      return false; 
    }
    // So we're just switching axes, which is a trivial transformation computationally: 
    rcdebug1 << "AxisSwitch: old and new orientations are aligned with an axis, so transforming with simple axis rotation" << endl; 
    int shft = newAxis - oldAxis; // spin in direction of axis "rotation"
    // declaring vector<Point<T> >::iterator is to make a declaration that itself depends on a template, so must use "typename" keyword to convince the compiler that I'm really declaring a type.  See Stroustrop sec 13.5, p 857
    typename vector<Point<T> >::iterator pos = pointsToRotate.begin(), endpos = pointsToRotate.end();
    Point<T> tmpPt; 
    bool firsttime = true; 
    while (pos != endpos) {
      int axis = 3, newaxis; 
      while (axis--) {     
        newaxis=(axis+shft+3)%3;
        tmpPt[newaxis] = (*pos)[axis]; 
      }
      firsttime = false; 
      rcdebug5 << "Point " << pos->Stringify() << " rotated to " << tmpPt.Stringify() << endl; 
      
      *pos = tmpPt; 
      
      ++pos; 
    }
    return true; 
    
  }
  
  // ======================================================================
  // GENERAL CASE (SLOWER, USES MATRICES): Performs rotations as described above in "POINT ROTATIONS" note. 
  template <class T>
    void RotatePoints(Point<T> oldOrientation, Point<T> newOrientation, 
                        vector<Point<T> > &pointsToRotate) {
    
    rcdebug2 << "RotateCylinder( " << oldOrientation.Stringify() << " ---> " << newOrientation.Stringify() << " )" << endl; 
    
    if (!oldOrientation || !newOrientation) {
      string err = string ("Error:  oldOrientation and newOrientation must both be nonzero, but oldOrientation is ") + oldOrientation.Stringify() + " and newOrientation is " + newOrientation.Stringify(); 
      throw err; 
    }
    if (oldOrientation == newOrientation) {
      rcdebug2 <<"newOrientation == oldOrientation, nothing to do" << endl; 
      return; 
    }
    
    // First, if both old and new orientations are aligned along axes, then it's very simple: 
    if  (AxisSwitch(oldOrientation, newOrientation, pointsToRotate)) {
      rcdebug2 << "RotateCylinder done: AxisSwitch succeeded" << endl;
      return;
    }
    
    // OK, so we are not so lucky, have to do the orientation
    
    // first, move the oldOrientation to the Z axis:
    oldOrientation.Normalize(); 
    newOrientation.Normalize(); 
    //debug5 << "After normalizing, oldOrientation is " << oldOrientation.Stringify() << " and newOrientation is " << newOrientation.Stringify()  << endl; 
    // make sure we're not already on the z axis: 
    bool rotateToZ = false; 
    if (oldOrientation[0] != 0 || oldOrientation[1] != 0) {
      // We need to apply rotations to bring the data in alignment with the Z axis.
      rotateToZ = true; 
    } else { 
      //debug5 << "oldOrientation is already along the Z axis, so initial rotation is not necessary" << endl; 
    }
    
    float aOld=oldOrientation[0], 
      bOld=oldOrientation[1], 
      cOld=oldOrientation[2], 
      dOld = sqrt(oldOrientation[1]*oldOrientation[1] + oldOrientation[2]*oldOrientation[2]), 
      aNew=newOrientation[0], 
      bNew=newOrientation[1], 
      cNew=newOrientation[2],
      dNew = sqrt(newOrientation[0]*newOrientation[0] + newOrientation[2]*newOrientation[2]), 
      eNew = sqrt(newOrientation[1]*newOrientation[1] + newOrientation[2]*newOrientation[2]);
    //debug5 << "aOld = " << aOld << ", bOld = " << bOld << ", cOld = " << cOld << endl; 
    //debug5 << "aNew = " << aNew << ", bNew = " << bNew << ", cNew = " << cNew << ", dNew = " << dNew << endl; 
  
    typename vector<Point<T> >::iterator pos = pointsToRotate.begin(), endpos = pointsToRotate.end();
    Point<T> tmpPt; 
    while (pos != endpos) {
      if (rotateToZ) {
        //debug5 << "rotate to z axis: Point " << pos->Stringify() << "with magnitude " << pos->Magnitude(); 
        // from Computer Graphics, p 417
        // rotate around X axis into YZ plane:
        tmpPt[0] = (*pos)[0];
        tmpPt[1] = (*pos)[1]*cOld/dOld - (*pos)[2]*bOld/dOld;
        tmpPt[2] = (*pos)[1]*bOld/dOld + (*pos)[2]*cOld/dOld;      
        // rotate around Y axis onto Z axis:
        (*pos)[0] = tmpPt[0]*dOld - tmpPt[2]*aOld; 
        (*pos)[1] = tmpPt[1]; 
        (*pos)[2] = tmpPt[0]*aOld + tmpPt[2]*dOld; 
        //debug5 << " rotated to " << pos->Stringify() << "with magnitude " << pos->Magnitude() << endl; 
      } 
      // now take from Z axis to new orientation.  Note that this is the opposite of the rotations performed in Computer Graphics text.  
      //debug5 << "rotate to new orientation: Point " << pos->Stringify() << " with magnitude " << pos->Magnitude(); 
      
      //rotate from Z axis around X axis into YZ plane:
      tmpPt[0] = (*pos)[0];
      tmpPt[1] = (*pos)[1]*dNew + (*pos)[2]*bNew;
      tmpPt[2] = -(*pos)[1]*bNew + (*pos)[2]*dNew;
      //debug5 << " rotated around X axis: " << tmpPt.Stringify() << " with magnitude " << tmpPt.Magnitude(); 
      
      // Now do the "first" rotation around Y to take us to the target orientation:
      if (0 && cNew == 0) {
        //rotate around Z now, because if cNew == 0, then the first rotation put us right on top of the X axis, so the 2nd rotation will be undefined
        //debug5 << " rotate around Z axis " ; 
        (*pos)[0] = tmpPt[0]*dNew - tmpPt[1]*bNew;
        (*pos)[1] = tmpPt[0]*bNew + tmpPt[1]*dNew;
        (*pos)[2] = tmpPt[2];
      } else  {
        // rotate around Y axis into new orientation:
        //debug5 << " rotate around Y axis " ; 
        (*pos)[0] = tmpPt[0]*cNew/dNew + tmpPt[2]*aNew/dNew;
        (*pos)[1] = tmpPt[1];
        (*pos)[2] = -tmpPt[0]*aNew/dNew + tmpPt[2]*cNew/dNew;
      }
      
      //debug5 << " to " << pos->Stringify() << " with magnitude " << pos->Magnitude() << endl; 
      
      ++pos; 
    }  
    
    return ;
  }
  
};//end namespace rclib

#endif
