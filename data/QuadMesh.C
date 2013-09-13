/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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
#include <cstring>
#include <QuadMesh.h>
#include <visitstream.h>

//
// Template function for memory reallocation.
//

template <class T>
T *remake(T *ptr, int oldsize, int size)
{
    T *retval = new T[size];
    T *iptr = retval;
    for(int i = 0; i < oldsize; ++i)
        *iptr++ = ptr[i];
    delete [] ptr;
    return retval;
}

// ****************************************************************************
// Class: QuadMesh::MaterialList
//
// Purpose:
//   Keeps track of mixed material information.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:35 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Mar 17 16:43:34 PST 2004
//   I fixed an off by one error.
//
// ****************************************************************************

class QuadMesh::MaterialList
{
public:
    MaterialList() : matNames()
    {
        have_mixed = false;
        mix_zone = NULL;
        mix_mat = NULL;
        mix_vf = NULL;
        mix_next = NULL;
        matlist = NULL;

        // initialize private members.
        _array_size = 0;
        _array_index = 1;
        _array_growth = 1000;
    }

    ~MaterialList()
    {
       delete [] matlist;
       if (have_mixed)
       {
           delete [] mix_zone;   
           delete [] mix_mat;    
           delete [] mix_vf;      
           delete [] mix_next;    
       }
    }

    void AddMaterial(const std::string &mat)
    {
        matNames.push_back(mat);
    }

    void AddClean(int siloZone, int matNumber)
    {
        matlist[siloZone] = matNumber;
    }

    void AddMixed(int siloZone, int *matNumbers, double *matVf, int nMats)
    {
        int i;

        /* Grow the arrays if they will not fit nMats materials. */
        Resize(nMats);

        /* Record the mixed zone as a negative offset into the mix arrays. */
        matlist[siloZone] = -_array_index;

        /* Update the mix arrays. */
        for(i = 0; i < nMats; ++i)
        {
            int index = _array_index - 1;

            mix_zone[index] = siloZone;
            mix_mat[index]  = matNumbers[i];
            mix_vf[index]   = matVf[i];

            if(i < nMats - 1)
                mix_next[index] = index + 2;
            else
                mix_next[index] = 0;

            ++(_array_index);
        }

        /* indicate that we have mixed materials. */
        have_mixed = true;
    }

    void AllocClean(int nZones)
    {
        matlist = new int[nZones];
    }

    int GetMixedSize() const { return _array_index - 1; };

    void WriteMaterial(DBfile *db, const std::string &matvarname, 
                       const std::string &meshName, int nx, int ny, int nz)
    {
        size_t i;
        int mdims[3] = {nx,ny,nz};
        int ndims = (nz > 0) ? 3 : 2;

        /* Create a 1..nTotalMaterials material number array. */
        int *allmats = new int[matNames.size()];
        for(i = 0; i < matNames.size(); ++i)
            allmats[i] = i + 1;

        DBoptlist *optList = DBMakeOptlist(2);

        // Add material names.
        char **matnames = new char *[4];
        for(i = 0; i < matNames.size(); ++i)
            matnames[i] = (char *)matNames[i].c_str();
        DBAddOption(optList, DBOPT_MATNAMES, matnames);

        if (have_mixed)
        {
            DBPutMaterial(db, (char *)matvarname.c_str(), (char *)meshName.c_str(),
                          matNames.size(), allmats,
                          matlist, mdims, ndims, mix_next,
                          mix_mat, mix_zone,
                          (float*)mix_vf , GetMixedSize(),
                          DB_DOUBLE, optList);
        }
        else
        {         
            DBPutMaterial(db, (char *)matvarname.c_str(), (char *)meshName.c_str(),
                          matNames.size(), allmats,
                          matlist, mdims, ndims, NULL,
                          NULL, NULL, NULL , 0,
                          DB_INT, optList);
        }

        DBFreeOptlist(optList);
        delete [] matnames;
        delete [] allmats;
    }
private:
    void Resize(int nMats)
    { 
        if(_array_index + nMats >= _array_size)
        {
            int new_size = _array_size + _array_growth;

            if(_array_size == 0)
            {
                /* Reallocate arrays in large increments. */
                mix_zone = new int[new_size];
                mix_mat  = new int[new_size];
                mix_vf   = new double[new_size];
                mix_next = new int[new_size];
            }
            else
            {
                /* Reallocate arrays in large increments. */
                mix_zone = remake(mix_zone, _array_size, new_size);
                mix_mat  = remake(mix_mat, _array_size,  new_size);
                mix_vf   = remake(mix_vf, _array_size, new_size);
                mix_next = remake(mix_next, _array_size, new_size);
            }

            _array_size = new_size;
        }
    }

    int    have_mixed;
    int    *mix_zone;
    int    *mix_mat;
    double *mix_vf;
    int    *mix_next;
    int    *matlist;
    int    _array_size;
    int    _array_index;
    int    _array_growth;
    std::vector<std::string> matNames;
};

// ****************************************************************************
//  Function:  SplitValues
//
//  Purpose:
//    Separate a string into a vector of strings using a single char delimiter.
//
//  Arguments:
//    buff       the string to split
//    delim      the single-character delimiter
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2004
//
// ****************************************************************************
std::vector<std::string>
SplitValues(const std::string &buff, char delim)
{
    std::vector<std::string> output;
    
    std::string tmp="";
    for (size_t i=0; i<buff.length(); i++)
    {
        if (buff[i] == delim)
        {
            if (!tmp.empty())
                output.push_back(tmp);
            tmp = "";
        }
        else
        {
            tmp += buff[i];
        }
    }
    if (!tmp.empty())
        output.push_back(tmp);

    return output;
}

