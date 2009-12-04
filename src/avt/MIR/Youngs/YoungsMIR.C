/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

#include "YoungsMIR.h"

#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkVisItPointDataToCellData.h>
#include <vtkAppendFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIntArray.h>

#include "vtkDataSetGradient.h"
#include "vtkCQS.h"
#include "vtkYoungsMaterialInterface.h"

#include <ImproperUseException.h>
#include <DebugStream.h>
#include <avtMaterial.h>

#include <string>
#include <vector>
using std::string;
using std::vector;

// ****************************************************************************
//  Constructor:  YoungsMIR::YoungsMIR
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  4, 2009
//
// ****************************************************************************
YoungsMIR::YoungsMIR()
{
}

// ****************************************************************************
//  Destructor:  YoungsMIR::~YoungsMIR
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  4, 2009
//
// ****************************************************************************
YoungsMIR::~YoungsMIR()
{
}

// ****************************************************************************
//  Method:  YoungsMIR::ReconstructMesh
//
//  Purpose:
//    Main method for interface reconstruction in any dimension.
//
//  Arguments:
//    mesh_orig       the mesh
//    mat_orig        the material
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  4, 2009
//
// ****************************************************************************
bool
YoungsMIR::ReconstructMesh(vtkDataSet *orig_ds, avtMaterial *orig_mat, int dim)
{
    debug3 << "Starting Youngs MIR algorithm.\n";
    vtkDataSet *ds =vtkDataSet::SafeDownCast(orig_ds->NewInstance());
    ds->ShallowCopy(orig_ds);


    // Pack the material
    avtMaterial *mat = orig_mat->CreatePackedMaterial();
    mapMatToUsedMat = orig_mat->GetMapMatToUsedMat();
    mapUsedMatToMat = orig_mat->GetMapUsedMatToMat();

    //vector<string> matNames = mat->GetMaterials();
    int nmats = mat->GetNMaterials();
    int ncells = ds->GetNumberOfCells();
    vector<vtkFloatArray *>vf(nmats);
    for (int m=0; m<nmats; m++)
    {
        vf[m] = vtkFloatArray::New();
        vf[m]->SetNumberOfTuples(ncells);
        vf[m]->SetNumberOfComponents(1);
        char str[256];
        sprintf(str, "material%05d", mapUsedMatToMat[m]);
        //vf[m]->SetName(matNames[m].c_str());
        vf[m]->SetName(str);
        ds->GetCellData()->AddArray(vf[m]);
        vf[m]->Delete();
    }

    vector<float> cellvfs(nmats);
    for (int c=0; c<ncells; c++)
    {
        mat->GetVolFracsForZone(c, cellvfs);
        for (int m=0; m<nmats; m++)
        {
            vf[m]->SetTuple1(c, cellvfs[m]);
        }
    }

    // gradietn
    vtkCQS *cqs = vtkCQS::New();
    cqs->SetInput(ds);
    ds = cqs->GetOutput();
    ds->Update();
    ds->Register(NULL);
    cqs->Delete();

    for (int m=0; m<nmats; m++)
    {
        vtkDataSetGradient *grad = vtkDataSetGradient::New();
        grad->SetInput(ds);

        char str[256];
        sprintf(str, "material%05d", mapUsedMatToMat[m]);
        ds->GetCellData()->SetActiveScalars(str);

        char str2[256];
        sprintf(str2, "grad%05d", mapUsedMatToMat[m]);
        grad->SetGradientArrayName(str2);

        ds = grad->GetOutput();
        ds->Update();
        ds->Register(NULL);
        grad->Delete();
    }

    // recenter the gradient!
    vtkVisItPointDataToCellData *pd2cd = vtkVisItPointDataToCellData::New();
    pd2cd->SetInput(ds);
    ds = pd2cd->GetOutput();
    ds->Update();
    ds->Register(NULL);
    pd2cd->Delete();


    vtkYoungsMaterialInterface *youngs = new vtkYoungsMaterialInterface();
    youngs->SetFillMaterial(true);
    for (int m=0; m<nmats; m++)
    {
        char str1[256], str2[256];
        sprintf(str1, "material%05d", mapUsedMatToMat[m]);
        sprintf(str2, "grad%05d", mapUsedMatToMat[m]);

        youngs->AddMaterial(str1,str2,str1);
    }

    output = new vtkDataSet*[nmats];
    youngs->Execute(ds, output);

#if 0
    if (false)
    {
        vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
        char str[1024];
        static int val =0;
        val++;
        sprintf(str, "ZZ_input%04d.vtk", val);
        wrtr->SetFileName(str);
        wrtr->SetInput(ds);
        wrtr->Write();
        wrtr->Delete();
    }
    for (int m=0; m<nmats; m++)
    {
        vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
        char str[1024];
        static int val = 0;
        val++;
        sprintf(str, "ZZ_output%04d.vtk", val);
        wrtr->SetFileName(str);
        cerr << "output[m] = "<<output[m]<<endl;
        wrtr->SetInput(output[m]);
        wrtr->Write();
        //wrtr->Delete();  uh, this causes problems.....   leak it for
        //                 convenience purposes since it's only debug code
    }
#endif

    delete mat;
    return true;
}

