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

// ************************************************************************* //
//                            avtProteinDataBankFileFormat.C                 //
// ************************************************************************* //

#include <avtProteinDataBankFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
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
// ****************************************************************************

avtProteinDataBankFileFormat::avtProteinDataBankFileFormat(const char *fn,
                                                DBOptionsAttributes *readOpts)
    : avtMTSDFileFormat(&fn, 1)
{
    filename = fn;
    OpenFileAtBeginning();

    nmodels = 0;
    metadata_read = false;
    dbTitle = "";
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
// ****************************************************************************

int
avtProteinDataBankFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return nmodels==0 ? 1 : nmodels;
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
// ****************************************************************************

void
avtProteinDataBankFileFormat::FreeUpResources(void)
{
    in.close();

    bonds[0].clear();
    bonds[1].clear();
    bonds[2].clear();
    bonds[3].clear();
    for (int i=0; i<allatoms.size(); i++)
    {
        allatoms[i].clear();
    }
    allatoms.clear();

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
// ****************************************************************************

void
avtProteinDataBankFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    ReadAllMetaData();
    md->SetDatabaseComment(dbTitle);

    AddMeshToMetaData(md, "mesh",  AVT_POINT_MESH,
                      NULL, 1, 0, 3, 0);

    AddScalarVarToMetaData(md, "element", "mesh", AVT_ZONECENT);
    AddScalarVarToMetaData(md, "restype", "mesh", AVT_ZONECENT);
    AddScalarVarToMetaData(md, "resseq",  "mesh", AVT_ZONECENT);
    AddScalarVarToMetaData(md, "backbone","mesh", AVT_ZONECENT);
    AddArrayVarToMetaData(md, "bonds", 4, "mesh", AVT_ZONECENT);

    // Add a couple of label variables.
    md->Add(new avtLabelMetaData("name", "mesh", AVT_ZONECENT));
    md->Add(new avtLabelMetaData("resname", "mesh", AVT_ZONECENT));
    md->Add(new avtLabelMetaData("longresname", "mesh", AVT_ZONECENT));
    md->Add(new avtLabelMetaData("elementname", "mesh", AVT_ZONECENT));
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
// ****************************************************************************

vtkDataSet *
avtProteinDataBankFileFormat::GetMesh(int timestate, const char *meshname)
{
    int model = timestate;
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
 
    vtkCellArray *verts = vtkCellArray::New();
    pd->SetVerts(verts);
    for (int k = 0 ; k < atoms.size() ; k++)
    {
        verts->InsertNextCell(1);
        verts->InsertCellPoint(k);
    }

    pts->Delete();
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
// ****************************************************************************

vtkDataArray *
avtProteinDataBankFileFormat::GetVar(int timestate, const char *varname)
{
    int model = timestate;
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
// ****************************************************************************

vtkDataArray *
avtProteinDataBankFileFormat::GetVectorVar(int timestate, const char *varname)
{
    int model = timestate;
    ReadAtomsForModel(model);

    vector<Atom> &atoms = allatoms[model];

    if (string(varname) == "bonds")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfComponents(4);
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            ptr[i*4 + 0] = bonds[0][i];
            ptr[i*4 + 1] = bonds[1][i];
            ptr[i*4 + 2] = bonds[2][i];
            ptr[i*4 + 3] = bonds[3][i];
        }
        return scalars;
    }

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
// ****************************************************************************
void
avtProteinDataBankFileFormat::CreateBondsFromModel_Slow(int model)
{
    // We should only have to create bonds once for all models
    if (bonds[0].size() > 0)
        return;

    vector<Atom> &atoms = allatoms[model];
    int natoms = atoms.size();
    bonds[0].resize(natoms, -1);
    bonds[1].resize(natoms, -1);
    bonds[2].resize(natoms, -1);
    bonds[3].resize(natoms, -1);

    //
    // This is an N^2 algorithm.  Slow, but safe.
    // Don't use it unless there's something wrong
    // with the fast one.
    //
    for (int i=0; i<natoms; i++)
    {
        int ctr = 0;
        for (int j=0; j<natoms && ctr<4; j++)
        {
            if (i==j)
                continue;

            float dx = atoms[i].x - atoms[j].x;
            float dy = atoms[i].y - atoms[j].y;
            float dz = atoms[i].z - atoms[j].z;
            float dist2 = dx*dx + dy*dy + dz*dz;
            if (AtomsShouldBeBonded(atoms,i,j))
            {
                bonds[ctr++][i] = j;
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
// ****************************************************************************
void
avtProteinDataBankFileFormat::CreateBondsFromModel_Fast(int model)
{
    // We should only have to create bonds once for all models
    if (bonds[0].size() > 0)
        return;

    vector<Atom> &atoms = allatoms[model];
    int natoms = atoms.size();

    bonds[0].resize(natoms, -1);
    bonds[1].resize(natoms, -1);
    bonds[2].resize(natoms, -1);
    bonds[3].resize(natoms, -1);

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

                                    if (AtomsShouldBeBonded(atoms,a1,a2))
                                    {
                                        bonds[ctr++][a1] = a2;
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
// ****************************************************************************
void
avtProteinDataBankFileFormat::ReadAllMetaData()
{
    if (metadata_read)
        return;

    OpenFileAtBeginning();

    metadata_read = true;

    char line[81];
    in.getline(line, 81);
    nmodels = 0;
    int titleLineCount = 0, sourceLineCount = 0;
    std::string source;
    while (in)
    {
        string record(line,0,6);
        if (record == "MODEL ")
        {
            // Count the models
            nmodels++;
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
        in.getline(line, 81);
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

    char line[81];
    in.getline(line, 81);

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
            in.getline(line, 81);
        }
    }

    std::string hetnam, longhetnam;
    bool readingHetnam = false;

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

        if (record == "ATOM  ")
        {
            Atom a(line);
            atoms.push_back(a);
        }
        else if (record == "HETATM")
        {
            Atom a(line);
            atoms.push_back(a);
        }
        else if (record == "ENDMDL")
        {
            break;
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
        else
        {
            // ignoring record type 'record'
        }

        in.getline(line, 81);
    }

    CreateBondsFromModel_Fast(model);
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
// ****************************************************************************
Atom::Atom(const char *line)
{
    char record[7];
    sscanf(line,
           "%6c%5d%*1c%4c%c%3c%*1c%1c%4d%c%*3c%8f%8f%8f%6f%6f%*6c%4c%2c%2c",
           record,
           &serial,
           name,
           &altloc,
           resname,
           &chainid,
           &resseq,
           &icode,
           &x,
           &y,
           &z,
           &occupancy,
           &tempfactor,
           segid,
           element,
           charge);
    name[4] = '\0';
    resname[3] = '\0';
    segid[4] = '\0';
    element[2] = '\0';
    charge[3] = '\0';

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

        if((atomicnumber = ElementNameToAtomicNumber(element)) < 0)
        {
            char msg[2000];
            SNPRINTF(msg, 2000, "Unknown element name in line: %s", line);
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
        debug4 << "Unknown residue name in line: " << line << endl;
    }

    backbone = false;
    if (strcmp(name, " N  ")==0 ||
        strcmp(name, " C  ")==0 ||
        strcmp(name, " CA ")==0)
    {
        backbone = true;
    }
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