// ****************************************************************************
// Function: BeginVar
//
// Purpose: 
//   Splits a name containing '/' into separate names and makes Silo directories
//   for the names up to the last name, which is the name of an object.
//
// Arguments:
//   dbfile  : The Silo file.
//   name    : The name to be split.
//   nlevels : The number of levels in the name.
//
// Returns:    The object name.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 6 15:31:08 PST 2009
//
// Modifications:
//   
// ****************************************************************************

std::string 
BeginVar(DBfile *dbfile, const std::string &name, int &nlevels)
{
    std::vector<std::string> names = SplitValues(name, '/');
    nlevels = 0;
    for(size_t i = 0; i < names.size()-1; ++i)
    {
        int t = DBInqVarType(dbfile, names[i].c_str());
        if(t != DB_DIR)
            DBMkDir(dbfile, names[i].c_str());
        DBSetDir(dbfile, names[i].c_str());
        ++nlevels;
    }
    std::string s(name);
    if(names.size() > 0)
        s = names[names.size()-1];
    return s;
}

// ****************************************************************************
// Function: EndVar
//
// Purpose: 
//   Back out of Silo subdirectories when we're done writing a variable.
//
// Arguments:
//   dbfile : The Silo file.
//   nlevels : The number of levels to back up.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 6 15:34:57 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
EndVar(DBfile *dbfile, int nlevels)
{
    for(int i = 0; i < nlevels; ++i)
        DBSetDir(dbfile, "..");
}

// ****************************************************************************
// Class: QuadMesh::VectorData
//
// Purpose:
//   This class stores vector data for QuadMesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:59 PST 2002
//
// Modifications:
//
// ****************************************************************************

class QuadMesh::VectorData
{
public:
    VectorData(const std::string &n, int nx, int ny, int nz, bool node) : name(n), units()
    {
        xdim = nx;
        ydim = ny;
        zdim = nz;
        int ndata;
        if(node)
            ndata = xdim * ydim * zdim;
        else
        {
            int zzones = (zdim > 1) ? (zdim-1) : 1;
            ndata = (xdim-1) * (ydim-1) * zzones;
        }
        xd = new float[ndata];
        yd = new float[ndata];
        zd = new float[ndata];
        nodal = node;
    }

    ~VectorData()
    {
        delete [] xd;
        delete [] yd;
        delete [] zd;
    }

    void SetUnits(const std::string &u) { units = u; }
    const std::string &GetUnits() const { return units; }

    void SetZonalValue(int x, int y, int z, float val[3])
    {
        int index = z*((ydim-1)*(xdim-1)) + y*(xdim-1) + x;
        xd[index] = val[0];
        yd[index] = val[1];
        zd[index] = val[2];
    }

    void SetNodalValue(int x, int y, int z, float val[3])
    {
        int index = z*(ydim*xdim) + y*xdim + x;
        xd[index] = val[0];
        yd[index] = val[1];
        zd[index] = val[2];
    }

    void WriteFile(DBfile *db, const std::string &meshName)
    {
         // Create subvar names.
         std::string xvar(name + std::string("_X"));
         std::string yvar(name + std::string("_Y"));
         std::string zvar(name + std::string("_Z"));
         char *varnames[3];
         varnames[0] = (char *)xvar.c_str();
         varnames[1] = (char *)yvar.c_str();
         varnames[2] = (char *)zvar.c_str();
         float *vars[] = {xd, yd, zd};

         int nlevels = 0;
         std::string varName = BeginVar(db, name, nlevels);

         DBoptlist *optList = NULL;
         if(units.size() > 0)
         {
             optList = DBMakeOptlist(2);
             DBAddOption(optList, DBOPT_UNITS, (void*)units.c_str());
         }
         int sdims = (zdim>1) ? 3 : 2;
         if(nodal)
         {
             int ndims[] = {xdim, ydim, zdim};
             DBPutQuadvar(db, (char *)varName.c_str(), (char *)meshName.c_str(),
                          3, varnames, vars, ndims, sdims,  NULL, 0, DB_FLOAT,
                          DB_NODECENT, optList);
         }
         else
         {
             int zdims[] = {xdim-1, ydim-1, zdim-1};
             DBPutQuadvar(db, (char *)varName.c_str(), (char *)meshName.c_str(),
                          3, varnames, vars, zdims, sdims,  NULL, 0, DB_FLOAT,
                          DB_ZONECENT, optList);
         }
         EndVar(db, nlevels);

         if(optList != NULL)
             DBFreeOptlist(optList);
    }

protected:
    int xdim, ydim, zdim;
    bool nodal;
    float *xd, *yd, *zd;
    std::string name, units;
};

// ****************************************************************************
// Class: QuadMesh::ScalarData
//
// Purpose:
//   This class stores scalar data for QuadMesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:59 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Mar 10 13:20:17 PST 2009
//   Added ability to set the units to something other than Joules.
//
//    Mark C. Miller, Sat Jan 23 16:01:00 PST 2010
//    Fixed adding extra leading slash for level>0 to do so only if there is
//    not already a leading slash.
// ****************************************************************************

class QuadMesh::ScalarData
{
public:
    ScalarData(const std::string &n, int nx, int ny, int nz, bool node) : name(n), units()
    {
        xdim = nx;
        ydim = ny;
        zdim = nz;
        int ndata;
        if(node)
           ndata = xdim * ydim * zdim;
        else
           ndata = (xdim-1) * (ydim-1) * (zdim-1);
        data = new float[ndata];
        nodal = node;
    }

    ~ScalarData()
    {
        delete [] data;
    }

    const std::string &GetName() const { return name; };

    void SetUnits(const std::string &u) { units = u; }
    const std::string &GetUnits() const { return units; }

