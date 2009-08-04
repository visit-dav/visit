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
//  File: avtSiloDumpFilter.C
// ************************************************************************* //

#include <avtSiloDumpFilter.h>
#include <VisItException.h>
#include <DebugStream.h>
#include <silo.h>
#include <vtkDataSet.h>
#include <vtkSystemIncludes.h> // for vtkIdType
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <algorithm>
#include <string>
using std::find;
using std::string;

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

static int  VTKZoneTypeToSiloZoneType(int);
static void TranslateVTKWedgeToSiloWedge(const vtkIdType [6], int *);
static void TranslateVTKPyramidToSiloPyramid(const vtkIdType [5], int *);
static void TranslateVTKPixelToSiloQuad(const vtkIdType [4], int *);
static void TranslateVTKVoxelToSiloHex(const vtkIdType [8], int *);

// ****************************************************************************
//  Function: VTKZoneTypeToSiloZoneType
//
//  Purpose:
//      Converts a zone type in VTK to a zone type in Silo.
//
//  Arguments:
//      zonetype      The zone type in VTK.
//
//  Returns:     The zone type in Silo.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  4, 2002
//
// ****************************************************************************

int
VTKZoneTypeToSiloZoneType(int zonetype)
{
    int  silo_zonetype = -1;

    switch (zonetype)
    {
      case VTK_POLYGON:
        silo_zonetype = DB_ZONETYPE_POLYGON;
        break;
      case VTK_TRIANGLE:
        silo_zonetype = DB_ZONETYPE_TRIANGLE;
        break;
      case VTK_QUAD:
        silo_zonetype = DB_ZONETYPE_QUAD;
        break;
      case VTK_PIXEL:
        silo_zonetype = DB_ZONETYPE_QUAD;
        break;
      case VTK_TETRA:
        silo_zonetype = DB_ZONETYPE_TET;
        break;
      case VTK_PYRAMID:
        silo_zonetype = DB_ZONETYPE_PYRAMID;
        break;
      case VTK_WEDGE:
        silo_zonetype = DB_ZONETYPE_PRISM;
        break;
      case VTK_HEXAHEDRON:
        silo_zonetype = DB_ZONETYPE_HEX;
        break;
      case VTK_VOXEL:
        silo_zonetype = DB_ZONETYPE_HEX;
        break;
      case VTK_LINE:
        silo_zonetype = DB_ZONETYPE_BEAM;
        break;
    }

    return silo_zonetype;
}

// ****************************************************************************
//  Function: TranslateVTKWedgeToSiloWedge
//
//  Purpose:
//      The silo and VTK wedges are stored differently; translate between them.
//
//  Arguments:
//      vtkWedge      The list of nodes in VTK ordering.
//      siloWedge     A list of nodes from a Silo node list.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  4, 2002
//
// ****************************************************************************

void
TranslateVTKWedgeToSiloWedge(const vtkIdType vtkWedge[6], int *siloWedge)
{
    //
    // The Silo wedge stores the four base nodes as 0, 1, 2, 3 and the two
    // top nodes as 4, 5.  The VTK wedge stores them as two triangles.  When
    // getting the exact translation, it is useful to look at the face lists
    // and edge lists in vtkWedge.cxx.
    //
    siloWedge[0] = vtkWedge[5];
    siloWedge[1] = vtkWedge[2];
    siloWedge[2] = vtkWedge[0];
    siloWedge[3] = vtkWedge[3];
    siloWedge[4] = vtkWedge[4];
    siloWedge[5] = vtkWedge[1];
}


// ****************************************************************************
//  Function: TranslateVTKPyramidToSiloPyramid
//
//  Purpose:
//    The silo and VTK pyramids are stored differently; translate between them.
//
//  Arguments:
//    vtkPyramid      The list of nodes in VTK ordering.
//    siloPyramid     A list of nodes from a Silo node list.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  4, 2002
//
// ****************************************************************************

void
TranslateVTKPyramidToSiloPyramid(const vtkIdType vtkPyramid[5], int *siloPyramid)
{
    //
    // The Silo pyramid stores the four base nodes as 0, 1, 2, 3 in 
    // opposite order from the VTK wedge. When getting the exact translation, 
    // it is useful to look at the face lists and edge lists in 
    // vtkPyramid.cxx.
    //
    siloPyramid[0] = vtkPyramid[0];
    siloPyramid[1] = vtkPyramid[3];
    siloPyramid[2] = vtkPyramid[2];
    siloPyramid[3] = vtkPyramid[1];
    siloPyramid[4] = vtkPyramid[4];
}