// ****************************************************************************
//  Method:  YoungsMIR::Reconstruct3DMesh
//
//  Purpose:
//    Main method for interface reconstruction in 3D.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  4, 2009
//
// ****************************************************************************
bool
YoungsMIR::Reconstruct3DMesh(vtkDataSet *ds, avtMaterial *mat)
{
    return ReconstructMesh(ds,mat,3);
}

// ****************************************************************************
//  Method:  YoungsMIR::Reconstruct2DMesh
//
//  Purpose:
//    Main method for interface reconstruction in 2D.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  4, 2009
//
// ****************************************************************************
bool
YoungsMIR::Reconstruct2DMesh(vtkDataSet *ds, avtMaterial *mat)
{
    return ReconstructMesh(ds,mat,2);
}

// ****************************************************************************
//  Method:  ZooMIR::GetDataset
//
//  Purpose:
//    Return the cells that correspond to the given materials.  Do the right
//    thing with the various data arrays, etc.
//
//  Arguments:
//    mats        the materials to select
//    ds          the original dataset
//    mixvars     the mixed variables
//    doMats      flag to write out the material numbers as a new zonal array
//    mat         the original material for this domain
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  4, 2009
//
// ****************************************************************************
vtkDataSet *
YoungsMIR::GetDataset(std::vector<int> mats, vtkDataSet *ds, 
                      std::vector<avtMixedVariable *> mixvars,
                      bool doMats,
                      avtMaterial *mat)
{
    if (mats.size() == 0)
        return NULL;

    // Collect up all the data sets we're asked for
    vector<int>         matIndex;
    vector<vtkDataSet*> matDS;
    for (unsigned int i=0; i<mats.size(); i++)
    {
        int matno = mats[i];
        int index = -1;
        for (unsigned int m=0; m<mapUsedMatToMat.size(); m++)
        {
            if (mapUsedMatToMat[m] == matno)
            {
                index = m;
                break;
            }
        }
        
        if (index >= 0)
        {
            matIndex.push_back(index);
            matDS.push_back(output[index]);
        }
    }

    // Return the input, or merge multiple ones
    vtkDataSet *retval = NULL;
    if (matDS.size() == 0)
    {
        return NULL;
    }
    else if (matDS.size() == 1)
    {
        retval = matDS[0];
        retval->Register(NULL);
    }
    else //(matDS.size() > 1)
    {
        vtkAppendFilter *append = vtkAppendFilter::New();
        for (int i=0; i<matDS.size(); i++)
            append->AddInput(matDS[i]);

        retval = append->GetOutput();
        append->Update();
        retval->Register(NULL);
        append->Delete();
    }


    if (retval && doMats)
    {
        //
        // Add an array that contains the material for each zone
        // (which is now clean after reconstruction).
        //
        int ncells = retval->GetNumberOfCells();
        vtkIntArray *outmat = vtkIntArray::New();
        outmat->SetName("avtSubsets");
        outmat->SetNumberOfTuples(ncells);
        int *buff = outmat->GetPointer(0);
        int matindex = 0;
        int currcount = 0;
        for (int i=0; i<ncells; i++)
        {
            if (matindex < matDS.size() &&
                currcount >= matDS[matindex]->GetNumberOfCells())
            {
                matindex++;
                currcount=0;
            }
            int matno = mapUsedMatToMat[matIndex[matindex]];
            buff[i] = matno;
            currcount++;
        }
        retval->GetCellData()->AddArray(outmat);
        outmat->Delete();
    }

    return retval;
}