    int ZonalIndex(int x, int y, int z) const
    {
        return z*((ydim-1)*(xdim-1)) + y*(xdim-1) + x;
    }

    int NodalIndex(int x, int y, int z) const
    {
        return z*(ydim*xdim) + y*xdim + x;
    }

    void SetZonalValue(int x, int y, int z, float val)
    {
        data[ZonalIndex(x,y,z)] = val;
    }

    void SetNodalValue(int x, int y, int z, float val)
    {
        data[NodalIndex(x,y,z)] = val;
    }

    void WriteFile(DBfile *db, const std::string &meshName)
    {
         DBoptlist *optList = NULL;
         if(units.size() > 0)
         {
             optList = DBMakeOptlist(2);
             DBAddOption(optList, DBOPT_UNITS, (void*)units.c_str());
         }
         int nlevels = 0;
         std::string varName = BeginVar(db, name, nlevels);
         std::string mName(meshName);
         if(nlevels > 0 && mName[0] != '/')
            mName = std::string("/") + mName;
         int sdims = (zdim > 1) ? 3 : 2;
         if(nodal)
         {
             int ndims[] = {xdim, ydim, zdim};
             DBPutQuadvar1(db, (char *)varName.c_str(), (char *)mName.c_str(), data,
                           ndims, sdims, NULL, 0, DB_FLOAT, DB_NODECENT, optList);
         }
         else
         {
             int zdims[] = {xdim-1, ydim-1, zdim-1};
             DBPutQuadvar1(db, (char *)varName.c_str(), (char *)mName.c_str(), data,
                           zdims, sdims, NULL, 0, DB_FLOAT, DB_ZONECENT, optList);
         }
         EndVar(db, nlevels);

         if(optList != NULL)
             DBFreeOptlist(optList);
    }

    void WriteDataSlice(DBfile *db, const std::string &newMeshName,
        const std::string &newVarName, int sliceVal, int sliceDimension)
    {
         DBoptlist *optList = NULL;
         if(units.size() > 0)
         {
             optList = DBMakeOptlist(2);
             DBAddOption(optList, DBOPT_UNITS, (void*)units.c_str());
         }
         float *sliceData;

         if(nodal)
         {
             int ndata, index = 0;
             int ndims[2];
             if(sliceDimension == 0)
             {
                 ndata = zdim * ydim;
                 ndims[0] = zdim; ndims[1] = ydim;
                 sliceData = new float[ndata];
                 for(int y = 0; y < ydim; ++y)
                     for(int z = 0; z < zdim; ++z)
                     {
                         int originalIndex = NodalIndex(sliceVal, y, z);
                         sliceData[index++] = data[originalIndex];
                     }
             }
             else if(sliceDimension == 1)
             {
                 ndata = zdim * xdim;
                 sliceData = new float[ndata];
                 ndims[0] = xdim; ndims[1] = zdim;
                 for(int z = 0; z < zdim; ++z)
                     for(int x = 0; x < xdim; ++x)
                     {
                         int originalIndex = NodalIndex(x, sliceVal, z);
                         sliceData[index++] = data[originalIndex];
                     }
             }
             else
             {
                 ndata = xdim * ydim;
                 sliceData = new float[ndata];
                 ndims[0] = xdim; ndims[1] = ydim;
                 for(int y = 0; y < ydim; ++y)
                     for(int x = 0; x < xdim; ++x)
                     {
                         int originalIndex = NodalIndex(x, y, sliceVal);
                         sliceData[index++] = data[originalIndex];
                     }
             }

             DBPutQuadvar1(db, (char *)newVarName.c_str(), (char *)newMeshName.c_str(),
                           sliceData, ndims, 2, NULL, 0, DB_FLOAT, DB_NODECENT, optList);

             delete [] sliceData;
         }
         else
         {
             int ndata, index = 0;
             int zdims[2];
             if(sliceDimension == 0)
             {
                 ndata = (zdim-1) * (ydim-1);
                 zdims[0] = zdim-1; zdims[1] = ydim-1;
                 sliceData = new float[ndata];
                 for(int y = 0; y < ydim-1; ++y)
                     for(int z = 0; z < zdim-1; ++z)
                     {
                         int originalIndex = ZonalIndex(sliceVal, y, z);
                         sliceData[index++] = data[originalIndex];
                     }
             }
             else if(sliceDimension == 1)
             {
                 ndata = (zdim-1) * (xdim-1);
                 sliceData = new float[ndata];
                 zdims[0] = xdim-1; zdims[1] = zdim-1;
                 for(int z = 0; z < zdim-1; ++z)
                     for(int x = 0; x < xdim-1; ++x)
                     {
                         int originalIndex = ZonalIndex(x, sliceVal, z);
                         sliceData[index++] = data[originalIndex];
                     }
             }
             else
             {
                 ndata = (xdim-1) * (ydim-1);
                 sliceData = new float[ndata];
                 zdims[0] = xdim-1; zdims[1] = ydim-1;
                 for(int y = 0; y < ydim-1; ++y)
                     for(int x = 0; x < xdim-1; ++x)
                     {
                         int originalIndex = ZonalIndex(x, y, sliceVal);
                         sliceData[index++] = data[originalIndex];
                     }
             }

             DBPutQuadvar1(db, (char *)newVarName.c_str(), (char *)newMeshName.c_str(),
                           sliceData, zdims, 2, NULL, 0, DB_FLOAT, DB_ZONECENT, optList);

             delete [] sliceData;
         }
         if(optList != NULL)
             DBFreeOptlist(optList);
    }

