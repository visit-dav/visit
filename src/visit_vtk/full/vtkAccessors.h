/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#ifndef VTK_ACCESSORS_H
#define VTK_ACCESSORS_H

#include <vtkType.h>
#include <vtkPoints.h>
#include <vtkDataArray.h>

//
// Functors that let us retrieve points quickly.
//

// ****************************************************************************
// Class: vtkPointAccessor
//
// Purpose:
//   This class gets the i'th point from a vtkPoints object.
//
// Notes:      Use faster direct array accesses
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 12 16:17:58 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

template <typename T>
class vtkPointAccessor
{
public:
    vtkPointAccessor(vtkPoints *p) : pts((const T *)p->GetVoidPointer(0)) { };
    inline void GetPoint(vtkIdType id, double pt[3]) const
    {
        const T *ptr = pts + 3 * id;
        pt[0] = double(ptr[0]);
        pt[1] = double(ptr[1]);
        pt[2] = double(ptr[2]);
    }
private:
    const T *pts;
};

// ****************************************************************************
// Class: vtkGeneralPointAccessor
//
// Purpose:
//   This class gets the i'th point from a vtkPoints object.
//
// Notes:      Use slower GetPoint method.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 12 16:17:58 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

class vtkGeneralPointAccessor
{
public:
    vtkGeneralPointAccessor(vtkPoints *p) : pts(p) { };
    inline void GetPoint(vtkIdType id, double pt[3]) const
    {
        return pts->GetPoint(id, pt);
    }
private:
    vtkPoints *pts;
};

// ****************************************************************************
// Class: vtkRectPointAccessor
//
// Purpose:
//   Access 3 different vtkDataArrays using 1 rectilinear cell index to make
//   the final point.
//
// Notes:      Uses faster direct array access.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 12 16:10:26 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

template <typename T>
class vtkRectPointAccessor
{
public:
    vtkRectPointAccessor(const int *d, vtkDataArray *Xc, vtkDataArray *Yc, vtkDataArray *Zc)
    {
        dims[0] = d[0]; dims[1] = d[1]; dims[2] = d[2];
        X = (const T *)Xc->GetVoidPointer(0);
        Y = (const T *)Yc->GetVoidPointer(0);
        Z = (const T *)Zc->GetVoidPointer(0);
        dims01 = dims[0] * dims[1];
    }

    inline void GetPoint(vtkIdType index, double pt[3]) const
    {
        vtkIdType cellI = index % dims[0];
        vtkIdType cellJ = (index/dims[0]) % dims[1];
        vtkIdType cellK = index/dims01;
        pt[0] = X[cellI];
        pt[1] = Y[cellJ];
        pt[2] = Z[cellK];
    }

    inline void GetPoint(vtkIdType cellI, vtkIdType cellJ, vtkIdType cellK, double pt[3]) const
    {
        pt[0] = X[cellI];
        pt[1] = Y[cellJ];
        pt[2] = Z[cellK];
    }
private:
    vtkIdType  dims[3], dims01;
    const T   *X, *Y, *Z;
};

// ****************************************************************************
// Class: vtkGeneralRectPointAccessor
//
// Purpose:
//   Access 3 different vtkDataArrays using 1 rectilinear cell index to make
//   the final point.
//
// Notes:      Uses slower GetTuple methods.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 12 16:10:26 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

class vtkGeneralRectPointAccessor
{
public:
    vtkGeneralRectPointAccessor(const int *d, vtkDataArray *Xc, vtkDataArray *Yc, vtkDataArray *Zc)
    {
        dims[0] = d[0]; dims[1] = d[1]; dims[2] = d[2];
        X = Xc;
        Y = Yc;
        Z = Zc;
        dims01 = dims[0] * dims[1];
    }

    inline void GetPoint(vtkIdType index, double pt[3]) const
    {
        vtkIdType cellI = index % dims[0];
        vtkIdType cellJ = (index/dims[0]) % dims[1];
        vtkIdType cellK = index/dims01;
        pt[0] = X->GetTuple1(cellI);
        pt[1] = Y->GetTuple1(cellJ);
        pt[2] = Z->GetTuple1(cellK);
    }

    inline void GetPoint(vtkIdType cellI, vtkIdType cellJ, vtkIdType cellK, double pt[3]) const
    {
        pt[0] = X->GetTuple1(cellI);
        pt[1] = Y->GetTuple1(cellJ);
        pt[2] = Z->GetTuple1(cellK);
    }
private:
    vtkIdType     dims[3], dims01;
    vtkDataArray *X, *Y, *Z;
};

// ****************************************************************************
// Class: vtkAccessor
//
// Purpose: 
//   Access vtkDataArray using direct memory accesses.
//
// Notes:      We could always add a generic GetTuple to deal with multi-
//             component arrays. I just didn't need it yet.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 29 13:10:20 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

template <typename T>
class vtkAccessor
{
public:
    vtkAccessor(T *da, bool ownIt = false) : arr(da), own(ownIt)
    {        
    }

    vtkAccessor(vtkDataArray *da) : arr((T *)da->GetVoidPointer(0)), own(false)
    {        
    }

    ~vtkAccessor()
    {
        if(own && arr != NULL)
            delete [] arr;
    }

    inline T GetTuple1(vtkIdType id) const
    {
        return arr[id];
    }

    inline void SetTuple1(vtkIdType id, T val)
    {
        arr[id] = val;
    }
private:
    T   *arr;
    bool own;
};

// ****************************************************************************
// Class: vtkGeneralAccessor
//
// Purpose:
//   Access vtkDataArray via GetTuple1 calls.
//
// Notes:      Use regular GetTuple methods to access data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 29 13:11:41 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

class vtkGeneralAccessor
{
public:
    vtkGeneralAccessor(vtkDataArray *da) : arr(da)
    {
    }

    inline double GetTuple1(vtkIdType id) const
    {
        return arr->GetTuple1(id);
    }

    inline void SetTuple1(vtkIdType id, double val)
    {
        arr->SetTuple1(id, val);
    }
private:
    vtkDataArray *arr;
};

#endif
