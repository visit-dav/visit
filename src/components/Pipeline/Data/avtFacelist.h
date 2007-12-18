/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               avtFacelist.h                               //
// ************************************************************************* //

#ifndef AVT_FACELIST_H
#define AVT_FACELIST_H
#include <pipeline_exports.h>


#include <vector>

#include <vtkSystemIncludes.h>  // for vtkIdType

class   vtkPoints;
class   vtkPolyData;
class   vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtFacelist
//
//  Purpose:
//      Contains a facelist for a domain.
//
//  Note:       This class was modelled after the struct DBfacelist from the
//              Silo library.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Aug  9 17:55:51 PDT 2001
//    Made nodelist be vtkIdType* instead of int* to match VTK 4.0 API
//    for insertion of cells.
//
//    Hank Childs, Mon Jul  1 20:13:33 PDT 2002
//    Added a bail-out option for when original zone numbers are not provided.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtFacelist
{
  public:
                           avtFacelist(int *, int, int, int *,int *,int *,int);
    virtual               ~avtFacelist();

    static void            Destruct(void *);

    void                   CalcFacelist(vtkUnstructuredGrid *, vtkPolyData *);
    void                   CalcFacelistFromPoints(vtkPoints *, vtkPolyData *);

    bool                   CanCalculateZonalVariables(void)
                               { return zones != NULL; };

  protected:
    vtkIdType             *nodelist;
    int                    lnodelist;
    int                    nshapes;
    int                    nfaces;
    int                   *shapecnt;
    int                   *shapesize;
    int                   *zones;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtFacelist(const avtFacelist &) {;};
    avtFacelist         &operator=(const avtFacelist &) { return *this; };
};


// ****************************************************************************
//  Class: avtMultiFacelist
//
//  Purpose:
//      Contains facelists for many domains.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
// ****************************************************************************

class PIPELINE_API avtMultiFacelist
{
  public:
                             avtMultiFacelist(int);
    virtual                 ~avtMultiFacelist();

    bool                     Valid();
    void                     SetDomain(avtFacelist *, int);

    void                     GetDomainsList(std::vector<int> &);

    void                     Calculate(void);
    void                     CalcFacelist(vtkUnstructuredGrid *, int,
                                          vtkPolyData *);

  protected:
    avtFacelist            **fl;
    bool                    *validFL;
    int                      numDomains;
};


#endif


