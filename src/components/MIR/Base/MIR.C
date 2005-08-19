#include "MIR.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

#include <vtkDataSet.h>
#include <vtkCell.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>

#include <avtMixedVariable.h>
#include <avtMaterial.h>
#include <avtSpecies.h>

#include <QuadraticHash.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <TimingsManager.h>

#include <string>
#include <vector>
using std::string;
using std::vector;


#define STDMIN(A, B) (((A)<(B)) ? (A) : (B))
#define STDMAX(A, B) (((A)>(B)) ? (A) : (B))


// ****************************************************************************
//  Method:  MIR::SetAlgorithm
//
//  Purpose:
//    Set the option algorithm
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 18, 2005
//
// ****************************************************************************
void
MIR::SetAlgorithm(int a)
{
    options.algorithm = a;
}

// ****************************************************************************
//  Method:  MIR::SetSubdivisionLevel
//
//  Purpose:
//    Set the option subdivisionLevel.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2002
//
// ****************************************************************************
void
MIR::SetSubdivisionLevel(MIROptions::SubdivisionLevel sl)
{
    options.subdivisionLevel = sl;
}

// ****************************************************************************
//  Method:  MIR::SetNumIterations
//
//  Purpose:
//    Set the option numIterations.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2002
//
// ****************************************************************************
void
MIR::SetNumIterations(int ni)
{
    options.numIterations = ni;
}

// ****************************************************************************
//  Method:  MIR::SetSmoothing
//
//  Purpose:
//    Set the option smoothing.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2002
//
// ****************************************************************************
void
MIR::SetSmoothing(bool sm)
{
    options.smoothing = sm;
}

// ****************************************************************************
//  Method:  MIR::SetLeaveCleanZonesWhole
//
//  Purpose:
//    Set the option leaveCleanZonesWhole.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2002
//
// ****************************************************************************
void
MIR::SetLeaveCleanZonesWhole(bool lczw)
{
    options.leaveCleanZonesWhole = lczw;
}

// ****************************************************************************
//  Method:  MIR::SetCleanZonesOnly
//
//  Purpose:
//    Set the option cleanZonesOnly.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 25, 2002
//
// ****************************************************************************
void
MIR::SetCleanZonesOnly(bool czo)
{
    options.cleanZonesOnly = czo;
}

// ****************************************************************************
//  Method:  MIR::SetIsovolumeVF
//
//  Purpose:
//    Set the VF cutoff for the isovolume algorithm.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 18, 2005
//
// ****************************************************************************
void
MIR::SetIsovolumeVF(float vf)
{
    options.isovolumeVF = vf;
}

// ****************************************************************************
//  Default Constructor:  MIR::MIR
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 19, 2003
//
//  Modifications:
// ****************************************************************************
MIR::MIR()
{
}

// ****************************************************************************
//  Destructor:  MIR::~MIR
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 19, 2003
//
//  Modifications:
//
// ****************************************************************************
MIR::~MIR()
{
}

// ****************************************************************************
//  Method: MIR::Destruct
//
//  Purpose:
//      This is a function that is kept with a void reference pointer so that
//      MIRs can be properly destructed.
//
//  Programmer: Hank Childs
//  Creation:   September 24, 2002
//
// ****************************************************************************

void
MIR::Destruct(void *p)
{
    MIR *mir = (MIR *) p;
    if (mir != NULL)
    {
        delete mir;
    }
}


