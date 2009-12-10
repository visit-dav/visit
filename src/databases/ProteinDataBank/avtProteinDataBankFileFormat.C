/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtProteinDataBankFileFormat.C                 //
// ************************************************************************* //

#include <avtProteinDataBankFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <avtDatabaseMetaData.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <AtomicProperties.h>
#include <snprintf.h>

#include <DebugStream.h>

#include <float.h>

using std::string;
using std::vector;
using std::pair;

string
TrimTrailingSpaces(char *s)
{
    char *c = s + strlen(s) - 1;
    while(*c == ' ' && c >= s)
        *c-- = '\0';

    return string(s);
}

// ****************************************************************************
//  Method: avtProteinDataBank constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 17:48:22 EDT 2006
//    Changed to STSD.
//
// ****************************************************************************

avtProteinDataBankFileFormat::avtProteinDataBankFileFormat(const char *fn,
                                                DBOptionsAttributes *readOpts)
    : avtSTSDFileFormat(fn)
{
    filename = fn;
    OpenFileAtBeginning();

    nmodels = 0;
    metadata_read = false;
    dbTitle = "";
}

// ****************************************************************************
//  Method: avtProteinDataBankFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 17:48:22 EDT 2006
//    Changed bonds to line elements.
//
//    Jeremy Meredith, Thu Oct 18 16:31:00 EDT 2007
//    Added compound support.
//
// ****************************************************************************

void
avtProteinDataBankFileFormat::FreeUpResources(void)
{
    in.close();

    bonds.clear();
    for (int i=0; i<allatoms.size(); i++)
    {
        allatoms[i].clear();
    }
    allatoms.clear();

    compoundNames.clear();

    nmodels = 0;
    metadata_read = false;
}


// ****************************************************************************
//  Method: avtProteinDataBankFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 17:49:30 EDT 2006
//    Exposed models through directories instead of time steps.
//
//    Jeremy Meredith, Wed Oct 17 11:27:10 EDT 2007
//    Added compound support.
//
//    Jeremy Meredith, Thu Oct 18 16:31:20 EDT 2007
//    Only add compounds as a variable if there's more than one of them.
//
//    Jeremy Meredith, Mon Oct 22 12:58:00 EDT 2007
//    Added compound name support.
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Changed interface to enum scalars
//
//    Jeremy Meredith, Thu May  1 12:43:27 EDT 2008
//    Exposed the occupancy and temperature factor fields.
//
//    Jeremy Meredith, Thu Aug  7 13:46:38 EDT 2008
//    Fixed broken sprintf commands.
//
//    Jeremy Meredith, Thu Feb 12 12:27:28 EST 2009
//    Added a simple residue type enumeration.  Needed to move
//    HETNAM parsing into meta-data reading instead of atom data reading.
//
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    MAX_ELEMENT_NUMBER now means the actual max element number, not the
//    total number of known elements in visit.  Added a fake "0" element
//    which means "unknown", and hydrogen now starts at 1.  This
//    also means we don't have to correct for 1-origin atomic numbers.
//
// ****************************************************************************

