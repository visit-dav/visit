/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                            avtDyna3DFileFormat.C                          //
// ************************************************************************* //

#include <avtDyna3DFileFormat.h>

#include <set>
#include <map>
#include <snprintf.h>

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <Expression.h>

#include <TimingsManager.h>
#include <DebugStream.h>
#include <Utility.h>


// ****************************************************************************
//  Method: avtDyna3DFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri Dec 16 15:22:21 PST 2011
//    Use external file object.
//
// ****************************************************************************

avtDyna3DFileFormat::avtDyna3DFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), file()
{
#ifdef MDSERVER
    if(!file.Identify(filename))
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
#endif
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::~avtDyna3DFileFormat
//
// Purpose: 
//   Destructor for avtDyna3DFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:25:13 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtDyna3DFileFormat::~avtDyna3DFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtDyna3DFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
// ****************************************************************************

void
avtDyna3DFileFormat::FreeUpResources(void)
{
    debug4 << "avtDyna3DFileFormat::FreeUpResources" << endl;
    file.FreeUpResources();
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::ActivateTimestep
//
// Purpose: 
//   Called when we're activating the current time step.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:25:13 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtDyna3DFileFormat::ActivateTimestep()
{
    debug4 << "avtDyna3DFileFormat::ActivateTimestep" << endl;
#ifndef MDSERVER
    if(!file.HasBeenRead())
    {
        file.Read(filename);
    }
#endif
}

// ****************************************************************************
//  Method: avtDyna3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri Aug  1 11:44:37 PDT 2008
//    Added strength.
//
//    Brad Whitlock, Fri Dec 16 15:37:58 PST 2011
//    Use the file object. Add EPS.
//
// ****************************************************************************

void
avtDyna3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4 << "avtDyna3DFileFormat::PopulateDatabaseMetaData" << endl;
    int sdim = 3;
    int tdim = 3;
    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, 1, 1,
                      sdim, tdim);

    // Only advertise a material if we have material names.
    const MaterialPropertiesVector &materialCards = file.GetMaterials();
    if(!materialCards.empty())
    {
        stringVector materialNames;
        for(size_t i = 0; i < materialCards.size(); ++i)
            materialNames.push_back(materialCards[i].materialName);

        avtMaterialMetaData *mmd = new avtMaterialMetaData("material",
            "mesh", (int)materialNames.size(), materialNames);
        md->Add(mmd);

        // We had material names so advertise a density field.
        avtScalarMetaData *smd = new avtScalarMetaData(
                "density", "mesh", AVT_ZONECENT);
        smd->hasUnits = true;
        smd->units = "g/cc";
        md->Add(smd);

        // We had material names so advertise a strength field.
        smd = new avtScalarMetaData(
                "strength", "mesh", AVT_ZONECENT);
        md->Add(smd);

        // We had material names so advertise an eps field.
        smd = new avtScalarMetaData(
                "equivalentPlasticStrain", "mesh", AVT_ZONECENT);
        md->Add(smd);

        // We had material names so advertise the material number.
        smd = new avtScalarMetaData(
                "matno", "mesh", AVT_ZONECENT);
        smd->hasUnits = true;
        smd->units = "material number";
        md->Add(smd);
    }

    // Add a velocity vector if we were able to find one.
    vtkDataArray *velocity = file.GetVectorVar("velocity");
    if(velocity != NULL)
    {
        velocity->Delete();

        avtVectorMetaData *vmd = new avtVectorMetaData(
                "velocity", "mesh", AVT_NODECENT, 3);
        md->Add(vmd);

        Expression vx, vy, vz;
        vx.SetName("vx");
        vx.SetDefinition("velocity[0]");
        vx.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&vx);
        vy.SetName("vy");
        vy.SetDefinition("velocity[1]");
        vy.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&vy);
        vz.SetName("vz");
        vz.SetDefinition("velocity[2]");
        vz.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&vz);
    }
}


// ****************************************************************************
//  Method: avtDyna3DFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri Dec 16 15:37:58 PST 2011
//    Use the file object.
//
// ****************************************************************************

vtkDataSet *
avtDyna3DFileFormat::GetMesh(const char *meshname)
{
    debug4 << "avtDyna3DFileFormat::GetMesh" << endl;
    return file.GetMesh(meshname);
}


// ****************************************************************************
//  Method: avtDyna3DFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri Aug  1 14:10:33 PDT 2008
//    Added support for strength.
//
//    Brad Whitlock, Fri Dec 16 15:37:58 PST 2011
//    Use the file object.
//
// ****************************************************************************

vtkDataArray *
avtDyna3DFileFormat::GetVar(const char *varname)
{
    debug4 << "avtDyna3DFileFormat::GetVar" << endl;
    vtkDataArray *ret = NULL;
    if(strcmp(varname, "matno") == 0)
        ret = file.GetVar("material");
    else
        ret = file.GetVar(varname);

    if(ret == NULL)
    {
        // Bad variable name.
        EXCEPTION1(InvalidVariableException, varname);
    }

    return ret;
}


// ****************************************************************************
//  Method: avtDyna3DFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri Dec 16 15:37:58 PST 2011
//    Use the file object.
//
// ****************************************************************************

vtkDataArray *
avtDyna3DFileFormat::GetVectorVar(const char *varname)
{
    debug4 << "avtDyna3DFileFormat::GetVectorVar" << endl;
    vtkDataArray *ret = file.GetVectorVar(varname);

    if(ret == NULL)
    {
        // Can't read variables yet.
        EXCEPTION1(InvalidVariableException, varname);
    }

    return ret;
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::GetAuxiliaryData
//
// Purpose: 
//   Read the material from the database.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:21:34 PST 2006
//
// Modifications:
//   Brad Whitlock, Fri Dec 16 15:44:05 PST 2011
//   Use the file object.
//
// ****************************************************************************

void  *
avtDyna3DFileFormat::GetAuxiliaryData(const char *var, const char *type,
    void *args, DestructorFunction &df)
{
    const char *mName = "avtDyna3DFileFormat::GetAuxiliaryData: ";
    debug4 << mName
           << "var=" << var
           << ", type=" << type
           << endl;

    avtMaterial *retval = 0;
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        const MaterialPropertiesVector &materialCards = file.GetMaterials();
        df = avtMaterial::Destruct;

        int *matnos = new int[materialCards.size()];
        char **names = new char *[materialCards.size()];
        for(size_t i = 0; i < materialCards.size(); ++i)
        {
            matnos[i] = materialCards[i].materialNumber;
            names[i] = (char *)materialCards[i].materialName.c_str();
        }

        vtkDataArray *matNumbers = file.GetVar("material");
        if(matNumbers != NULL)
        {
            int dims[3]; 
            dims[0] = matNumbers->GetNumberOfTuples();
            dims[1] = 1;
            dims[2] = 1;
            retval = new avtMaterial(
                (int)materialCards.size(),
                matnos,
                names,
                3,
                dims,
                0,
                (const int *)matNumbers->GetVoidPointer(0),
                0,
                0,
                0,
                0,
                0);
            matNumbers->Delete();
        }
        else
        {
            delete [] matnos;
            delete [] names;
            debug4 << mName << "Could not get material data" << endl;
        }
    }

    debug4 << mName << "end" << endl;

    return retval;
}