// ****************************************************************************
//  Method:  MIR::SpeciesSelect
//
//  Purpose:
//    Perform species selection on a variable.
//
//  Arguments:
//    selection   true for each material-species index if it is selected
//    mat         the material object
//    spec        the species object
//    var_in      the zonal scalar array input
//    mixvar_in   the mixed scalar array input
//    var_out     the zonal scalar array output
//    mixvar_out  the zonal scalar array output
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 18, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, 
//    use vtkDataArray and vtkFloatArray instead.
//
//    Hank Childs, Thu Jul  4 16:51:37 PDT 2002
//    Add new name argument to mixed variable constructor.
//
//    Hank Childs, Wed Sep 25 15:16:13 PDT 2002
//    Fixed memory leak.
//
//    Hank Childs, Tue Dec 10 15:37:14 PST 2002
//    Put in a sanity check.
//
// ****************************************************************************
void
MIR::SpeciesSelect(const vector<bool> &selection,
                   avtMaterial *mat, avtSpecies *spec,
                   vtkDataArray *var_in, avtMixedVariable *mixvar_in,
                   vtkDataArray *&var_out, avtMixedVariable *&mixvar_out)
{
    if (mat->GetNMaterials() != spec->GetNMat())
    {
        string str = "The species and the material do not match up.  This may"
                " be a problem with the data file or it may be an internal "
                "error.";
        EXCEPTION1(VisItException, str);
    }

    vector<bool>          selMat(mat->GetNMaterials(), false);
    vector<vector<bool> > selSpec(mat->GetNMaterials());
    int selIndex = 0;

    // Make a couple easier lookup arrays for which mats/specs are selected
    for (int m=0; m<mat->GetNMaterials(); m++)
    {
        for (int s=0; s<spec->GetNSpecies()[m]; s++)
        {
            if (selection[selIndex])
            {
                selMat[m] = true;
                selSpec[m].push_back(true);
            }
            else
            {
                selSpec[m].push_back(false);
            }
            selIndex++;
        }
    }

    // Get the necessary input arrays
    int                nCells         = mat->GetNZones();
    const int         *matlist        = mat->GetMatlist();
    const int         *mixmat         = mat->GetMixMat();
    const int         *mixnext        = mat->GetMixNext();
    const float       *mixvf          = mat->GetMixVF();
    const vector<int> &nSpecies       = spec->GetNSpecies();
    const float       *specMF         = spec->GetSpecMF();
    const int         *speclist       = spec->GetSpeclist();
    const int         *mixSpeclist    = spec->GetMixSpeclist();

    // Get the input mixed arrays
    int                mixlen         = mixvar_in ? mat->GetMixlen() : 0;
    const float       *mixvarbuff     = mixlen ? mixvar_in->GetBuffer() : NULL;

    // create the output arrays
    float       *mixvarbuff_out = mixlen ? new float[mixlen] : NULL;
    vtkFloatArray  *var         = vtkFloatArray::New();
    var->SetNumberOfTuples(nCells);

    // Loop over every zone
    for (int i=0; i<nCells; i++)
    {
        float specmf = 0;
        int matno  = matlist[i];
        if (matno >= 0)
        {
            // Clean zone
            if (selMat[matno])
            {
                int specindex = speclist[i];
                if (specindex == 0)
                {
                    // A zero indicates only one species in this material
                    specmf = 1;
                }
                else
                {
                    for (int j=0 ; j<nSpecies[matno]; j++)
                    {
                        if (selSpec[matno][j])
                            specmf += specMF[specindex + j - 1];
                    }
                }
            }
        }
        else
        {
            // Mixed zone ; loop over every material in the zone
            int mixindex = -matno - 1;
            while (mixindex >= 0)
            {
                float vf = mixvf[mixindex];
                int mixmatno = mixmat[mixindex];

                float mixspecmf = 0;
                if (selMat[mixmatno])
                {
                    int specindex = mixSpeclist[mixindex];
                    if (specindex == 0)
                    {
                        // A zero indicates only one species in this material
                        mixspecmf = 1;
                    }
                    else
                    {
                        for (int j=0 ; j<nSpecies[mixmatno]; j++)
                        {
                            if (selSpec[mixmatno][j])
                                mixspecmf += specMF[specindex + j - 1];
                        }
                    }
                }

                // Do the mixed values while we know the mixspecmf
                if (mixlen)
                {
                    mixvarbuff_out[mixindex] = mixspecmf*mixvarbuff[mixindex];
                }

                // Weight the mass fractions by volume fraction
                // (This is so they sum to 1.0)
                specmf += mixspecmf * vf;

                mixindex = mixnext[mixindex] - 1;
            }
        }

        // Set the clean values
        var->SetValue(i, specmf * var_in->GetTuple1(i));
    }

    // Set the return values
    var_out = var;
    var_out->SetName(var_in->GetName());
    mixvar_out = NULL;
    if (mixlen > 0)
    {
        mixvar_out = new avtMixedVariable(mixvarbuff_out, mixlen,
                                          mixvar_in->GetVarname());
    }
    delete [] mixvarbuff_out;
}
