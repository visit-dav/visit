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

// ************************************************************************* //
//                            avtEulerianQuery.h                             //
// ************************************************************************* //

#ifndef AVT_EULERIAN_QUERY_H
#define AVT_EULERIAN_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <string>
#include <map>


class vtkDataSet;
class vtkGeometryFilter;



// ****************************************************************************
//  Class: avtEulerianQuery
//
//  Purpose:
//    A query that calculates the Eulerian number for the mesh.
//
//  Programmer: Akira Haddox
//  Creation:   June 28, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 12:37:11 PST 2002  
//    Moved from avtEulerianFilter, made to fit query model. 
//
//    Brad Whitlock, Fri Nov 22 11:00:01 PDT 2002
//    Introduced a typedef to make the Windows compiler happier. Added API.
//
//    Kathleen Bonnell, Fri Sep  3 10:10:28 PDT 2004 
//    Added VerifyInput. 
//
// ****************************************************************************

class QUERY_API avtEulerianQuery : public avtDatasetQuery
{
  public:
                              avtEulerianQuery();
    virtual                  ~avtEulerianQuery();


    virtual const char       *GetType(void)   { return "avtEulerianQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating Eulerian of mesh."; };

  protected:

    // Structure to represent an edge
    // Stores two node ids: order is not important.
    // However, we store them sorted, so that edge(0,1) == edge(1,0)
    // Note that < is a strict weak ordering to work with set, such that
    // Equiv(a,b) is defined as !(a<b || b<a)
    struct edgepair
    {
        int myGreater, myLesser;
        edgepair(int _a, int _b)
        {
            if (_a < _b)
            {
                myLesser = _a;
                myGreater = _b;
            }
            else
            {
                 myLesser = _b;
                 myGreater = _a;
            }
         }
 
        bool
        operator < (const edgepair &e) const
        {
             return myGreater == e.myGreater ?
                    myLesser < e.myLesser : myGreater < e.myGreater;
        }
    };

    typedef std::map<int, int> DomainToEulerMap;

    vtkGeometryFilter              *gFilter;
    DomainToEulerMap                domToEulerMap;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    VerifyInput(void);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
};


#endif
