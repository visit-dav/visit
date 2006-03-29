// ************************************************************************* //
//                           avtMoleculeFilter.C                             //
// ************************************************************************* //

#include <avtMoleculeFilter.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <DebugStream.h>
#include <AtomicProperties.h>

// ****************************************************************************
//  Method: avtMoleculeFilter constructor
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   February 14, 2006
//
//  Modifications:
//
// ****************************************************************************

avtMoleculeFilter::avtMoleculeFilter()
{
}


// ****************************************************************************
//  Method: avtMoleculeFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   February 14, 2006
//
//  Modifications:
//
// ****************************************************************************

avtMoleculeFilter::~avtMoleculeFilter()
{
}


// ****************************************************************************
//  Method: avtMoleculeFilter::ExecuteData
//
//  Purpose:
//      If we've got an integral data type with less than 1000 entries, then
//      we want to treat it like an enumerated type.  We'll figure out which
//      values are used, then treat them like labels (cf. subset labels).
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer:   Jeremy Meredith
//  Creation:     March 23, 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtMoleculeFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    int natoms = in_ds->GetNumberOfCells();
    vtkDataArray *primary = in_ds->GetCellData()->GetScalars();
    if (!primary)
        primary = in_ds->GetPointData()->GetScalars();
    if (natoms>0 && primary)
    {
        name = primary->GetName();
        float *scalar = primary->GetTuple(0);

        bool integral_type = true;
        int minval =  INT_MAX;
        int maxval = -INT_MAX;
        int i;
        for (i=0; i<natoms; i++)
        {
            int iscalar = int(scalar[i]);
            if (scalar[i] != float(iscalar))
            {
                integral_type = false;
                break;
            }
            if (iscalar < minval)
                minval = iscalar;
            if (iscalar > maxval)
                maxval = iscalar;
        }

        debug4 << "avtMoleculeFilter::ExecuteData: integral_type="
               << integral_type << endl;
        debug4 << "avtMoleculeFilter::ExecuteData: min=" << minval << endl;
        debug4 << "avtMoleculeFilter::ExecuteData: max=" << maxval << endl;

        if (integral_type && minval >= 0 && maxval < 10000)
        {
            bool *hasval = new bool[maxval+1];
            for (i=0; i<=maxval; i++)
                hasval[i] = false;

            for (i=0; i<natoms; i++)
            {
                hasval[int(scalar[i])] = true;
            }

            for (i=0; i<=maxval; i++)
            {
                if (hasval[i])
                {
                    used_values.insert(i);
                }
            }

            delete [] hasval;
        }
    }
    return in_ds;
}


// ****************************************************************************
//  Method: avtMoleculeFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Tell the thing to keep the node and zone number arrays.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 14, 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtMoleculeFilter::RefashionDataObjectInfo(void)
{
    // We probably don't need this.
    //GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);

    // Don't bother with the normals.
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);

    // Even though we requested original cell numbers, it will
    // rudely strip them off unless we tell it we were serious.
    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(true);
}


// ****************************************************************************
//  Method: avtMoleculeFilter::ReleaseData
//
//  Purpose:
//      Releases all problem size data associated with this filter.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 14, 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtMoleculeFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();
}


// ****************************************************************************
//  Method: avtMoleculeFilter::PerformRestriction
//
//  Purpose:  
//    Do nothing; the plot already requests original zones. 
// 
//  Programmer: Jeremy Meredith
//  Creation:   February 14, 2006
//
//  Modifications:
//
// ****************************************************************************

avtPipelineSpecification_p
avtMoleculeFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = spec;

    return rv;
}


// ****************************************************************************
//  Method:  avtMoleculeFilter::PreExecute
//
//  Purpose:
//    Clear out the values we've accumulated.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtMoleculeFilter::PreExecute()
{
    used_values.clear();
    name = "";
}

// ****************************************************************************
//  Method:  avtMoleculeFilter::PostExecute
//
//  Purpose:
//    Now that we know which integral values exist in this data base,
//    apply names to them and make these be the date attribute labels.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtMoleculeFilter::PostExecute()
{
    vector<string> labels;

    if (name == "element")
    {
        for (set<int>::iterator it = used_values.begin();
             it != used_values.end();
             it++)
        {
            labels.push_back(element_names[*it - 1]);
        }
    }
    else if (name == "restype")
    {
        // Poke on/off state, and the short and long names of all of 
        // the known residue names into the labels. We send them all 
        // with an on/off flag so we can let the viewer know about 
        // new residues that we've learned about.
        bool err = false;
        for(int i = 0; i < NumberOfKnownResidues(); ++i)
        {
            const char *rn = NumberToResiduename(i);
            if(rn == 0)
            {
                err = true;
                break;
            }

            if(used_values.find(i) == used_values.end())
                labels.push_back("off");
            else
                labels.push_back("on");
            labels.push_back(rn);
            labels.push_back(ResiduenameToLongName(rn));
        }
    }
    else
    {
        char nm[256];
        for (set<int>::iterator it = used_values.begin();
             it != used_values.end();
             it++)
        {
            sprintf(nm, "%d", *it);
            labels.push_back(nm);
        }
    }

    GetOutput()->GetInfo().GetAttributes().SetLabels(labels);
}


// ****************************************************************************
//  Method:  avtMoleculeFilter::SetAtts
//
//  Purpose:
//    Get a copy of the Plot's attributes.
//
//  Arguments:
//    a          the new MoleculeAttributes
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtMoleculeFilter::SetAtts(const MoleculeAttributes *a)
{
    atts = *a;
}
