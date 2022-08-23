/*****************************************************************************
*
* Copyright (c) 2000 - 2022, Lawrence Livermore National Security, LLC
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

#ifndef AVT_CONDUIT_BLUEPRINT_DATA_ADAPTOR_H
#define AVT_CONDUIT_BLUEPRINT_DATA_ADAPTOR_H
#include <avtblueprint_exports.h>
#include <conduit.hpp>

//-----------------------------------------------------------------------------
// mfem includes
//-----------------------------------------------------------------------------
#include <mfem.hpp>

//-----------------------------------------------------------------------------
// vtk forward decls
//-----------------------------------------------------------------------------
class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
//  Class:  avtConduitBlueprintDataAdaptor
//
//  Purpose:
//    Helper methods that convert between Conduit Blueprint and VTK meshes.
//
//  Programmer:  Cyrus Harrison
//  Creation:    May 31, 2022
//
//  Modifications to Blueprint Data Adaptor (now deprecated and moved here):
//      Matt Larsen, Feb 15, 2019 -- adding conversions from vtk to bp
//
//      Justin Privitera, Wed Mar 23 12:26:31 PDT 2022
//      Added "domain" as first arg of MeshToVTK.
// 
//      Justin Privitera, Mon Apr 11 18:20:19 PDT 2022
//      Added "new_refine" as an arg to RefineMeshToVTK.
// 
//      Justin Privitera, Wed Apr 13 13:49:43 PDT 2022
//      Added LegacyRefineMeshToVTK and LowOrderMeshToVTK to MFEM class.
// 
//      Justin Privitera, Mon May  9 14:35:18 PDT 2022
//      Added LegacyRefineGridFunctionToVTK and LowOrderGridFunctionToVTK to 
//      MFEM class and added "new_refine" as an arg to RefineGridFunctionToVTK.
// 
//  Modifications:
//
// ****************************************************************************

class AVTBLUEPRINT_API avtConduitBlueprintDataAdaptor
{
public:
    static void Initialize();

    // set warning and info handlers to redirect conduit warnings and info
    static void SetInfoWarningHandlers();

    class BlueprintToVTK
    {
    public:
        /// Helpers for converting Mesh and Field Blueprint conforming data
        /// to vtk instances.
        static vtkDataSet*    MeshToVTK(int domain, 
                                        const conduit::Node &mesh);
        static vtkDataArray*  FieldToVTK(const conduit::Node &field);
    };

    class VTKToBlueprint
    {
    public:
        /// Helpers for converting vtk datasets to Mesh and Field Blueprint
        /// conforming data
        static void VTKFieldNameToBlueprint(const std::string &vtk_name,
                                            const std::string &topo_name,
                                            std::string &bp_name);

        static void VTKFieldsToBlueprint(conduit::Node &node,
                                         const std::string topo_name,
                                         vtkDataSet* dataset);

        static void VTKToBlueprintMesh(conduit::Node &mesh,
                                   vtkDataSet* dataset,
                                   const int ndims);
    };

    class BlueprintToMFEM
    {
    public:
        static mfem::Mesh         *MeshToMFEM(const conduit::Node &mesh,
                                              const std::string &topo_name = "");

        static mfem::GridFunction *FieldToMFEM(mfem::Mesh *mesh,
                                               const conduit::Node &field);
    };
};

#endif