    void ZonalGradientAt(int i, int j, int k, float grad[3]) const
    {
        if(i == 0)
            grad[0] = (data[ZonalIndex(i+1,j,k)]-data[ZonalIndex(i,j,k)]);
        else if(i == xdim-2)
            grad[0] = (data[ZonalIndex(i,j,k)]-data[ZonalIndex(i-1,j,k)]);
        else
            grad[0] = (data[ZonalIndex(i+1,j,k)]-data[ZonalIndex(i-1,j,k)]) * 0.5;

        if(j == 0)
            grad[1] = (data[ZonalIndex(i,j+1,k)]-data[ZonalIndex(i,j,k)]);
        else if(j == ydim-2)
            grad[1] = (data[ZonalIndex(i,j,k)]-data[ZonalIndex(i,j-1,k)]);
        else
            grad[1] = (data[ZonalIndex(i,j+1,k)]-data[ZonalIndex(i,j-1,k)]) * 0.5;

        if(k == 0)
            grad[2] = (data[ZonalIndex(i,j,k+1)]-data[ZonalIndex(i,j,k)]);
        else if(k == zdim-2)
            grad[2] = (data[ZonalIndex(i,j,k)]-data[ZonalIndex(i,j,k-1)]);
        else
            grad[2] = (data[ZonalIndex(i,j,k+1)]-data[ZonalIndex(i,j,k-1)]) * 0.5;
    }

    void NodalGradientAt(int i, int j, int k, float grad[3]) const
    {
        if(i == 0)
            grad[0] = (data[NodalIndex(i+1,j,k)]-data[NodalIndex(i,j,k)]);
        else if(i == xdim-1)
            grad[0] = (data[NodalIndex(i,j,k)]-data[NodalIndex(i-1,j,k)]);
        else
            grad[0] = (data[NodalIndex(i+1,j,k)]-data[NodalIndex(i-1,j,k)]) * 0.5;

        if(j == 0)
            grad[1] = (data[NodalIndex(i,j+1,k)]-data[NodalIndex(i,j,k)]);
        else if(j == ydim-1)
            grad[1] = (data[NodalIndex(i,j,k)]-data[NodalIndex(i,j-1,k)]);
        else
            grad[1] = (data[NodalIndex(i,j+1,k)]-data[NodalIndex(i,j-1,k)]) * 0.5;

        if(k == 0)
            grad[2] = (data[NodalIndex(i,j,k+1)]-data[NodalIndex(i,j,k)]);
        else if(k == zdim-1)
            grad[2] = (data[NodalIndex(i,j,k)]-data[NodalIndex(i,j,k-1)]);
        else
            grad[2] = (data[NodalIndex(i,j,k+1)]-data[NodalIndex(i,j,k-1)]) * 0.5;
    }

    void ZonalTensorGradientAt(int i, int j, int k, float grad[9]) const
    {
        // ii
        if(i == 0)
            grad[0] = (data[ZonalIndex(i+1,j,k)]-data[ZonalIndex(i,j,k)]);
        else if(i == xdim-2)
            grad[0] = (data[ZonalIndex(i,j,k)]-data[ZonalIndex(i-1,j,k)]);
        else
            grad[0] = (data[ZonalIndex(i+1,j,k)]-data[ZonalIndex(i-1,j,k)]) * 0.5;

        // ij
        if((i == 0) || (j == 0) || (i == xdim-2) || (j == ydim-2))
            grad[1] = 0.;
        else 
            grad[1] = (data[ZonalIndex(i-1,j-1,k)]-data[ZonalIndex(i+1,j+1,k)]) * 0.5;

        // ik
        if((i == 0) || (k == 0) || (i == xdim-2) || (k == zdim-2))
            grad[2] = 0.;
        else 
            grad[2] = (data[ZonalIndex(i-1,j,k-1)]-data[ZonalIndex(i+1,j,k+1)]) * 0.5;

        // ji
        grad[3] = grad[1];

        // jj
        if(j == 0)
            grad[4] = (data[ZonalIndex(i,j+1,k)]-data[ZonalIndex(i,j,k)]);
        else if(j == ydim-2)
            grad[4] = (data[ZonalIndex(i,j,k)]-data[ZonalIndex(i,j-1,k)]);
        else
            grad[4] = (data[ZonalIndex(i,j+1,k)]-data[ZonalIndex(i,j-1,k)]) * 0.5;

        // jk
        if((j == 0) || (k == 0) || (j == ydim-2) || (k == zdim-2))
            grad[5] = 0.;
        else 
            grad[5] = (data[ZonalIndex(i,j-1,k-1)]-data[ZonalIndex(i,j+1,k+1)]) * 0.5;

        // ki
        grad[6] = grad[2];

        // kj
        grad[7] = grad[5];

        // kk
        if(k == 0)
            grad[8] = (data[ZonalIndex(i,j,k+1)]-data[ZonalIndex(i,j,k)]);
        else if(k == zdim-2)
            grad[8] = (data[ZonalIndex(i,j,k)]-data[ZonalIndex(i,j,k-1)]);
        else
            grad[8] = (data[ZonalIndex(i,j,k+1)]-data[ZonalIndex(i,j,k-1)]) * 0.5;
    }