void
avtProteinDataBankFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadAllMetaData();
    md->SetDatabaseComment(dbTitle);

    char prefix[20] = "";
    int count = 1;
    if (nmodels > 1)
        count = nmodels + 1;
    for (int i=0; i<count; i++)
    {
        if (count > 1 && i != count-1)
        {
            sprintf(prefix, "models/model_%02d/", i);
        }
        else
        {
            strcpy(prefix, "");
        }

        char name_mesh[80];
        char name_el[80],name_rt[80],name_rs[80],name_bk[80];
        char name_nm[80],name_rn[80],name_lr[80],name_en[80];
        char name_cmp[80],name_cmpnm[80],name_occ[80],name_temp[80];
        sprintf(name_mesh, "%smesh",        prefix);
        sprintf(name_el,   "%selement",     prefix);
        sprintf(name_rt,   "%srestype",     prefix);
        sprintf(name_rs,   "%sresseq",      prefix);
        sprintf(name_bk,   "%sbackbone",    prefix);
        sprintf(name_nm,   "%sname",        prefix);
        sprintf(name_rn,   "%sresname",     prefix);
        sprintf(name_lr,   "%slongresname", prefix);
        sprintf(name_en,   "%selementname", prefix);
        sprintf(name_cmp,  "%scompound",    prefix);
        sprintf(name_cmpnm,"%scompoundname",prefix);
        sprintf(name_occ,  "%soccupancy",   prefix);
        sprintf(name_temp, "%stempFactor",  prefix);

        avtMeshMetaData *mmd = new avtMeshMetaData(name_mesh, 1, 0,0,0,
                                                   3, 0,
                                                   AVT_POINT_MESH);
        mmd->nodesAreCritical = true;
        md->Add(mmd);

        // Add the element scalars
        avtScalarMetaData *el_smd =
            new avtScalarMetaData(name_el, name_mesh, AVT_NODECENT);
        el_smd->SetEnumerationType(avtScalarMetaData::ByValue);
        for (int a=0; a<=MAX_ELEMENT_NUMBER; a++)
            el_smd->AddEnumNameValue(element_names[a], a);
        md->Add(el_smd);

        // Add the compound scalars
        if (compoundNames.size() > 1)
        {
            avtScalarMetaData *cmp_smd =
                new avtScalarMetaData(name_cmp, name_mesh, AVT_NODECENT);
            cmp_smd->SetEnumerationType(avtScalarMetaData::ByValue);
            for (int a=0; a<compoundNames.size(); a++)
                cmp_smd->AddEnumNameValue(compoundNames[a], a);
            md->Add(cmp_smd);
            md->Add(new avtLabelMetaData(name_cmpnm, name_mesh, AVT_NODECENT));
        }

        // Add the residue types with enumeration
        avtScalarMetaData *rt_smd =
            new avtScalarMetaData(name_rt, name_mesh, AVT_NODECENT);
        rt_smd->SetEnumerationType(avtScalarMetaData::ByValue);
        for (int r=0; r<NumberOfKnownResidues(); r++)
            rt_smd->AddEnumNameValue(NumberToResiduename(r), r);
        md->Add(rt_smd);

        // Add the rest of the scalars
        AddScalarVarToMetaData(md, name_rs, name_mesh, AVT_NODECENT);
        AddScalarVarToMetaData(md, name_bk, name_mesh, AVT_NODECENT);
        AddScalarVarToMetaData(md, name_occ, name_mesh, AVT_NODECENT);
        AddScalarVarToMetaData(md, name_temp, name_mesh, AVT_NODECENT);

        // Add a couple of label variables.
        md->Add(new avtLabelMetaData(name_nm, name_mesh, AVT_NODECENT));
        md->Add(new avtLabelMetaData(name_rn, name_mesh, AVT_NODECENT));
        md->Add(new avtLabelMetaData(name_lr, name_mesh, AVT_NODECENT));
        md->Add(new avtLabelMetaData(name_en, name_mesh, AVT_NODECENT));
    }
}


// ****************************************************************************
//  Method: avtProteinDataBankFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 17:49:30 EDT 2006
//    Exposed models through directories instead of time steps.
//    Changed bonds to line segment elements.
//
// ****************************************************************************

vtkDataSet *
avtProteinDataBankFileFormat::GetMesh(const char *orig_meshname)
{
    int model = 0;
    const char *meshname = orig_meshname;
    if (sscanf(orig_meshname, "models/model_%02d/", &model))
    {
        meshname = &(orig_meshname[16]);
    }
    else
    {
        model = 0;
    }

    ReadAtomsForModel(model);

    vector<Atom> &atoms = allatoms[model];
    int natoms = atoms.size();

    vtkPolyData *pd  = vtkPolyData::New();
    vtkPoints   *pts = vtkPoints::New();
    pd->SetPoints(pts);

    pts->SetNumberOfPoints(atoms.size());
    for (int j = 0 ; j < atoms.size() ; j++)
    {
        pts->SetPoint(j,
                      atoms[j].x,
                      atoms[j].y,
                      atoms[j].z);
    }
 
    vtkCellArray *lines = vtkCellArray::New();
    pd->SetLines(lines);
    for (int k = 0 ; k < bonds.size() ; k++)
    {
        lines->InsertNextCell(2);
        lines->InsertCellPoint(bonds[k].first);
        lines->InsertCellPoint(bonds[k].second);
    }

    vtkCellArray *verts = vtkCellArray::New();
    pd->SetVerts(verts);
    for (int k = 0 ; k < atoms.size() ; k++)
    {
        verts->InsertNextCell(1);
        verts->InsertCellPoint(k);
    }

    pts->Delete();
    lines->Delete();
    verts->Delete();

    return pd;
}