// ****************************************************************************
//  Function: TranslateVTKPixelToSiloQuad
//
//  Purpose:
//    Translates a rectilinear quad to an unstructured quad
//
//  Arguments:
//    vtkPixel     The list of nodes in VTK ordering.
//    siloQuad     A list of nodes from a Silo node list.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  5, 2002
//
// ****************************************************************************

void
TranslateVTKPixelToSiloQuad(const vtkIdType vtkPixel[4], int *siloQuad)
{
    // A pixel is indexed by i+2*j- a quad is normal ucd ordering
    siloQuad[0] = vtkPixel[0];
    siloQuad[1] = vtkPixel[1];
    siloQuad[2] = vtkPixel[3];
    siloQuad[3] = vtkPixel[2];
}


// ****************************************************************************
//  Function: TranslateVTKVoxelToSiloHex
//
//  Purpose:
//    Translates a rectilinear hex to an unstructured hex
//
//  Arguments:
//    vtkVoxel     The list of nodes in VTK ordering.
//    siloHex      A list of nodes from a Silo node list.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  5, 2002
//
// ****************************************************************************

void
TranslateVTKVoxelToSiloHex(const vtkIdType vtkVoxel[8], int *siloHex)
{
    // A pixel is indexed by i+2*j+4*k -- a hex is normal ucd ordering
    siloHex[0] = vtkVoxel[0];
    siloHex[1] = vtkVoxel[1];
    siloHex[2] = vtkVoxel[3];
    siloHex[3] = vtkVoxel[2];
    siloHex[4] = vtkVoxel[4];
    siloHex[5] = vtkVoxel[5];
    siloHex[6] = vtkVoxel[7];
    siloHex[7] = vtkVoxel[6];
}

// ****************************************************************************
//  Method: avtSiloDumpFilter constructor
//
//  Programmer: meredith -- generated by xml2info
//  Creation:   Fri Jan 4 12:12:54 PDT 2002
//
// ****************************************************************************

avtSiloDumpFilter::avtSiloDumpFilter()
{
}


// ****************************************************************************
//  Method: avtSiloDumpFilter destructor
//
//  Programmer: meredith -- generated by xml2info
//  Creation:   Fri Jan 4 12:12:54 PDT 2002
//
//  Modifications:
//
// ****************************************************************************

avtSiloDumpFilter::~avtSiloDumpFilter()
{
}


// ****************************************************************************
//  Method:  avtSiloDumpFilter::Create
//
//  Programmer: meredith -- generated by xml2info
//  Creation:   Fri Jan 4 12:12:54 PDT 2002
//
// ****************************************************************************

avtFilter *
avtSiloDumpFilter::Create()
{
    return new avtSiloDumpFilter();
}


// ****************************************************************************
//  Method:      avtSiloDumpFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: meredith -- generated by xml2info
//  Creation:   Fri Jan 4 12:12:54 PDT 2002
//
// ****************************************************************************

void
avtSiloDumpFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const SiloDumpAttributes*)a;
}


// ****************************************************************************
//  Method: avtSiloDumpFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtSiloDumpFilter with the given
//      parameters would result in an equivalent avtSiloDumpFilter.
//
//  Programmer: meredith -- generated by xml2info
//  Creation:   Fri Jan 4 12:12:54 PDT 2002
//
// ****************************************************************************

bool
avtSiloDumpFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(SiloDumpAttributes*)a);
}


// ****************************************************************************
//  Method: avtSiloDumpFilter::PreExecute
//
//  Purpose:
//      Opens the Silo file for the current processor.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  4, 2002
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtSiloDumpFilter::PreExecute()
{
    avtPluginDataTreeIterator::PreExecute();

    // set up the filename
    char file[256];
#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    sprintf(file, "%s-%05d.silo", atts.GetFilename().c_str(), rank);
#else
    sprintf(file, "%s.silo", atts.GetFilename().c_str());
#endif

    // create the file
    db = DBCreate(file, DB_CLOBBER, DB_LOCAL, "Silo dump", DB_PDB);

    // start with no domains
    domains.clear();
}


// ****************************************************************************
//  Method: avtSiloDumpFilter::ExecuteData
//
//  Purpose:
//      Dumps the input into the Silo file.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  4, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Jan  8 16:33:50 PST 2002
//    Added ghost zone creation support.
//    Added support for empty facelists (i.e. don't write 'em!!).
//
//    Hank Childs, Tue Jan  8 12:11:09 PST 2002
//    Fix memory leak.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Fri Aug 27 15:25:22 PDT 2004
//    Rename ghost data array.
//
//    Jeremy Meredith, Thu Aug  7 14:30:46 EDT 2008
//    Made string literal const char*, not char*.
//
// ****************************************************************************