    void NodalTensorGradientAt(int i, int j, int k, float grad[9]) const
    {
        // ii
        if(i == 0)
            grad[0] = (data[NodalIndex(i+1,j,k)]-data[NodalIndex(i,j,k)]);
        else if(i == xdim-1)
            grad[0] = (data[NodalIndex(i,j,k)]-data[NodalIndex(i-1,j,k)]);
        else
            grad[0] = (data[NodalIndex(i+1,j,k)]-data[NodalIndex(i-1,j,k)]) * 0.5;

        // ij
        if((i == 0) || (j == 0) || (i == xdim-1) || (j == ydim-1))
            grad[1] = 0.;
        else 
            grad[1] = (data[NodalIndex(i-1,j-1,k)]-data[NodalIndex(i+1,j+1,k)]) * 0.5;

        // ik
        if((i == 0) || (k == 0) || (i == xdim-1) || (k == zdim-1))
            grad[2] = 0.;
        else 
            grad[2] = (data[NodalIndex(i-1,j,k-1)]-data[NodalIndex(i+1,j,k+1)]) * 0.5;

        // ji
        grad[3] = grad[1];

        // jj
        if(j == 0)
            grad[4] = (data[NodalIndex(i,j+1,k)]-data[NodalIndex(i,j,k)]);
        else if(j == ydim-1)
            grad[4] = (data[NodalIndex(i,j,k)]-data[NodalIndex(i,j-1,k)]);
        else
            grad[4] = (data[NodalIndex(i,j+1,k)]-data[NodalIndex(i,j-1,k)]) * 0.5;

        // jk
        if((j == 0) || (k == 0) || (j == ydim-1) || (k == zdim-1))
            grad[5] = 0.;
        else 
            grad[5] = (data[NodalIndex(i,j-1,k-1)]-data[NodalIndex(i,j+1,k+1)]) * 0.5;

        // ki
        grad[6] = grad[2];

        // kj
        grad[7] = grad[5];

        // kk
        if(k == 0)
            grad[8] = (data[NodalIndex(i,j,k+1)]-data[NodalIndex(i,j,k)]);
        else if(k == zdim-1)
            grad[8] = (data[NodalIndex(i,j,k)]-data[NodalIndex(i,j,k-1)]);
        else
            grad[8] = (data[NodalIndex(i,j,k+1)]-data[NodalIndex(i,j,k-1)]) * 0.5;
    }

    VectorData *CreateGradient(const std::string &name)
    {
        VectorData *vec = new VectorData(name, xdim, ydim, zdim, nodal);
        float grad[3];

        if(!nodal)
        {
            // Create the data.
            for(int i = 0; i < xdim-1; ++i)
                for(int j = 0; j < ydim-1; ++j)
                    for(int k = 0; k < zdim-1; ++k)
                    {
                        ZonalGradientAt(i, j, k, grad);

                        // If the zonal gradient is zero then make it a small
                        // gradient in the up direction.
                        if(grad[0] == 0. && grad[1] == 0. && grad[2] == 0.)
                        {
                            grad[1] = 0.01;
                        }

                        vec->SetZonalValue(i, j, k, grad);
                    }
        }
        else
        {
            // Create the data.
            for(int i = 0; i < xdim; ++i)
                for(int j = 0; j < ydim; ++j)
                    for(int k = 0; k < zdim; ++k)
                    {
                        NodalGradientAt(i, j, k, grad);
                        vec->SetNodalValue(i, j, k, grad);
                    }
        }

        return vec;
    }

    TensorData *CreateGradientTensor(const std::string &name);

protected:
    int xdim, ydim, zdim;
    bool nodal;
    float *data;
    std::string name, units;
};

// ****************************************************************************
// Class: QuadMesh::TensorData
//
// Purpose:
//   This class stores tensor data for QuadMesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:59 PST 2002
//
// Modifications:
//
// ****************************************************************************

class QuadMesh::TensorData
{
public:
    TensorData(const std::string &n, QuadMesh::ScalarData *comps[9]) : name(n)
    {
        for (int i = 0 ; i < 9 ; i++)
            components[i] = comps[i];
    }
    ~TensorData()
    {
        for (int i = 0 ; i < 9 ; i++)
            delete components[i];
    }

    void WriteFile(DBfile *db, const std::string &meshName)
    {
         DBMkDir(db, "tensor_comps");
         char absolute_meshname[1024];
         sprintf(absolute_meshname, "/%s", meshName.c_str());
         for (int i = 0 ; i < 9 ; i++)
             components[i]->WriteFile(db, absolute_meshname);
         char defvars[1024];
         sprintf(defvars, "%s tensor { { <%s>, <%s>, <%s> }, { <%s>, <%s>, <%s> }, { <%s>, <%s>, <%s> } }; %s_diagonal array array_compose(<%s>, <%s>, <%s>)",
                 name.c_str(), components[0]->GetName().c_str(),
                 components[1]->GetName().c_str(), components[2]->GetName().c_str(),
                 components[3]->GetName().c_str(), components[4]->GetName().c_str(), 
                 components[5]->GetName().c_str(), components[6]->GetName().c_str(),
                 components[7]->GetName().c_str(), components[8]->GetName().c_str(), 
                 name.c_str(), components[0]->GetName().c_str(),
                 components[4]->GetName().c_str(), components[8]->GetName().c_str());
         int len = strlen(defvars)+1;
         DBWrite(db, "_visit_defvars", defvars, &len, 1, DB_CHAR);
    }

protected:
    QuadMesh::ScalarData *components[9];
    std::string name;
};

