/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef MATRIX_H
#define MATRIX_H

#include <visit-config.h>
#include <math_exports.h>
#include <visitstream.h>

#ifdef DBIO_ONLY 
#include <avtVector.h>
#define STUB_VOID {}
#define STUB_OP {return arg;}
#define STUB_MAT {return avtMatrix();}
#define STUB_OSTR {return ostr;}
#else
class avtVector;
#define STUB_VOID
#define STUB_OP
#define STUB_MAT
#define STUB_OSTR
#endif

// ****************************************************************************
//  Class:  avtMatrix
//
//  Purpose:
//    Encapsulation of a 4x4 matrix.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Oct 16 16:12:32 PST 2001
//    Got rid of vtkMatrix4x4.
//
//    Kathleen Bonnell, Tue Mar  4 09:54:46 PST 2003 
//    Added methods to convert between frames. 
//
//    Eric Brugger, Tue Feb 10 08:28:05 PST 2004
//    Added the optional argument lhs (left handed coordinate system) to
//    MakeTrackball and CreateTrackball.
//
//    Hank Childs, Mon Nov  1 11:17:25 PST 2004
//    I added [Make|Create]Rotation, which were originally coded by Jeremy
//    Meredith for me to help with shadows (and I am checking in).
//
// ****************************************************************************

class MATH_API avtMatrix
{
  public:
    avtMatrix() STUB_VOID;
    avtMatrix(const avtMatrix&) STUB_VOID;
    avtMatrix(const double *) STUB_VOID;
    ~avtMatrix() STUB_VOID;

    // index operators
    double       *operator[](int i);
    const double *operator[](int i) const;

    // assignment operator
    void   operator=(const avtMatrix&) STUB_VOID;

    // multiply matrix*matrix
    avtMatrix operator*(const avtMatrix& arg) const STUB_OP;
    // transform point
    avtVector operator*(const avtVector& arg) const STUB_OP;
    // transform vector
    avtVector operator^(const avtVector& arg) const STUB_OP;

    void   Inverse() STUB_VOID;
    void   Transpose() STUB_VOID;

    // utility
    void   MakeIdentity() STUB_VOID;
    void   MakeZero() STUB_VOID;
    void   MakeTrackball(double,double, double,double, bool lhs=false) STUB_VOID;
    void   MakeTranslate(double, double, double) STUB_VOID;
    void   MakeTranslate(const avtVector&) STUB_VOID;
    void   MakeRBT(const avtVector&, const avtVector&, const avtVector&) STUB_VOID;
    void   MakeRotation(const avtVector&, const avtVector&, const avtVector&) STUB_VOID;
    void   MakeScale(double,double,double) STUB_VOID;
    void   MakeScale(double) STUB_VOID;
    void   MakePerspectiveProjection(double,double, double, double) STUB_VOID;
    void   MakeOrthographicProjection(double, double,double, double) STUB_VOID;
    void   MakeView(const avtVector&, const avtVector&, const avtVector&) STUB_VOID;

    void   MakeFrameToFrameConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &, 
               const avtVector &, const avtVector&, const avtVector&, const avtVector &) STUB_VOID;
    void   MakeCartesianToFrameConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &) STUB_VOID;
    void   MakeFrameToCartesianConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &) STUB_VOID;
    // static utility
    static avtMatrix CreateIdentity() STUB_MAT;
    static avtMatrix CreateZero() STUB_MAT;
    static avtMatrix CreateTrackball(double,double, double,double, bool lhs=false) STUB_MAT;
    static avtMatrix CreateTranslate(double, double, double) STUB_MAT;
    static avtMatrix CreateTranslate(const avtVector&) STUB_MAT;
    static avtMatrix CreateRBT(const avtVector&, const avtVector&, const avtVector&) STUB_MAT;
    static avtMatrix CreateRotation(const avtVector&, const avtVector&, const avtVector&) STUB_MAT;
    static avtMatrix CreateScale(double,double,double) STUB_MAT;
    static avtMatrix CreateScale(double) STUB_MAT;
    static avtMatrix CreatePerspectiveProjection(double,double, double, double) STUB_MAT;
    static avtMatrix CreateOrthographicProjection(double, double,double, double) STUB_MAT;
    static avtMatrix CreateView(const avtVector&, const avtVector&, const avtVector&) STUB_MAT;
    static avtMatrix CreateFrameToFrameConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &, 
               const avtVector &, const avtVector&, const avtVector&, const avtVector &) STUB_MAT;
    static avtMatrix CreateCartesianToFrameConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &) STUB_MAT;
    static avtMatrix CreateFrameToCartesianConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &) STUB_MAT;

    const double *GetElements() const;

    // friends
    friend ostream& operator<<(ostream& ostr,const avtMatrix&) STUB_OSTR;
  private:
    double m[4][4];
};

#undef STUB_VOID
#undef STUB_OP
#undef STUB_MAT
#undef STUB_OSTR

inline double *
avtMatrix::operator[](int i)
{
    return &(m[i][0]);
}

const inline double *
avtMatrix::operator[](int i) const
{
    return &(m[i][0]);
}

const inline double *
avtMatrix::GetElements() const
{
    return (const double *)m;
}

#endif
