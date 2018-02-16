/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
// avtBlueprintDataAdaptor.h
// ************************************************************************* //

#ifndef AVT_BLUEPRINT_DATA_ADAPTOR_H
#define AVT_BLUEPRINT_DATA_ADAPTOR_H

//-----------------------------------------------------------------------------
// conduit includes
//-----------------------------------------------------------------------------
#include "conduit.hpp"

//-----------------------------------------------------------------------------
// mfem includes
//-----------------------------------------------------------------------------
#include "mfem.hpp"


//-----------------------------------------------------------------------------
// vtk forward decls
//-----------------------------------------------------------------------------
class vtkDataSet;
class vtkDataArray;

//-----------------------------------------------------------------------------
//  Class: avtBlueprintDataAdaptor
//
//  Purpose:
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Dec 27 14:14:40 PST 2016
//
//-----------------------------------------------------------------------------
class avtBlueprintDataAdaptor
{
public:
    
  /// Helpers for converting Mesh and Field Blueprint conforming data
  /// to vtk instances.
  class VTK
  {
    public:
      static vtkDataSet*    MeshToVTK(const conduit::Node &mesh);
      static vtkDataArray*  FieldToVTK(const conduit::Node &field);

  };

  /// Helpers for converting Mesh and Field Blueprint conforming data
  /// to mfem instances + helpers for refining mfem data to vtk. 
  class MFEM
  {
    public:
    //-------------------------------------------------------------------------
    // blueprint to mfem 
    //-------------------------------------------------------------------------
    static mfem::Mesh         *MeshToMFEM(const conduit::Node &mesh);

    static mfem::GridFunction *FieldToMFEM(mfem::Mesh *mesh, 
                                           const conduit::Node &field);
    //-------------------------------------------------------------------------
    // mfem to vtk 
    //-------------------------------------------------------------------------
    static vtkDataSet   *RefineMeshToVTK(mfem::Mesh *mesh,
                                             int lod);

    static vtkDataArray *RefineGridFunctionToVTK(mfem::Mesh *mesh,
                                                     mfem::GridFunction *gf,
                                                     int lod);

    static vtkDataArray *RefineElementColoringToVTK(mfem::Mesh *mesh,
                                                        int lod);

    static vtkDataArray *RefineElementAttributeToVTK(mfem::Mesh *mesh,
                                                     int lod);
  };
};


#endif