// ****************************************************************************
//  Method: avtProteinDataBankFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
//  Modifications:
//
//    Hank Childs, Thu May 18 11:15:01 PDT 2006
//    Fix UMR.
//
//    Jeremy Meredith, Mon Aug 28 17:51:07 EDT 2006
//    Exposed models through directories, not time steps.
//
//    Jeremy Meredith, Wed Oct 17 11:27:10 EDT 2007
//    Added compound support.
//
//    Jeremy Meredith, Mon Oct 22 12:58:00 EDT 2007
//    Added compound name support.
//
//    Jeremy Meredith, Thu May  1 12:43:27 EDT 2008
//    Exposed the occupancy and temperature factor fields.
//
// ****************************************************************************

vtkDataArray *
avtProteinDataBankFileFormat::GetVar(const char *orig_varname)
{
    int model = 0;
    const char *varname = orig_varname;
    if (sscanf(orig_varname, "models/model_%02d/", &model))
    {
        varname = &(orig_varname[16]);
    }
    else
    {
        model = 0;
    }

    ReadAtomsForModel(model);

    vector<Atom> &atoms = allatoms[model];

    if (string(varname) == "element")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            ptr[i] = atoms[i].atomicnumber;
        }
        return scalars;
    }

    if (string(varname) == "restype")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            ptr[i] = atoms[i].residuenumber;
        }
        return scalars;
    }

    if (string(varname) == "resseq")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            ptr[i] = atoms[i].resseq;
        }
        return scalars;
    }

    if (string(varname) == "backbone")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            ptr[i] = atoms[i].backbone ? 1 : 0;
        }
        return scalars;
    }

    if (string(varname) == "compound")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            ptr[i] = atoms[i].compound;
        }
        return scalars;
    }

    if (string(varname) == "occupancy")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            ptr[i] = atoms[i].occupancy;
        }
        return scalars;
    }

    if (string(varname) == "tempFactor")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            ptr[i] = atoms[i].tempfactor;
        }
        return scalars;
    }

    if (string(varname) == "name")
    {
        vtkUnsignedCharArray *labels = vtkUnsignedCharArray::New();
        labels->SetNumberOfComponents(5);
        labels->SetNumberOfTuples(atoms.size());
        char *cptr = (char *)labels->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            memcpy(cptr, atoms[i].name, 5);
            cptr += 5;
        }
        return labels;       
    }

    if (string(varname) == "resname")
    {
        vtkUnsignedCharArray *labels = vtkUnsignedCharArray::New();
        labels->SetNumberOfComponents(4);
        labels->SetNumberOfTuples(atoms.size());
        char *cptr = (char *)labels->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            memcpy(cptr, atoms[i].resname, 4);
            cptr += 4;
        }
        return labels;       
    }

    if (string(varname) == "longresname")
    {
        vtkUnsignedCharArray *labels = vtkUnsignedCharArray::New();
        int maxlen = ResidueLongnameMaxlen() + 1;
        labels->SetNumberOfComponents(maxlen);
        labels->SetNumberOfTuples(atoms.size());
        char *cptr = (char *)labels->GetVoidPointer(0);
        memset(cptr, 0, maxlen*atoms.size());  // Initialize all of this for
                                            // purify, extents, etc.
        for (int i=0; i<atoms.size(); i++)
        {
            const char *n = ResiduenameToLongName(atoms[i].resname);
            if(n != 0)
                strcpy(cptr, n);
            else
                strcpy(cptr, atoms[i].resname);
            cptr += maxlen;
        }
        return labels;       
    }

    if (string(varname) == "elementname")
    {
        vtkUnsignedCharArray *labels = vtkUnsignedCharArray::New();
        labels->SetNumberOfComponents(3);
        labels->SetNumberOfTuples(atoms.size());
        char *cptr = (char *)labels->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            memcpy(cptr, atoms[i].element, 3);
            cptr += 3;
        }
        return labels;       
    }

    if (string(varname) == "compoundname")
    {
        int maxlen = 1;
        for (int j=0; j<compoundNames.size(); j++)
        {
            int l = compoundNames[j].length() + 1;
            if (l > maxlen)
                maxlen = l;
        }

        vtkUnsignedCharArray *labels = vtkUnsignedCharArray::New();
        labels->SetNumberOfComponents(maxlen);
        labels->SetNumberOfTuples(atoms.size());
        char *cptr = (char *)labels->GetVoidPointer(0);
        memset(cptr, 0, maxlen*atoms.size());  // Initialize all of this for
                                               // purify, extents, etc.
        for (int i=0; i<atoms.size(); i++)
        {
            strcpy(cptr, compoundNames[atoms[i].compound].c_str());
            cptr += maxlen;
        }
        return labels;       
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtProteinDataBankFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 17:49:30 EDT 2006
//    Bonds are now line segment cells, not an atom-centered 4-comp array.
//
// ****************************************************************************

vtkDataArray *
avtProteinDataBankFileFormat::GetVectorVar(const char *varname)
{
    return NULL;
}


// ****************************************************************************
//  Function:  AtomsShouldBeBonded
//
//  Purpose:
//    Simple but effective test to see if two atoms are bonded.
//
//  Arguments:
//    atoms      all atoms
//    a1,a2      two atom indices
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************

static bool
AtomsShouldBeBonded(const vector<Atom> &atoms, int a1, int a2)
{
    float dx = atoms[a1].x - atoms[a2].x;
    float dy = atoms[a1].y - atoms[a2].y;
    float dz = atoms[a1].z - atoms[a2].z;
    float dist2 = dx*dx + dy*dy + dz*dz;
    if (dist2 > .4*.4)
    {
        if (atoms[a1].atomicnumber==1 ||
            atoms[a2].atomicnumber==1)
        {
            if (dist2 < 1.2*1.2)
            {
                return true;
            }
        }
        else
        {
            if (dist2 < 1.9*1.9)
            {
                return true;
            }
        }
    }
    return false;
}

// ****************************************************************************
//  Method:  avtProteinDataBankFileFormat::CreateBondsFromModel_Slow
//
//  Purpose:
//    Search all appropriate atom pairs for bonds using a slower algorithm.
//
//  Arguments:
//    model      the model index to use for distance tests
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 17:49:30 EDT 2006
//    Bonds are now line segment cells, not an atom-centered 4-comp array.
//
// ****************************************************************************
void
avtProteinDataBankFileFormat::CreateBondsFromModel_Slow(int model)
{
    // We should only have to create bonds once for all models
    if (bonds.size() > 0)
        return;

    vector<Atom> &atoms = allatoms[model];
    int natoms = atoms.size();
    bonds.reserve(natoms);  // just a guess

    //
    // This is an N^2 algorithm.  Slow, but safe.
    // Don't use it unless there's something wrong
    // with the fast one.
    //
    for (int i=0; i<natoms; i++)
    {
        for (int j=0; j<i; j++)
        {
            if (AtomsShouldBeBonded(atoms,i,j))
            {
                bonds.push_back(pair<int,int>(i,j));
            }
        }
    }
}

// ****************************************************************************
//  Method:  avtProteinDataBankFileFormat::CreateBondsFromModel_Fast
//
//  Purpose:
//    Search all appropriate atom pairs for bonds using a faster algorithm.
//
//  Arguments:
//    model      the model index to use for distance tests
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 17:49:30 EDT 2006
//    Bonds are now line segment cells, not an atom-centered 4-comp array.
//
//    Jeremy Meredith, Wed Apr 18 11:02:04 EDT 2007
//    Account for potentially zero atoms.  This only seemed to appear
//    when there was a parsing problem with the file, so maybe it should
//    be changed to throw an error in ReadAtomsForModel()?
//
// ****************************************************************************
void
avtProteinDataBankFileFormat::CreateBondsFromModel_Fast(int model)
{
    // We should only have to create bonds once for all models
    if (bonds.size() > 0)
        return;

    vector<Atom> &atoms = allatoms[model];
    int natoms = atoms.size();
    if (natoms <= 0)
        return;

    //
    // The strategy here is to divide atoms into 3D spatial bins
    // and compare atoms in some (i,j,k) bin with all atoms in
    // the 27 surrounding bins -- i.e. (i-1,j-1,k-1) thru
    // (i+1,j+1,k+1) -- to find ones that should be bonded.
    //
    // This means that the size of each bin, determined by
    // the value "maxBondDist", should truly at least as
    // large as the maximum bond distance, or else this
    // will fail to catch some bonds.
    //
    // Simultaneously, setting maxBondDist too large means
    // that too many atoms must be compared for bonds, and
    // will likely slow down the algorithm.
    //
    float maxBondDist = 3.0;

    float minx =  FLT_MAX;
    float maxx = -FLT_MAX;
    float miny =  FLT_MAX;
    float maxy = -FLT_MAX;
    float minz =  FLT_MAX;
    float maxz = -FLT_MAX;

    for (int a=0; a<natoms; a++)
    {
        Atom &atom = atoms[a];
        if (atom.x < minx)
            minx = atom.x;
        if (atom.x > maxx)
            maxx = atom.x;
        if (atom.y < miny)
            miny = atom.y;
        if (atom.y > maxy)
            maxy = atom.y;
        if (atom.z < minz)
            minz = atom.z;
        if (atom.z > maxz)
            maxz = atom.z;
    }

    float szx = maxx - minx;
    float szy = maxy - miny;
    float szz = maxz - minz;

    int ni = 1 + int(szx / maxBondDist);
    int nj = 1 + int(szy / maxBondDist);
    int nk = 1 + int(szz / maxBondDist);

    //
    // I know -- I'm using a grid of STL vectors, and this
    // could potentially be inefficient, but I'll wait until
    // I see a problem with this strategy before I change it.
    //
    typedef vector<int> intvec;
    intvec *atomgrid = new intvec[ni*nj*nk];

    for (int a=0; a<natoms; a++)
    {
        Atom &atom = atoms[a];
        int ix = int((atom.x - minx) / maxBondDist);
        int jx = int((atom.y - miny) / maxBondDist);
        int kx = int((atom.z - minz) / maxBondDist);
        atomgrid[ix + ni*(jx + nj*(kx))].push_back(a);
    }

    for (int i=0; i<ni; i++)
    {
        for (int j=0; j<nj; j++)
        {
            for (int k=0; k<nk; k++)
            {
                int index1 = i + ni*(j + nj*(k));
                int na = atomgrid[index1].size();
                for (int a=0; a<na; a++)
                {
                    int ctr = 0;
                    int a1 = atomgrid[index1][a];
                    for (int p=-1; p<=1 && ctr<4; p++)
                    {
                        int ii = i+p;
                        if (ii<0 || ii>=ni)
                            continue;

                        for (int q=-1; q<=1 && ctr<4; q++)
                        {
                            int jj = j+q;
                            if (jj<0 || jj>=nj)
                                continue;

                            for (int r=-1; r<=1 && ctr<4; r++)
                            {
                                int kk = k+r;
                                if (kk<0 || kk>=nk)
                                    continue;

                                int index2 = ii + ni*(jj + nj*(kk));
                                int naa = atomgrid[index2].size();
                                for (int aa=0; aa<naa && ctr<4; aa++)
                                {
                                    if (index1==index2 && a==aa)
                                        continue;

                                    int a2 = atomgrid[index2][aa];

                                    // Only create one direction of
                                    // each bond pair
                                    if (a1 > a2)
                                        continue;

                                    if (AtomsShouldBeBonded(atoms,a1,a2))
                                    {
                                        bonds.push_back(pair<int,int>(a1,a2));
                                        ctr++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    delete[] atomgrid;
}


// ****************************************************************************
//  Method:  avtProteinDataBankFileFormat::ReadAllMetaData
//
//  Purpose:
//    Open the file and read the meta-data it contains.
//    There isn't much to do for this file format other than
//    count the number of models.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Apr 18 10:59:48 EDT 2007
//    Files with non-unixy text formatting (^M's at the end of every line)
//    required allowing for an extra character in getline.
//
//    Jeremy Meredith, Wed Oct 17 11:27:10 EDT 2007
//    Added compound support.
//
//    Jeremy Meredith, Thu Oct 18 16:31:20 EDT 2007
//    COMPND records can be multi-line; ignore all but the first line.
//
//    Jeremy Meredith, Mon Oct 22 12:58:00 EDT 2007
//    Explicitly make "no compound" part of the compound name array.
//    This makes getting the name for any particular compound number easier.
//
//    Jeremy Meredith, Thu Feb 12 12:33:54 EST 2009
//    Moved HETNAM parsing into this function so that we can create the
//    enumerated scalar to include new residue types defined in this file.
//
// ****************************************************************************
void
avtProteinDataBankFileFormat::ReadAllMetaData()
{
    if (metadata_read)
        return;

    OpenFileAtBeginning();

    metadata_read = true;

    char line[82];
    in.getline(line, 82);
    nmodels = 0;
    int titleLineCount = 0, sourceLineCount = 0;
    bool canReadCompounds = true;
    std::string hetnam, longhetnam;
    bool readingHetnam = false;
    std::string source;
    while (in)
    {
        string record(line,0,6);
        if (readingHetnam && record != "HETNAM")
        {
            debug4 << "Adding new residue name: " << hetnam.c_str()
                   << ", longname=" << longhetnam.c_str() << endl;
            AddResiduename(hetnam.c_str(), longhetnam.c_str());
            hetnam = "";
            longhetnam = "";
            readingHetnam = false;
        }

        if (record == "MODEL ")
        {
            // Count the models
            nmodels++;
            // Only read compound names once, even if there are multiple models
            if (compoundNames.size() > 0)
                canReadCompounds = false;
        }
        else if (record == "TITLE ")
        {
            dbTitle += "\n\t";
            dbTitle += string(line + ((titleLineCount > 0) ? 11 : 10));
            titleLineCount++;
        }
        else if (record == "SOURCE")
        {
            source += "\n\t";
            source += string(line + 10);
            sourceLineCount++;
        }
        else if (canReadCompounds &&
                 record == "COMPND" && line[8]==' ' && line[9]==' ')
        {
            if (compoundNames.size() == 0)
            {
                compoundNames.push_back("No compound");
            }
            compoundNames.push_back(string(line + 10));
        }
        else if (record == "HETNAM")
        {
            char het[4];
            memcpy(het, line + 11, 3);
            het[3] = '\0';

            if(hetnam != std::string(het))
            {
                if(readingHetnam)
                {
                    debug4 << "Adding new residue name: " << hetnam.c_str()
                           << ", longname=" << longhetnam.c_str() << endl;
                    AddResiduename(hetnam.c_str(), longhetnam.c_str());
                    longhetnam = "";
                }

                readingHetnam = true;
            }

            if(longhetnam.size() > 0)
                longhetnam += "\n";
            longhetnam += TrimTrailingSpaces(line + 15);
            hetnam = het;
        }
        in.getline(line, 82);
    }

    if(titleLineCount == 0 && sourceLineCount > 0)
        dbTitle = source;

    allatoms.resize(nmodels==0 ? 1 : nmodels);

    OpenFileAtBeginning();
}

// ****************************************************************************
//  Method:  avtProteinDataBankFileFormat::OpenFileAtBeginning
//
//  Purpose:
//    We don't want to close and re-open the file every time we want to
//    start back at the beginning, so we encapsulate the logic to both
//    ensure the file is still opened (in case it got closed or was never
//    opened) and to reset the flags and seek back to the beginning, in 
//    this function.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtProteinDataBankFileFormat::OpenFileAtBeginning()
{
    if (!in.is_open())
    {
        in.open(filename.c_str());
        if (!in)
        {
            EXCEPTION1(InvalidFilesException, filename.c_str());
        }
    }
    else
    {
        in.clear();
        in.seekg(0, ios::beg);
    }
}

// ****************************************************************************
//  Method:  avtProteinDataBankFileFormat::ReadAtomsForModel
//
//  Purpose:
//    Reads the atom records for the given model.
//
//  Arguments:
//    model      the zero-origin index of the model to read
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Brad Whitlock, Thu Mar 23 18:27:48 PST 2006
//    Added support for HETNAM.
//
//    Jeremy Meredith, Mon Aug 28 17:53:21 EDT 2006
//    Added support for CONECT records.
//
//    Jeremy Meredith, Wed Apr 18 10:59:48 EDT 2007
//    Files with non-unixy text formatting (^M's at the end of every line)
//    required allowing for an extra character in getline.
//
//    Jeremy Meredith, Wed Oct 17 11:27:10 EDT 2007
//    Added compound support.
//
//    Jeremy Meredith, Thu Oct 18 16:31:20 EDT 2007
//    COMPND records can be multi-line; ignore all but the first line.
//
//    Jeremy Meredith, Thu Feb 12 12:33:54 EST 2009
//    Moved HETNAM parsing out of this function, and into the meta-data
//    reading so that we can create the enumerated scalar with new residues.
//
// ****************************************************************************

void
avtProteinDataBankFileFormat::ReadAtomsForModel(int model)
{
    ReadAllMetaData();
    OpenFileAtBeginning();

    if (allatoms[model].size() > 0)
        return;

    vector<Atom> &atoms = allatoms[model];    
    atoms.clear();

    char line[82];
    in.getline(line, 82);

    if (nmodels != 0)
    {
        int curmodel = -1;
        while (in && curmodel < model)
        {
            string record(line,0,6);
            if (record == "MODEL ")
            {
                curmodel++;
            }
            in.getline(line, 82);
        }
    }

    int compound = 0;

    while (in)
    {
        string record(line,0,6);

        if (record == "ATOM  ")
        {
            Atom a(line, compound);
            atoms.push_back(a);
        }
        else if (record == "HETATM")
        {
            Atom a(line, compound);
            atoms.push_back(a);
        }
        else if (record == "ENDMDL")
        {
            break;
        }
        else if (record == "CONECT")
        {
            ConnectRecord c(line);
            connect.push_back(c);
            //c.Print(cout);
        }
        else if (record == "COMPND" && line[8]==' ' && line[9]==' ')
        {
            compound++;
        }
        else
        {
            // ignoring record type 'record'
        }

        in.getline(line, 82);
    }

    CreateBondsFromModel(model);
}

// ****************************************************************************
//  Method:  avtProteinDataBankFileFormat::CreateBondsFromModel
//
//  Purpose:
//    Create the bonds using a distance method.
//    It's disabled right now, but this is also where we would
//    add the bonds from the CONECT records.
//
//  Arguments:
//    model      the model index
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2006
//
// ****************************************************************************
void
avtProteinDataBankFileFormat::CreateBondsFromModel(int model)
{
    CreateBondsFromModel_Fast(model);
    
#if 0 // to generate bonds from CONECT records, re-enable this

    // NOTE: this needs to be updated to create bonds
    // as line segments instead of as a 4-comp cell array
    // before it will work.

    // ALSO: the conect records appear to reference atoms by
    // number only within the current compound; this should be
    // checked using a file with >1 compound
    for (int i=0; i<connect.size(); i++)
    {
        const ConnectRecord &c = connect[i];
        int a = c.a - 1; // ASSUME 1-origin atom sequence numbers

        int q = 0;
        for (int q=0; q < 4 && c.b[q] != -1; q++)
        {
            int b = c.b[q] - 1; // ASSUME 1-origin atom sequence numbers
            for (int p=0; p<4; p++)
            {
                if (bonds[p][a] == b)
                {
                    break;
                }

                if (bonds[p][a] == -1)
                {
                    bonds[p][a] = b;
                    break;
                }
            }
        }
    }
#endif
}

// ****************************************************************************
//  Method:  static Scan* functions
//
//  Purpose:
//    Fast functions to get the characters in a line by position.
//
//  Arguments:
//    line       input
//    len        lengths of input line
//    start      index of first character to extract
//    end        index of last character to extract
//    val        where to store the result
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2006
//
// ****************************************************************************
static inline void
ScanString(const char *line, int len, int start, int end, char *val)
{
    int i;
    int first = start - 1;
    for (i=first; i<end && i<len; i++)
    {
        val[i - first] = line[i];
    }
    val[i - first] = '\0';
}

static char tmpbuff[1024];

static inline void
ScanInt(const char *line, int len, int start, int end, int *val)
{
    int i;
    int first = start - 1;
    for (i=first; i<end && i<len; i++)
    {
        tmpbuff[i - first] = line[i];
    }
    tmpbuff[i - first] = '\0';
    *val = atoi(tmpbuff);
}

static inline void
ScanChar(const char *line, int len, int start, char *val)
{
    if (len < start)
        *val = '\0';
    else
        *val = tmpbuff[start-1];
}

static inline void
ScanFloat(const char *line, int len, int start, int end, float *val)
{
    int i;
    int first = start - 1;
    for (i=first; i<end && i<len; i++)
    {
        tmpbuff[i - first] = line[i];
    }
    tmpbuff[i - first] = '\0';

    //sscanf(tmpbuff, "%f", val);
    *val = atof(tmpbuff);
}

// ****************************************************************************
//  Constructor:  Atom::Atom
//
//  Arguments:
//    line       the line of text in a PDB file
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Brad Whitlock, Fri Jun 2 13:15:47 PST 2006
//    Added Jeremy's fix for yet another style of ATOM line.
//
//    Jeremy Meredith, Mon Aug 28 17:58:02 EDT 2006
//    Changed the scanning to (a) match the PDB spec document more 
//    effectively, (b) be faster, and (c) handle some missing elements
//    (short lines) better.
//
//    Jeremy Meredith, Wed Oct 17 11:27:10 EDT 2007
//    Added compound support.
//
// ****************************************************************************
Atom::Atom(const char *line, int cmpnd)
{
    char record[7];
    int len = strlen(line);
    ScanString(line, len,  1,  6,  record);
    ScanInt   (line, len,  7, 11, &serial);
    ScanString(line, len, 13, 16,  name);
    ScanChar  (line, len, 17,     &altloc);
    ScanString(line, len, 18, 20,  resname);
    ScanChar  (line, len, 22,     &chainid);
    ScanInt   (line, len, 23, 26, &resseq);
    ScanChar  (line, len, 27,     &icode);
    ScanFloat (line, len, 31, 38, &x);
    ScanFloat (line, len, 39, 46, &y);
    ScanFloat (line, len, 47, 54, &z);
    ScanFloat (line, len, 55, 60, &occupancy);
    ScanFloat (line, len, 61, 66, &tempfactor);
    ScanString(line, len, 73, 76,  segid);
    ScanString(line, len, 77, 78,  element);
    ScanString(line, len, 79, 80,  charge);

    // Left-justify element names
    if (element[0] == ' ')
    {
        element[0] = element[1];
        element[1] = '\0';
    }

    if((atomicnumber = ElementNameToAtomicNumber(element)) < 0)
    {
        // We have a weird file that does not keep the element name in
        // the place designated by the ATOM record. Files like this seem
        // to use the space for a line number. Check columns 12,13
        // for the atom number.
        if(line[12] == ' ' || (line[12] >= '0' && line[12] <= '9'))
        {
            element[0] = line[13];
            element[1] = '\0';
        }
        else if (line[13] >= '0' && line[13] <= '9')
        {
            element[0] = line[12];
            element[1] = '\0';
        }
        else
        {
            element[0] = line[12];
            element[1] = line[13];
        }

        atomicnumber = ElementNameToAtomicNumber(element);
        if (atomicnumber < 0 &&
            element[1] != '\0')
        {
            element[1] = '\0';
            atomicnumber = ElementNameToAtomicNumber(element);
        }

        if (atomicnumber < 0)
        {
            char msg[2000];
            SNPRINTF(msg, 2000, "Unknown element name <%s> in line: %s",
                     element, line);
            EXCEPTION1(VisItException, msg);
        }
    }

    // Shift spaces out of the resname.
    if(resname[0] == ' ')
    {
        if(resname[1] == ' ')
        {
            resname[0] = resname[2];
            resname[1] = '\0';
        }
        else
        {
            resname[0] = resname[1];
            resname[1] = resname[2];
            resname[2] = '\0';
        }
    }
    // Look up the residue number from the name.
    if((residuenumber = ResiduenameToNumber(resname)) < 0)
    {
        residuenumber = 0;
    }

    backbone = false;
    if (strcmp(name, " N  ")==0 ||
        strcmp(name, " C  ")==0 ||
        strcmp(name, " CA ")==0)
    {
        backbone = true;
    }

    compound = cmpnd;
}

// ****************************************************************************
//  Method:  Atom::Print
//
//  Purpose:
//    Print the atom to a stream.
//
//  Arguments:
//    out        the ostream.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void Atom::Print(ostream &out)
{
    out << "Atom:\n"
        << " serial   ="<<serial<<endl
        << " name     ="<<name<<endl
        << " altloc   ="<<altloc<<endl
        << " resname  ="<<resname<<endl
        << " chainid  ="<<chainid<<endl
        << " resseq   ="<<resseq<<endl
        << " icode    ="<<icode<<endl
        << " x        ="<<x<<endl
        << " y        ="<<y<<endl
        << " z        ="<<z<<endl
        << " occupancy="<<occupancy<<endl
        << " tempfact ="<<tempfactor<<endl
        << " segid    ="<<segid<<endl
        << " element  ="<<element<<endl
        << " charge   ="<<charge<<endl;
}


// ****************************************************************************
//  Constructor:  ConnectRecord::ConnectRecord
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2006
//
// ****************************************************************************
ConnectRecord::ConnectRecord(const char *origline)
{
    // We need to prevent this from trying to
    // skip over whitespace, as the last three
    // of these fields are optional, but there
    // may be more stuff later on the line.  This
    // probably means sscanf is not the best way
    // to accomplish this.
    char line[82];
    strcpy(line, origline);
    line[31] = '\0';

    char record[7];
    int n;
    b[0] = -1;
    b[1] = -1;
    b[2] = -1;
    b[3] = -1;
    n = sscanf(line, "%6c%5d%5d%5d%5d%5d",
               record,
               &a,
               &b[0], &b[1], &b[2], &b[3]);
}

// ****************************************************************************
//  Method:  ConnectRecord::Print
//
//  Purpose:
//    Print the connect record contets.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2006
//
// ****************************************************************************
void
ConnectRecord::Print(ostream &out)
{
    out << "Connect Record:\n"
        << "a  = "<<a<<endl
        << "b1 = "<<b[0]<<endl
        << "b2 = "<<b[1]<<endl
        << "b3 = "<<b[2]<<endl
        << "b4 = "<<b[3]<<endl;
}