vtkDataSet *
avtSiloDumpFilter::ExecuteData(vtkDataSet *in_ds, int domain, std::string)
{
    vtkDataSet *retval = atts.GetDisplay() ? in_ds : NULL;

    // Get the input data
    int in_ncells  = in_ds->GetNumberOfCells();
    int in_npoints = in_ds->GetNumberOfPoints();
    if (in_ncells == 0 || in_npoints == 0)
        return retval;

    vtkDataArray *in_ps = NULL;
    vtkDataArray *in_pv = NULL;
    vtkDataArray *in_cs = NULL;
    vtkDataArray *in_cv = NULL;
    unsigned char *in_gl = NULL;

    if (in_ds->GetPointData())
    {
        in_ps = in_ds->GetPointData()->GetScalars();
        in_pv = in_ds->GetPointData()->GetVectors();
    }
    if (in_ds->GetCellData())
    {
        in_cs = in_ds->GetCellData()->GetScalars();
        in_cv = in_ds->GetCellData()->GetVectors();
        vtkDataArray *tmp_gl = in_ds->GetCellData()->GetArray("avtGhostZones");
        in_gl = tmp_gl ? ((vtkUnsignedCharArray*)tmp_gl)->GetPointer(0) : NULL;
    }

    // Do the vectors  --  Ignore the vectors for now

    // Do the scalars
    const char *csname = in_cs ? in_cs->GetName() : NULL;
    const char *psname = in_ps ? in_ps->GetName() : NULL;
    if (!csname) csname = "cell_var";
    if (!psname) psname = "point_var";
    if (in_cs && (find(vars.begin(), vars.end(), string(csname)) == vars.end()))
        vars.push_back(csname);
    if (in_ps && (find(vars.begin(), vars.end(), string(psname)) == vars.end()))
        vars.push_back(psname);

    //
    // Create a Silo dataset and write it
    //

    // copy the point array
    int out_npoints = in_npoints;
    vector<float> out_x(out_npoints);
    vector<float> out_y(out_npoints);
    vector<float> out_z(out_npoints);
    for (int n=0; n<in_npoints; n++)
    {
        double point[3];
        in_ds->GetPoint(n, point);
        out_x[n] = point[0];
        out_y[n] = point[1];
        out_z[n] = point[2];
    }

    // copy the cell arrays
    vector<int>           out_shapetype;
    vector<int>           out_shapecnt;
    vector<int>           out_shapesize;
    vector< vector<int> > out_singlenodelist;
    int out_ncells    = 0;

    // create an input cellindex -> output cellindex map
    vector<int>           cellindexmap(in_ncells, -1);

    int last_out_type = -1;
    int last_nverts   = -1;
    int last_index    = -1;
    int n_real_cells  = 0;
    int n_total_cells = 0;
    int max_ghost_lvl = (in_gl ? 1 : 0);
    for (int gl=0; gl<=max_ghost_lvl; gl++)
    {
        for (int c=0; c<in_ncells; c++)
        {
            vtkCell *cell = in_ds->GetCell(c);
            int in_type  = cell->GetCellType();
            int out_type = VTKZoneTypeToSiloZoneType(in_type);
            int nverts   = cell->GetNumberOfPoints();

            // Make sure we know what to do with it
            if (out_type == -1)
            {
                debug1 << "avtSiloDumpFilter: Could not translate an input "
                          "cell into an output cell.";
                continue;
            }

            // Only do real/ghosts as appropriate (if doing two-pass)
            if (in_gl && in_gl[c] != gl)
            {
                continue;
            }

            // We will use the cell

            // Find the right place to put it
            int index;
            if (last_out_type == out_type &&
                last_nverts   == nverts)
            {
                // use the last cached index if it matches
                index = last_index;
            }
            else
            {
                // not a match; actually find the right one
                for (index = 0; index < out_shapetype.size(); index++)
                {
                    if (out_shapetype[index] == out_type &&
                        out_shapesize[index] == nverts)
                    {
                        break;
                    }
                }
                // doesn't exist yet; create a new spot
                if (index == out_shapetype.size())
                {
                    out_shapetype.push_back(out_type);
                    out_shapesize.push_back(nverts);
                    out_shapecnt.push_back(0);
                    out_singlenodelist.push_back(vector<int>());
                }
            }
            cellindexmap[c] = out_ncells;
            out_shapecnt[index]++;
            out_ncells++;

            // Set the ids properly
            vtkIdType *in_ids  = cell->GetPointIds()->GetPointer(0);
            int        out_ids[1024];
            if (in_type == VTK_WEDGE)
            {
                TranslateVTKWedgeToSiloWedge(in_ids, out_ids);
            }
            else if (in_type == VTK_PYRAMID)
            {
                TranslateVTKPyramidToSiloPyramid(in_ids, out_ids);
            }
            else if (in_type == VTK_PIXEL)
            {
                TranslateVTKPixelToSiloQuad(in_ids, out_ids);
            }
            else if (in_type == VTK_VOXEL)
            {
                TranslateVTKVoxelToSiloHex(in_ids, out_ids);
            }
            else
            {
                for (int v=0; v<nverts; v++)
                    out_ids[v] = in_ids[v];
            }

            // Add to the right nodelist
            for (int v=0; v<nverts; v++)
                out_singlenodelist[index].push_back(out_ids[v]);

            // Cache our findings
            last_out_type = out_type;
            last_nverts   = nverts;
            last_index    = index;
        }
        if (gl == 0)
            n_real_cells = out_ncells;
    }
    n_total_cells = out_ncells;

    // create the big nodelist
    vector<int>           out_nodelist;
    for (int s=0; s<out_shapetype.size(); s++)
    {
        for (int v=0; v<out_singlenodelist[s].size(); v++)
            out_nodelist.push_back(out_singlenodelist[s][v]);
    }

    if (out_nodelist.size() == 0)
        return retval;

    // Make the directory
    char dir[256];
    sprintf(dir, "domain_%05d", domain);
    DBMkDir(db, dir);
    DBSetDir(db, dir);

    // Write out the zone list
    int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    DBPutZonelist2(db, "zl", out_ncells, ndims,
                   &out_nodelist[0],
                   out_nodelist.size(), 0/*origin*/,
                   0, n_total_cells - n_real_cells,
                   &out_shapetype[0],
                   &out_shapesize[0],
                   &out_shapecnt[0],
                   out_shapetype.size(), NULL);

    // Write out the face list
    const char *flname = NULL;
    if (ndims == 3)
    {
        DBfacelist *fl;
        fl = DBCalcExternalFacelist2(&out_nodelist[0], out_nodelist.size(),
                                     0,n_total_cells - n_real_cells,
                                     0,
                                     &out_shapetype[0], &out_shapesize[0],
                                     &out_shapecnt[0], out_shapetype.size(),
                                     NULL, 0);
        if (fl->nfaces > 0)
        {
            flname = "fl";
            DBPutFacelist(db, "fl", fl->nfaces, ndims,
                          fl->nodelist, fl->lnodelist, fl->origin, fl->zoneno,
                          fl->shapesize, fl->shapecnt, fl->nshapes, fl->types,
                          fl->typelist, fl->ntypes);
        }
        DBFreeFacelist(fl);
    }

    // Write out the mesh
    float *out_coords[3] = {&out_x[0], &out_y[0], &out_z[0]};
    DBPutUcdmesh(db, "mesh", ndims, NULL, out_coords,
                 out_npoints, out_ncells, "zl", flname, DB_FLOAT, NULL);

    // Write out the data arrays
    if (in_cs)
    {
        vector<float> array(n_total_cells);
        for (int c=0; c<in_ncells; c++)
        {
            int d = cellindexmap[c];
            if (d >= 0)
                array[d] = ((float*)in_cs->GetVoidPointer(0))[c];
        }
        DBPutUcdvar1(db, (char*)csname, "mesh", 
                     &array[0],
                     out_ncells, NULL,0, DB_FLOAT, DB_ZONECENT, NULL);
    }
    if (in_ps)
    {
        DBPutUcdvar1(db, (char*)psname, "mesh", 
                     (float*)in_ps->GetVoidPointer(0),
                     out_npoints, NULL,0, DB_FLOAT, DB_NODECENT, NULL);
    }

    // Add to our domains
    domains.push_back(domain);

    DBSetDir(db, "..");

    return retval;
}