// ****************************************************************************
// Method: QuadMesh::ScalarData::CreateGradientTensor
//
// Purpose: 
//   This method creates tensor data from a scalar.
//
// Arguments:
//   name : The name of the new tensor data.
//
// Returns:    Tensor data.
//
// Note:       
//
// Programmer: Hank Childs
// Creation:   Mon Dec  1 09:24:23 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QuadMesh::TensorData *
QuadMesh::ScalarData::CreateGradientTensor(const std::string &name)
{
    QuadMesh::ScalarData *sub_comps[9];

    for (int i = 0 ; i < 9 ; i++)
    {
        char comp1;
        char comp2;
        switch (i%3)
        {
           case 0: comp1 = 'i'; break;
           case 1: comp1 = 'j'; break;
           default: comp1 = 'k'; break;
        }
        switch (i/3)
        {
           case 0: comp2 = 'i'; break;
           case 1: comp2 = 'j'; break;
           default: comp2 = 'k'; break;
        }
        char comp_name[1024];
        sprintf(comp_name, "tensor_comps/%s_%c%c", name.c_str(), comp1, comp2);
        sub_comps[i] = new ScalarData(comp_name, xdim, ydim, zdim, nodal);
        sub_comps[i]->SetUnits(units);
    }
    if(!nodal)
    {
        // Create the data.
        for(int i = 0; i < xdim-1; ++i)
            for(int j = 0; j < ydim-1; ++j)
                for(int k = 0; k < zdim-1; ++k)
                {
                    float vals[9];
                    ZonalTensorGradientAt(i,j,k, vals);
                    for(int l = 0 ; l < 9 ; l++)
                        sub_comps[l]->SetZonalValue(i, j, k, vals[l]);
                }
    }
    else
    {
        // Create the data.
        for(int i = 0; i < xdim; ++i)
            for(int j = 0; j < ydim; ++j)
                for(int k = 0; k < zdim; ++k)
                {
                    float vals[9];
                    NodalTensorGradientAt(i,j,k, vals);
                    for(int l = 0 ; l < 9 ; l++)
                        sub_comps[l]->SetNodalValue(i, j, k, vals[l]);
                }
    }

    return new QuadMesh::TensorData(name, sub_comps);
}

// ****************************************************************************
// Class: QuadMesh::SliceInfo
//
// Purpose:
//   This class stores slice information for QuadMesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:59 PST 2002
//
// Modifications:
//
// ****************************************************************************

class QuadMesh::SliceInfo
{
public:
    SliceInfo(const std::string &nm, const std::string &nvn,
        int sv, int sd, ScalarData *d) : newMeshName(nm), newVarName(nvn)
    {
        sliceVal = sv;
        sliceDimension = sd;
        scalars = d;
    }

    ~SliceInfo()
    {
    }

    void WriteFile(DBfile *db, QuadMesh *qm)
    {
        // Write a slice of the mesh
        qm->WriteMeshSlice(db, newMeshName, sliceVal, sliceDimension);
         // Write the sliced data on that mesh.
        scalars->WriteDataSlice(db, newMeshName, newVarName, sliceVal, sliceDimension);
    }

protected:
    std::string newMeshName;
    std::string newVarName;
    int sliceVal;
    int sliceDimension;
    ScalarData *scalars;
};

///////////////////////////////////////////////////////////////////////////////

QuadMesh::QuadMesh(int nx, int ny, int nz, bool rect) : scalarData(),
    vectorData(), tensorData(), sliceInfo(), meshName("Mesh"), 
    meshLabelX(), meshLabelY(), meshLabelZ(),
    meshUnitsX(), meshUnitsY(), meshUnitsZ()
{
    xdim = nx;
    ydim = ny;
    zdim = nz;
    if(rect)
    {
        coordX = new float[nx];
        coordY = new float[ny];
        coordZ = new float[nz];
    }
    else
    {
        int nels = xdim * ydim * zdim;
        coordX = new float[nels];
        coordY = new float[nels];
        coordZ = new float[nels];
    }

    // Allocate storage for the material cells.
    mats = new MaterialList;
    if(zdim > 1)
        mats->AllocClean((xdim-1) * (ydim-1) * (zdim-1));
    else
        mats->AllocClean((xdim-1) * (ydim-1));
    writeMaterial = false;
}

QuadMesh::~QuadMesh()
{
    delete [] coordX;
    delete [] coordY;
    delete [] coordZ;

    size_t i;
    for(i = 0; i < scalarData.size(); ++i)
        delete scalarData[i];
    for(i = 0; i < vectorData.size(); ++i)
        delete vectorData[i];
    for(i = 0; i < tensorData.size(); ++i)
        delete tensorData[i];
    for(i = 0; i < sliceInfo.size(); ++i)
        delete sliceInfo[i];

    delete mats;
}

void QuadMesh::SetMeshName(const std::string &name)
{
    meshName = name;
}

int QuadMesh::XDim() const { return xdim; };
int QuadMesh::YDim() const { return ydim; };
int QuadMesh::ZDim() const { return zdim; };

void QuadMesh::SetMeshLabels(const std::string &labelX, 
   const std::string &labelY, const std::string &labelZ)
{
    meshLabelX = labelX;
    meshLabelY = labelY;
    meshLabelZ = labelZ;
}

void QuadMesh::SetMeshUnits(const std::string &unitsX, 
    const std::string &unitsY, const std::string &unitsZ)
{
    meshUnitsX = unitsX;
    meshUnitsY = unitsY;
    meshUnitsZ = unitsZ;
}

void
QuadMesh::CreateZonalData(const std::string &name, 
    float (*zonal)(int,int,int, QuadMesh *),
    const std::string &units)
{
    ScalarData *m = new ScalarData(name, xdim, ydim, zdim, false);
    m->SetUnits(units);
    scalarData.push_back(m);

    // Create the data.
    int zzones = (zdim>1) ? (zdim-1) : 1;
    for(int k = 0; k < zzones; ++k)
        for(int j = 0; j < ydim-1; ++j)
            for(int i = 0; i < xdim-1; ++i)
                m->SetZonalValue(i,j,k, (*zonal)(i,j,k, this));
}

