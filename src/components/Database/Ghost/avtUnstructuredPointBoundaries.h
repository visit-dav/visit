/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ****************************************************************************
//                      avtUnstructuredPointBoundaries.h
// ****************************************************************************

#ifndef AVT_UNSTRUCTURED_POINT_BOUNDARIES_H
#define AVT_UNSTRUCTURED_POINT_BOUNDARIES_H

#include <database_exports.h>
#include <avtUnstructuredDomainBoundaries.h>

#include <map>
#include <vector>
#include <utility>

using std::vector;
using std::pair;
using std::map;

class vtkDataSet;
class vtkDataArray;
class avtMixedVariable;
class avtMaterial;

// ****************************************************************************
//  Class:  avtUnstructuredPointBoundaries
//
//  Purpose:
//    A domain boundaries for unstructured datasets which constructs
//    the boundaries from the list of shared points. When using this
//    class, only SetSharedPoints needs to be called. It will provide
//    the needed information to SetGivenCellsAndPoints.
//
//  Notes:
//    This class is based on the assumption that ExchangeMesh or
//    ConfirmMesh will be called before any of the other Exchange
//    methods.
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

class DATABASE_API avtUnstructuredPointBoundaries
                                       : public avtUnstructuredDomainBoundaries
{
  public:
                 avtUnstructuredPointBoundaries();
    virtual      ~avtUnstructuredPointBoundaries();

    static void Destruct(void *);
    
    virtual vector<vtkDataSet*>       ExchangeMesh(vector<int>       domainNum,
                                         vector<vtkDataSet*>   meshes);

    virtual bool                      ConfirmMesh(vector<int>      domainNum,
                                        vector<vtkDataSet*> meshes);
    
  protected:
    void                              Generate(vector<int> domainNum,
                                               vector<vtkDataSet*> meshes);

    bool                              CheckGenerated(int d1, int d2);

    vector<vector<bool> >             generated;
};

#endif