// ****************************************************************************
//  Method: avtSiloDumpFilter::PostExecute
//
//  Purpose:
//      Writes multimeshes into the Silo file, or into a new root file
//      if in parallel.  Closes Silo files.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  4, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Jan  8 16:34:39 PST 2002
//    Added support for variables which don't exist on the first processor.
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
avtSiloDumpFilter::PostExecute()
{
    int i;
#ifdef PARALLEL
    DBClose(db);

    int rank;
    int size;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    MPI_Comm_size(VISIT_MPI_COMM, &size);
    int mpiDomListSizeTag = GetUniqueMessageTag();
    int mpiDomListTag     = GetUniqueMessageTag();
    int mpiVarListSizeTag = GetUniqueMessageTag();
    int mpiVarNameSizeTag = GetUniqueMessageTag();
    int mpiVarNameTag     = GetUniqueMessageTag();

    if (rank!=0)
    {
        int nd = domains.size();
        MPI_Send(&nd, 1, MPI_INT, 0, mpiDomListSizeTag, VISIT_MPI_COMM);
        if (nd)
            MPI_Send(&domains[0], nd, MPI_INT, 0, mpiDomListTag, VISIT_MPI_COMM);

        int nv = vars.size();
        MPI_Send(&nv, 1, MPI_INT, 0, mpiVarListSizeTag, VISIT_MPI_COMM);
        for (int v=0; v<nv; v++)
        {
            int len = vars[v].length()+1;
            MPI_Send(&len, 1, MPI_INT, 0, mpiVarNameSizeTag, VISIT_MPI_COMM);
            MPI_Send((char*)vars[v].c_str(), len, MPI_CHAR, 0, mpiVarNameTag, VISIT_MPI_COMM);
        }
        return;
    }

    vector<vector<int> > all_domains(size);
    all_domains[0] = domains;
    for (i=1; i<size; i++)
    {
        MPI_Status stat;
        int nd;
        MPI_Recv(&nd, 1, MPI_INT, i, mpiDomListSizeTag, VISIT_MPI_COMM, &stat);
        if (nd)
        {
            all_domains[i].resize(nd);
            MPI_Recv(&all_domains[i][0], nd, MPI_INT, i, mpiDomListTag,
                     VISIT_MPI_COMM, &stat);
        }

        int nv;
        MPI_Recv(&nv, 1, MPI_INT, i, mpiVarListSizeTag, VISIT_MPI_COMM, &stat);
        for (int v=0; v<nv; v++)
        {
            int  len;
            char var[1024];
            MPI_Recv(&len, 1, MPI_INT, i, mpiVarNameSizeTag, VISIT_MPI_COMM, &stat);
            MPI_Recv(var, len, MPI_CHAR, i, mpiVarNameTag, VISIT_MPI_COMM, &stat);

            if (find(vars.begin(), vars.end(), string(var)) == vars.end())
                vars.push_back(var);
        }
    }

    char file[256];
    sprintf(file, "%s.root", atts.GetFilename().c_str());
    db = DBCreate(file, DB_CLOBBER, DB_LOCAL, "2-domain ghost test", DB_PDB);
#endif

    vector<char *> mname;
    vector<int>    mtype;

    mname.clear();
    mtype.clear();
#ifdef PARALLEL
    for (int p=0; p<all_domains.size(); p++)
    {
        for (i=0; i<all_domains[p].size(); i++)
        {
            char *str = new char[256];
            sprintf(str, "%s-%05d.silo:domain_%05d/mesh",
                    atts.GetFilename().c_str(), p, all_domains[p][i]);
            mname.push_back(str);
            mtype.push_back(DB_UCDMESH);
        }
    }
#else
    for (i=0; i<domains.size(); i++)
    {
        char *str = new char[256];
        sprintf(str, "domain_%05d/mesh", domains[i]);
        mname.push_back(str);
        mtype.push_back(DB_UCDMESH);
    }
#endif
    DBPutMultimesh(db, "mesh", mname.size(),
                   (char**)&mname[0], &mtype[0], NULL);
    for (i=0; i<mname.size(); i++)
        delete[] mname[i];

    for (int v=0; v<vars.size(); v++)
    {
        mname.clear();
        mtype.clear();
#ifdef PARALLEL
        for (int p=0; p<all_domains.size(); p++)
        {
            for (i=0; i<all_domains[p].size(); i++)
            {
                char *str = new char[256];
                sprintf(str, "%s-%05d.silo:domain_%05d/%s",
                        atts.GetFilename().c_str(), p, all_domains[p][i],
                        vars[v].c_str());
                mname.push_back(str);
                mtype.push_back(DB_UCDVAR);
            }
        }
#else
        for (i=0; i<domains.size(); i++)
        {
            char *str = new char[256];
            sprintf(str, "domain_%05d/%s", domains[i], vars[v].c_str());
            mname.push_back(str);
            mtype.push_back(DB_UCDVAR);
        }
#endif
        DBPutMultivar(db, (char*)vars[v].c_str(), mname.size(),
                       (char**)&mname[0], &mtype[0], NULL);
        for (i=0; i<mname.size(); i++)
            delete[] mname[i];
    }

    DBClose(db);
}
