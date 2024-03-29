// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <TimingsManager.h>

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
    nmats = 0;
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
//  Modifications:
//    Jeremy Meredith, Mon Jan  4 15:09:23 EST 2010
//    Added some timings.
//
//    Kathleen Biagas, Mon Jan 28 10:35:29 PST 2013
//    Call update on the filter not the data object.
//
// ****************************************************************************
bool
YoungsMIR::ReconstructMesh(vtkDataSet *orig_ds, avtMaterial *orig_mat, int dim)
{
    debug3 << "Starting Youngs MIR algorithm.\n";
    vtkDataSet *ds =vtkDataSet::SafeDownCast(orig_ds->NewInstance());
    ds->ShallowCopy(orig_ds);

    int th_full = visitTimer->StartTimer();

    // Pack the material
    int th_packmat = visitTimer->StartTimer();
    avtMaterial *mat = orig_mat->CreatePackedMaterial();
    mapMatToUsedMat = orig_mat->GetMapMatToUsedMat();
    mapUsedMatToMat = orig_mat->GetMapUsedMatToMat();
    visitTimer->StopTimer(th_packmat, "MIR: Pack material");

    //vector<string> matNames = mat->GetMaterials();
    nmats = mat->GetNMaterials();
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

    // gradient
    int th_grad = visitTimer->StartTimer();
    vtkCQS *cqs = vtkCQS::New();
    cqs->SetInputData(ds);
    cqs->Update();
    ds = cqs->GetOutput();
    ds->Register(NULL);
    cqs->Delete();

    for (int m=0; m<nmats; m++)
    {
        vtkDataSetGradient *grad = vtkDataSetGradient::New();
        grad->SetInputData(ds);

        char str[256];
        sprintf(str, "material%05d", mapUsedMatToMat[m]);
        ds->GetCellData()->SetActiveScalars(str);
        // KSB: VTK (as of version 9) complains if this isn't set, because the
        // grad filter calls GetInputArrayToProcess
        grad->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_CELLS, str);

        char str2[256];
        sprintf(str2, "grad%05d", mapUsedMatToMat[m]);
        grad->SetGradientArrayName(str2);

        grad->Update();
        ds = grad->GetOutput();
        ds->Register(NULL);
        grad->Delete();
    }
    visitTimer->StopTimer(th_grad, "MIR: Gradient");

    // recenter the gradient!
    int th_recenter = visitTimer->StartTimer();
    vtkVisItPointDataToCellData *pd2cd = vtkVisItPointDataToCellData::New();
    pd2cd->SetInputData(ds);
    pd2cd->Update();
    ds = pd2cd->GetOutput();
    ds->Register(NULL);
    pd2cd->Delete();
    visitTimer->StopTimer(th_recenter, "MIR: Recenter gradients");


    int th_youngs = visitTimer->StartTimer();
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
    visitTimer->StopTimer(th_youngs, "MIR: Actual PLIC execution");

    visitTimer->StopTimer(th_full, "MIR: Full reconstruction");

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
//  Modifications:
//    Jeremy Meredith, Mon Jan  4 15:09:23 EST 2010
//    Added some timings.
//
//    Kathleen Biagas, Wed Dec 18 11:21:12 PST 2013
//    Return all materials if mats is empty. (To match behavior of ZooMIR).
//
// ****************************************************************************
vtkDataSet *
YoungsMIR::GetDataset(std::vector<int> mats, vtkDataSet *ds,
                      std::vector<avtMixedVariable *> mixvars,
                      bool doMats,
                      avtMaterial *mat)
{
    bool doAllMats = mats.size() == 0;

    int timerHandle = visitTimer->StartTimer();

    // Collect up all the data sets we're asked for
    vector<int>         matIndex;
    vector<vtkDataSet*> matDS;

    if (!doAllMats)
    {
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
    }
    else
    {
        for (int i = 0; i < nmats; i++)
        {
            matIndex.push_back(i);
            matDS.push_back(output[i]);
        }
    }

    // Return the input, or merge multiple ones
    vtkDataSet *retval = NULL;
    if (matDS.size() == 0)
    {
        visitTimer->StopTimer(timerHandle, "MIR: Getting empty clean dataset");
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
        for (size_t i=0; i<matDS.size(); i++)
            append->AddInputData(matDS[i]);

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
            if ((size_t)matindex < matDS.size() &&
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

    visitTimer->StopTimer(timerHandle, "MIR: Getting clean dataset");
    return retval;
}