void
QuadMesh::CreateNodalData(const std::string &name,
    float (*nodal)(float *, QuadMesh *),
    const std::string &units)
{
    ScalarData *m = new ScalarData(name, xdim, ydim, zdim, true);
    m->SetUnits(units);
    scalarData.push_back(m);

    // Create the data.
    for(int k = 0; k < zdim; ++k)
        for(int j = 0; j < ydim; ++j)
            for(int i = 0; i < xdim; ++i)
            {
                float pt[3];
                pt[0] = GetX(i,j,k);
                pt[1] = GetY(i,j,k);
                pt[2] = GetZ(i,j,k);
                m->SetNodalValue(i, j, k, (*nodal)(pt, this));
            }
}

void
QuadMesh::CreateNodalVectorData(const std::string &name,
    void (*nodal)(float *, int, int, int, QuadMesh *),
    const std::string &units)
{
    VectorData *m = new VectorData(name, xdim, ydim, zdim, true);
    vectorData.push_back(m);

    // Create the data.
    for(int k = 0; k < zdim; ++k)
        for(int j = 0; j < ydim; ++j)
            for(int i = 0; i < xdim; ++i)
            {
                float vec[3];
                // Get the vector value.
                (*nodal)(vec, i, j, k, this);
                m->SetNodalValue(i, j, k, vec);
            }
}

void QuadMesh::CreateGradient(const std::string &name, 
    const std::string &gradName, const std::string &units)
{
    for(size_t i = 0; i < scalarData.size(); ++i)
    {
        if(scalarData[i]->GetName() == name)
        {
            VectorData *gradient = scalarData[i]->CreateGradient(gradName);
            if(units.size() == 0)
                gradient->SetUnits(scalarData[i]->GetUnits());
            else
                gradient->SetUnits(units);
            vectorData.push_back(gradient);
            break;
        }
    }
}

void QuadMesh::CreateGradientTensor(const std::string &varName, 
    const std::string &outputName)
{
    for(size_t i = 0; i < scalarData.size(); ++i)
    {
        if(scalarData[i]->GetName() == varName)
        {
            TensorData *tensor = scalarData[i]->CreateGradientTensor(outputName);
            tensorData.push_back(tensor);
            break;
        }
    }
}

void QuadMesh::AddMaterial(const std::string &matname)
{
    mats->AddMaterial(matname);
}

void QuadMesh::CreateMaterialData(void (*createmat)(int,int,int, int *, double*, int *, QuadMesh *))
{
    int nMats = 1;
    int matnos[100];
    double matVf[100];

    // Create the data.
    int zzones = (zdim>1) ? (zdim-1) : 1;
    for(int k = 0; k < zzones; ++k)
        for(int j = 0; j < ydim-1; ++j)
            for(int i = 0; i < xdim-1; ++i)
            {
                int zoneid = k*((ydim-1)*(xdim-1)) + j*(xdim-1) + i;
                (*createmat)(i,j,k, matnos, matVf, &nMats, this);
                if(nMats > 1)
                    mats->AddMixed(zoneid, matnos, matVf, nMats);
                else
                    mats->AddClean(zoneid, matnos[0]);
            }

     writeMaterial = true;
}

void QuadMesh::AddSlice(const std::string &varName, const std::string &nmn, const std::string &nvn,
    int sliceVal, int sliceDimension)
{
    if(zdim <= 1)
    {
        cerr << "The mesh is 2D so it can't be sliced." << endl;
        return;
    }

    // Look for the variable name in the scalars array.
    for(size_t i = 0; i < scalarData.size(); ++i)
    {
        if(scalarData[i]->GetName() == varName)
        {
            SliceInfo *slice = new SliceInfo(nmn, nvn, sliceVal, sliceDimension, scalarData[i]);
            sliceInfo.push_back(slice);
            return;
        }
    }

    cerr << "The variable " << varName.c_str() << " is not a scalar variable." << endl;
}

void QuadMesh::WriteFile(DBfile *db)
{
     // Write the mesh.
     WriteMesh(db);

     // Write the scalar mesh data
     size_t i;
     for(i = 0; i < scalarData.size(); ++i)
         scalarData[i]->WriteFile(db, meshName);

     // Write the vector mesh data
     for(i = 0; i < vectorData.size(); ++i)
         vectorData[i]->WriteFile(db, meshName);

     // Write the tensor mesh data
     for(i = 0; i < tensorData.size(); ++i)
         tensorData[i]->WriteFile(db, meshName);

     // Write the material data
     if(writeMaterial)
         mats->WriteMaterial(db, "mat1", meshName, xdim-1, ydim-1, zdim-1);

     // Write the slices
     for(i = 0; i < sliceInfo.size(); ++i)
         sliceInfo[i]->WriteFile(db, this);
}

DBoptlist *
QuadMesh::MakeOptionList() const
{
    DBoptlist *optList = DBMakeOptlist(6);
    int opt = 0;
    if(meshLabelX.size() > 0)
    {
        ++opt;
        DBAddOption(optList, DBOPT_XLABEL, (void*)meshLabelX.c_str());
    }
    if(meshLabelY.size() > 0)
    {
        ++opt;
        DBAddOption(optList, DBOPT_YLABEL, (void*)meshLabelY.c_str());
    }
    if(meshLabelZ.size() > 0)
    {
        ++opt;
        DBAddOption(optList, DBOPT_ZLABEL, (void*)meshLabelZ.c_str());
    }

    if(meshUnitsX.size() > 0)
    {
        ++opt;
        DBAddOption(optList, DBOPT_XUNITS, (void*)meshUnitsX.c_str());
    }
    if(meshUnitsY.size() > 0)
    {
        ++opt;
        DBAddOption(optList, DBOPT_YUNITS, (void*)meshUnitsY.c_str());
    }
    if(meshUnitsZ.size() > 0)
    {
        ++opt;
        DBAddOption(optList, DBOPT_ZUNITS, (void*)meshUnitsZ.c_str());
    }

    if(opt == 0)
    {
        DBFreeOptlist(optList);
        optList = NULL;
    }

    return optList;
}

////////////////////////////////////////////////////////////////////////////////


RectilinearMesh::RectilinearMesh(int nx, int ny, int nz) : QuadMesh(nx, ny, nz, true)
{
}

RectilinearMesh::~RectilinearMesh()
{
}

void RectilinearMesh::WriteMesh(DBfile *db)
{
     float *coords[3] = {coordX, coordY, coordZ};
     int dims[3] = {xdim, ydim, zdim};
     int ndims = (zdim > 1) ? 3 : 2;
     DBoptlist *optList = MakeOptionList();
     DBPutQuadmesh(db, (char *)meshName.c_str(), NULL, coords, dims, ndims,
                   DB_FLOAT, DB_COLLINEAR, optList);
     if(optList != NULL)
         DBFreeOptlist(optList);
}

void RectilinearMesh::WriteMeshSlice(DBfile *db, const std::string &newMeshName,
     int sliceVal, int sliceDimension)
{
    float *slice_coordX, *slice_coordY;
    int i, dims[2];

    if(sliceDimension == 0)
    {
        dims[0] = zdim; dims[1] = ydim;
        slice_coordX = new float[zdim];
        for(i = 0; i < zdim; ++i)
            slice_coordX[i] = coordZ[i];
        slice_coordY = new float[ydim];
        for(i = 0; i < ydim; ++i)
            slice_coordY[i] = coordY[i];
    }
    else if(sliceDimension == 1)
    {
        dims[0] = xdim; dims[1] = zdim;
        slice_coordX = new float[xdim];
        for(i = 0; i < xdim; ++i)
            slice_coordX[i] = coordX[i];
        slice_coordY = new float[zdim];
        for(i = 0; i < zdim; ++i)
            slice_coordY[i] = coordZ[i];
    }
    else
    {
        dims[0] = xdim; dims[1] = ydim;
        slice_coordX = new float[xdim];
        for(i = 0; i < xdim; ++i)
            slice_coordX[i] = coordX[i];
        slice_coordY = new float[ydim];
        for(i = 0; i < ydim; ++i)
            slice_coordY[i] = coordY[i];
    }

    float *coords[] = {slice_coordX, slice_coordY};

    DBoptlist *optList = MakeOptionList();
    DBPutQuadmesh(db, (char *)newMeshName.c_str(), NULL, coords, dims, 2,
                  DB_FLOAT, DB_COLLINEAR, optList);
    if(optList != NULL)
        DBFreeOptlist(optList);

    delete [] slice_coordX;
    delete [] slice_coordY;
}

void RectilinearMesh::SetRange(float *coord, float minval, float maxval, int steps)
{
     for(int i = 0; i < steps; ++i)
     {
         float t = float(i) / float(steps - 1);
         coord[i] = t*maxval + (1. - t)*minval;
     }
}

////////////////////////////////////////////////////////////////////////////////

CurvilinearMesh::CurvilinearMesh(int X, int Y, int Z) : QuadMesh(X, Y, Z, false)
{
}

CurvilinearMesh::~CurvilinearMesh()
{
}

void CurvilinearMesh::WriteMesh(DBfile *db)
{
     float *coords[3] = {coordX, coordY, coordZ};
     int dims[3] = {xdim, ydim, zdim};
     int ndims = (zdim > 1) ? 3 : 2;
     DBoptlist *optList = MakeOptionList();
     DBPutQuadmesh(db, (char *)meshName.c_str(), NULL, coords, dims,
                   ndims, DB_FLOAT, DB_NONCOLLINEAR, NULL);
     if(optList != NULL)
         DBFreeOptlist(optList);
}

void CurvilinearMesh::WriteMeshSlice(DBfile *db, const std::string &newMeshName,
     int sliceVal, int sliceDimension)
{
    float *slice_coordX, *slice_coordY;
    int index = 0;
    int dims[2];

    if(sliceDimension == 0)
    {
        dims[0] = zdim; dims[1] = ydim;
        slice_coordX = new float[zdim * ydim];
        slice_coordY = new float[zdim * ydim];
        for(int y = 0; y < ydim; ++y)
            for(int z = 0; z < zdim; ++z, ++index)
            {
                slice_coordX[index] = GetZ(sliceVal, y, z);
                slice_coordY[index] = GetY(sliceVal, y, z);
            }
    }
    else if(sliceDimension == 1)
    {
        dims[0] = xdim; dims[1] = zdim;
        slice_coordX = new float[xdim * zdim];
        slice_coordY = new float[xdim * zdim];
        for(int z = 0; z < zdim; ++z)
            for(int x = 0; x < xdim; ++x, ++index)
            {
                slice_coordX[index] = GetX(x, sliceVal, z);
                slice_coordY[index] = GetZ(x, sliceVal, z);
            }
    }
    else
    {
        dims[0] = xdim; dims[1] = ydim;
        slice_coordX = new float[xdim * ydim];
        slice_coordY = new float[xdim * ydim];
        for(int y = 0; y < ydim; ++y)
            for(int x = 0; x < xdim; ++x, ++index)
            {
                slice_coordX[index] = GetX(x, y, sliceVal);
                slice_coordY[index] = GetY(x, y, sliceVal);
            }
    }

    float *coords[] = {slice_coordX, slice_coordY};

    DBoptlist *optList = MakeOptionList();
    DBPutQuadmesh(db, (char *)newMeshName.c_str(), NULL, coords, dims, 2,
                  DB_FLOAT, DB_COLLINEAR, optList);

    if(optList != NULL)
        DBFreeOptlist(optList);
    delete [] slice_coordX;
    delete [] slice_coordY;
}
