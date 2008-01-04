/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
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

#include <stdio.h>
#include <silo.h>
#include <math.h>
#include <visitstream.h>
#include <string>
#include <vector>

// ****************************************************************************
// File: noise.C
//
// Purpose:
//   This program creates a set of scattered points and uses two methods for
//   interpolating scattered data to sample the field at the nodes of a
//   rectilinear grid. This database is intended for visualizing wild
//   isosurfaces.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 14:02:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

//
// The size of the mesh i,j,k range
//

#define ISTEPS 50
#define JSTEPS 50
#define KSTEPS 50

//
// The size of the mesh
//

#define XMIN -10.
#define XMAX 10.
#define YMIN -10.
#define YMAX 10.
#define ZMIN -10.
#define ZMAX 10.

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
// Class: MaterialList
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

class MaterialList
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

    void WriteMaterial(DBfile *db, const char *matvarname, const char *meshName, int nx, int ny, int nz)
    {
        int i, mdims[3] = {nx,ny,nz};

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
            DBPutMaterial(db, (char *)matvarname, (char *)meshName,
                          matNames.size(), allmats,
                          matlist, mdims, 3, mix_next,
                          mix_mat, mix_zone,
                          (float*)mix_vf , GetMixedSize(),
                          DB_DOUBLE, optList);
        }
        else
        {         
            DBPutMaterial(db, (char *)matvarname, (char *)meshName,
                          matNames.size(), allmats,
                          matlist, mdims, 3, NULL,
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
// Class: QuadMesh3D
//
// Purpose:
//   Abstract base class for 3D quad meshes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:59 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Oct 4 14:25:40 PST 2002
//   I added methods to create the gradient of a scalar field.
//
//   Eric Brugger, Tue Mar 25 15:56:59 PST 2003
//   I corrected an out of range index error in ZonalGradientAt.
//
//   Hank Childs, Mon Dec  1 09:24:23 PST 2003
//   Added support for tensors.
//
//   Hank Childs, Thu Jul 21 16:49:02 PDT 2005
//   Added support for array variables.
//
// ****************************************************************************

class QuadMesh3D
{
protected:
    class TensorData;
    class VectorData
    {
    public:
        VectorData(const std::string &n, int nx, int ny, int nz, bool node) : name(n)
        {
            xdim = nx;
            ydim = ny;
            zdim = nz;
            int ndata;
            if(node)
               ndata = xdim * ydim * zdim;
            else
               ndata = (xdim-1) * (ydim-1) * (zdim-1);
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

        void WriteFile(DBfile *db, const char *meshName)
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

             DBoptlist *optList = DBMakeOptlist(2);
             DBAddOption(optList, DBOPT_UNITS, (void*)"cm/s");
             if(nodal)
             {
                 int ndims[] = {xdim, ydim, zdim};
                 DBPutQuadvar(db, (char *)name.c_str(), (char *)meshName,
                              3, varnames, vars, ndims, 3,  NULL, 0, DB_FLOAT,
                              DB_NODECENT, optList);
             }
             else
             {
                 int zdims[] = {xdim-1, ydim-1, zdim-1};
                 DBPutQuadvar(db, (char *)name.c_str(), (char *)meshName,
                              3, varnames, vars, zdims, 3,  NULL, 0, DB_FLOAT,
                              DB_ZONECENT, optList);
             }
             DBFreeOptlist(optList);
        }

    protected:
        int xdim, ydim, zdim;
        bool nodal;
        float *xd, *yd, *zd;
        std::string name;
    };

    class ScalarData
    {
    public:
        ScalarData(const std::string &n, int nx, int ny, int nz, bool node) : name(n)
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

        void WriteFile(DBfile *db, const char *meshName)
        {
             DBoptlist *optList = DBMakeOptlist(2);
             DBAddOption(optList, DBOPT_UNITS, (void*)"Joules");
             if(nodal)
             {
                 int ndims[] = {xdim, ydim, zdim};
                 DBPutQuadvar1(db, (char *)name.c_str(), (char *)meshName, data,
                               ndims, 3, NULL, 0, DB_FLOAT, DB_NODECENT, optList);
             }
             else
             {
                 int zdims[] = {xdim-1, ydim-1, zdim-1};
                 DBPutQuadvar1(db, (char *)name.c_str(), (char *)meshName, data,
                               zdims, 3, NULL, 0, DB_FLOAT, DB_ZONECENT, optList);
             }
             DBFreeOptlist(optList);
        }

        void WriteDataSlice(DBfile *db, const std::string &newMeshName,
            const std::string &newVarName, int sliceVal, int sliceDimension)
        {
             DBoptlist *optList = DBMakeOptlist(1);
             DBAddOption(optList, DBOPT_UNITS, (void*)"Joules");
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

        VectorData *CreateGradient(const char *name)
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

        TensorData *CreateGradientTensor(const char *name)
        {
            ScalarData *sub_comps[9];

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
                sprintf(comp_name, "tensor_comps/%s_%c%c", name, comp1, comp2);
                sub_comps[i] = new ScalarData(comp_name, xdim, ydim, zdim, nodal);
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

            return new TensorData(name, sub_comps);
        }

    protected:
        int xdim, ydim, zdim;
        bool nodal;
        float *data;
        std::string name;
    };

    class TensorData {
    public:
        TensorData(const std::string &n, ScalarData *comps[9]) : name(n)
        {
            for (int i = 0 ; i < 9 ; i++)
                components[i] = comps[i];
        }
        ~TensorData()
        {
            for (int i = 0 ; i < 9 ; i++)
                delete components[i];
        }

        void WriteFile(DBfile *db, const char *meshName)
        {
             DBMkDir(db, "tensor_comps");
             char absolute_meshname[1024];
             sprintf(absolute_meshname, "/%s", meshName);
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
        ScalarData *components[9];
        std::string name;
    };

    class SliceInfo
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

        void WriteFile(DBfile *db, QuadMesh3D *qm)
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

    typedef std::vector<ScalarData *> ScalarDataVector;
    typedef std::vector<VectorData *> VectorDataVector;
    typedef std::vector<TensorData *> TensorDataVector;
    typedef std::vector<SliceInfo *>  SliceInfoVector;

    friend class SliceInfo;
public:
    QuadMesh3D(int nx, int ny, int nz, bool rect = true) : scalarData(),
        vectorData(), tensorData(), sliceInfo(), meshName("Mesh")
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
        mats.AllocClean((xdim-1) * (ydim-1) * (zdim-1));
        writeMaterial = false;
    }

    virtual ~QuadMesh3D()
    {
        delete [] coordX;
        delete [] coordY;
        delete [] coordZ;

        int i;
        for(i = 0; i < scalarData.size(); ++i)
            delete scalarData[i];
        for(i = 0; i < vectorData.size(); ++i)
            delete vectorData[i];
        for(i = 0; i < tensorData.size(); ++i)
            delete tensorData[i];
        for(i = 0; i < sliceInfo.size(); ++i)
            delete sliceInfo[i];
    }

    void SetMeshName(const std::string &name)
    {
        meshName = name;
    }

    int XDim() const { return xdim; };
    int YDim() const { return ydim; };
    int ZDim() const { return zdim; };

    virtual float GetX(int, int, int) const = 0;
    virtual float GetY(int, int, int) const = 0;
    virtual float GetZ(int, int, int) const = 0;

    void CreateZonalData(const char *name, float (*zonal)(int,int,int, QuadMesh3D *))
    {
        ScalarData *m = new ScalarData(name, xdim, ydim, zdim, false);
        scalarData.push_back(m);

        // Create the data.
        for(int i = 0; i < xdim-1; ++i)
            for(int j = 0; j < ydim-1; ++j)
                for(int k = 0; k < zdim-1; ++k)
                    m->SetZonalValue(i,j,k, (*zonal)(i,j,k, this));
    }

    void CreateNodalData(const char *name, float (*nodal)(float *, QuadMesh3D *))
    {
        ScalarData *m = new ScalarData(name, xdim, ydim, zdim, true);
        scalarData.push_back(m);

        // Create the data.
        for(int i = 0; i < xdim; ++i)
            for(int j = 0; j < ydim; ++j)
                for(int k = 0; k < zdim; ++k)
                {
                    float pt[3];
                    pt[0] = GetX(i,j,k);
                    pt[1] = GetY(i,j,k);
                    pt[2] = GetZ(i,j,k);
                    m->SetNodalValue(i, j, k, (*nodal)(pt, this));
                }
    }

    void CreateNodalVectorData(const char *name, void (*nodal)(float *, int, int, int, QuadMesh3D *))
    {
        VectorData *m = new VectorData(name, xdim, ydim, zdim, true);
        vectorData.push_back(m);

        // Create the data.
        for(int i = 0; i < xdim; ++i)
            for(int j = 0; j < ydim; ++j)
                for(int k = 0; k < zdim; ++k)
                {
                    float vec[3];
                    // Get the vector value.
                    (*nodal)(vec, i, j, k, this);
                    m->SetNodalValue(i, j, k, vec);
                }
    }

    void CreateGradient(const char *name, const char *gradName)
    {
        for(int i = 0; i < scalarData.size(); ++i)
        {
            if(scalarData[i]->GetName() == name)
            {
                VectorData *gradient = scalarData[i]->CreateGradient(gradName);
                vectorData.push_back(gradient);
                break;
            }
        }
    }

    void CreateGradientTensor(const char *varName, const char *outputName)
    {
        for(int i = 0; i < scalarData.size(); ++i)
        {
            if(scalarData[i]->GetName() == varName)
            {
                TensorData *tensor = scalarData[i]->CreateGradientTensor(outputName);
                tensorData.push_back(tensor);
                break;
            }
        }
    }

    void AddMaterial(const char *matname)
    {
        mats.AddMaterial(matname);
    }

    void CreateMaterialData(void (*createmat)(int,int,int, int *, double*, int *, QuadMesh3D *))
    {
        int nMats = 1;
        int matnos[100];
        double matVf[100];

        // Create the data.
        for(int i = 0; i < xdim-1; ++i)
            for(int j = 0; j < ydim-1; ++j)
                for(int k = 0; k < zdim-1; ++k)
                {
                    int zoneid = k*((ydim-1)*(xdim-1)) + j*(xdim-1) + i;
                    (*createmat)(i,j,k, matnos, matVf, &nMats, this);
                    if(nMats > 1)
                        mats.AddMixed(zoneid, matnos, matVf, nMats);
                    else
                        mats.AddClean(zoneid, matnos[0]);
                }

         writeMaterial = true;
    }

    void AddSlice(const std::string &varName, const std::string &nmn, const std::string &nvn,
        int sliceVal, int sliceDimension)
    {
        // Look for the variable name in the scalars array.
        for(int i = 0; i < scalarData.size(); ++i)
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

    void WriteFile(DBfile *db)
    {
         // Write the mesh.
         WriteMesh(db);

         // Write the scalar mesh data
         int i;
         for(i = 0; i < scalarData.size(); ++i)
             scalarData[i]->WriteFile(db, meshName.c_str());

         // Write the vector mesh data
         for(i = 0; i < vectorData.size(); ++i)
             vectorData[i]->WriteFile(db, meshName.c_str());

         // Write the tensor mesh data
         for(i = 0; i < tensorData.size(); ++i)
             tensorData[i]->WriteFile(db, meshName.c_str());

         // Write the material data
         if(writeMaterial)
             mats.WriteMaterial(db, "mat1", meshName.c_str(), xdim-1, ydim-1, zdim-1);

         // Write the slices
         for(i = 0; i < sliceInfo.size(); ++i)
             sliceInfo[i]->WriteFile(db, this);
    }

protected:
    virtual void WriteMesh(DBfile *db) = 0;
    virtual void WriteMeshSlice(DBfile *db, const std::string &, int, int) = 0;

    int xdim;
    int ydim;
    int zdim;
    float *coordX;
    float *coordY;
    float *coordZ;
    ScalarDataVector scalarData;
    VectorDataVector vectorData;
    TensorDataVector tensorData;
    SliceInfoVector  sliceInfo;
    MaterialList     mats;
    bool             writeMaterial;
    std::string      meshName;
};

// ****************************************************************************
// Class: RectilinearMesh3D
//
// Purpose:
//   Represents a 3D rectilinear mesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:54:28 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Mar 14 09:33:46 PDT 2003
//   I added the WriteMeshSlice method.
//
// ****************************************************************************

class RectilinearMesh3D : public QuadMesh3D
{
public:
    RectilinearMesh3D(int nx, int ny, int nz) : QuadMesh3D(nx, ny, nz, true)
    {
    }

    virtual ~RectilinearMesh3D()
    {
    }

    virtual float GetX(int x, int, int) const
    {
        return coordX[x];
    }

    virtual float GetY(int, int y, int) const
    {
        return coordY[y];
    }

    virtual float GetZ(int, int, int z) const
    {
        return coordZ[z];
    }

    void SetX(int x, float val)
    {
        coordX[x] = val;
    }

    void SetY(int y,float val)
    {
        coordY[y] = val;
    }

    void SetZ(int z, float val)
    {
        coordZ[z] = val;
    }

    void SetXValues(float minX, float maxX)
    {
        SetRange(coordX, minX, maxX, xdim);
    }

    void SetYValues(float minY, float maxY)
    {
        SetRange(coordY, minY, maxY, ydim);
    }

    void SetZValues(float minZ, float maxZ)
    {
        SetRange(coordZ, minZ, maxZ, zdim);
    }

protected:
    virtual void WriteMesh(DBfile *db)
    {
         float *coords[3] = {coordX, coordY, coordZ};
         int dims[3] = {xdim, ydim, zdim};

         DBoptlist *optList = DBMakeOptlist(6);
         DBAddOption(optList, DBOPT_XLABEL, (void*)"Width");
         DBAddOption(optList, DBOPT_YLABEL, (void*)"Height");
         DBAddOption(optList, DBOPT_ZLABEL, (void*)"Depth");
         DBAddOption(optList, DBOPT_XUNITS, (void*)"parsec");
         DBAddOption(optList, DBOPT_YUNITS, (void*)"parsec");
         DBAddOption(optList, DBOPT_ZUNITS, (void*)"parsec");
         DBPutQuadmesh(db, (char *)meshName.c_str(), NULL, coords, dims, 3,
                       DB_FLOAT, DB_COLLINEAR, optList);
         DBFreeOptlist(optList);
    }

    virtual void WriteMeshSlice(DBfile *db, const std::string &newMeshName,
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

         DBoptlist *optList = DBMakeOptlist(6);
         DBAddOption(optList, DBOPT_XLABEL, (void*)"Width");
         DBAddOption(optList, DBOPT_YLABEL, (void*)"Height");
         DBAddOption(optList, DBOPT_ZLABEL, (void*)"Depth");
         DBAddOption(optList, DBOPT_XUNITS, (void*)"parsec");
         DBAddOption(optList, DBOPT_YUNITS, (void*)"parsec");
         DBPutQuadmesh(db, (char *)newMeshName.c_str(), NULL, coords, dims, 2,
                       DB_FLOAT, DB_COLLINEAR, optList);
         DBFreeOptlist(optList);

         delete [] slice_coordX;
         delete [] slice_coordY;
    }

    void SetRange(float *coord, float minval, float maxval, int steps)
    {
         for(int i = 0; i < steps; ++i)
         {
             float t = float(i) / float(steps - 1);
             coord[i] = t*maxval + (1. - t)*minval;
         }
    }
};

// ****************************************************************************
// Class: CurveMesh3D
//
// Purpose:
//   Represents a 3D curvilinear mesh
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:55:27 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Mar 14 09:33:32 PDT 2003
//   I added the WriteMeshSlice method.
//
// ****************************************************************************

class CurveMesh3D : public QuadMesh3D
{
public:
    CurveMesh3D(int X, int Y, int Z) : QuadMesh3D(X, Y, Z, false)
    {
    }

    virtual ~CurveMesh3D()
    {
    }

    virtual float GetX(int x, int y, int z) const
    {
        return coordX[z*(ydim*xdim) + y*xdim + x];
    }

    virtual float GetY(int x, int y, int z) const
    {
        return coordY[z*(ydim*xdim) + y*xdim + x];
    }

    virtual float GetZ(int x, int y, int z) const
    {
        return coordZ[z*(ydim*xdim) + y*xdim + x];
    }

    void SetX(int x, int y, int z, float val)
    {
        coordX[z*(ydim*xdim) + y*xdim + x] = val;
    }

    void SetY(int x, int y, int z, float val)
    {
        coordY[z*(ydim*xdim) + y*xdim + x] = val;
    }

    void SetZ(int x, int y, int z, float val)
    {
        coordZ[z*(ydim*xdim) + y*xdim + x] = val;
    }

private:
    virtual void WriteMesh(DBfile *db, const char *name)
    {
         float *coords[3] = {coordX, coordY, coordZ};
         int dims[3] = {xdim, ydim, zdim};
         DBoptlist *optList = DBMakeOptlist(6);
         DBAddOption(optList, DBOPT_XLABEL, (void*)"Width");
         DBAddOption(optList, DBOPT_YLABEL, (void*)"Height");
         DBAddOption(optList, DBOPT_ZLABEL, (void*)"Depth");
         DBAddOption(optList, DBOPT_XUNITS, (void*)"parsec");
         DBAddOption(optList, DBOPT_YUNITS, (void*)"parsec");
         DBAddOption(optList, DBOPT_ZUNITS, (void*)"parsec");
         DBPutQuadmesh(db, (char *)meshName.c_str(), NULL, coords, dims,
                       3, DB_FLOAT, DB_NONCOLLINEAR, NULL);
         DBFreeOptlist(optList);
    }

    virtual void WriteMeshSlice(DBfile *db, const std::string &newMeshName,
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

         DBoptlist *optList = DBMakeOptlist(6);
         DBAddOption(optList, DBOPT_XLABEL, (void*)"Width");
         DBAddOption(optList, DBOPT_YLABEL, (void*)"Height");
         DBAddOption(optList, DBOPT_ZLABEL, (void*)"Depth");
         DBAddOption(optList, DBOPT_XUNITS, (void*)"parsec");
         DBAddOption(optList, DBOPT_YUNITS, (void*)"parsec");
         DBPutQuadmesh(db, (char *)newMeshName.c_str(), NULL, coords, dims, 2,
                       DB_FLOAT, DB_COLLINEAR, optList);
         DBFreeOptlist(optList);

         delete [] slice_coordX;
         delete [] slice_coordY;
    }
};

// 
// Template based matrix solver class.
//

template <class D> class matrix{
  // NOTE: maxsize determines available memory storage, but
  // actualsize determines the actual size of the stored matrix in use
  // at a particular time.
  int maxsize;  // max number of rows (same as max number of columns)
  int actualsize;  // actual size (rows, or columns) of the stored matrix
  D* data;      // where the data contents of the matrix are stored
  void allocate()   {
    delete[] data;
    data = new D [maxsize*maxsize];
    };
  matrix() {};                  // private ctor's
  matrix(int newmaxsize) {matrix(newmaxsize,newmaxsize);};
public:
  matrix(int newmaxsize, int newactualsize)  { // the only public ctor
    if (newmaxsize <= 0) newmaxsize = 5;
    maxsize = newmaxsize;
    if ((newactualsize <= newmaxsize)&&(newactualsize>0))
      actualsize = newactualsize;
    else
      actualsize = newmaxsize;
    // since allocate() will first call delete[] on data:
    data = 0;
    allocate();
    };
  ~matrix() { delete[] data; };
  void comparetoidentity()  {
    int worstdiagonal = 0;
    D maxunitydeviation = 0.0;
    D currentunitydeviation;
    for ( int i = 0; i < actualsize; i++ )  {
      currentunitydeviation = data[i*maxsize+i] - 1.;
      if ( currentunitydeviation < 0.0) currentunitydeviation *= -1.;
      if ( currentunitydeviation > maxunitydeviation )  {
           maxunitydeviation = currentunitydeviation;
           worstdiagonal = i;
           }
      }
    int worstoffdiagonalrow = 0;
    int worstoffdiagonalcolumn = 0;
    D maxzerodeviation = 0.0;
    D currentzerodeviation ;
    for ( int i = 0; i < actualsize; i++ )  {
      for ( int j = 0; j < actualsize; j++ )  {
        if ( i == j ) continue;  // we look only at non-diagonal terms
        currentzerodeviation = data[i*maxsize+j];
        if ( currentzerodeviation < 0.0) currentzerodeviation *= -1.0;
        if ( currentzerodeviation > maxzerodeviation )  {
          maxzerodeviation = currentzerodeviation;
          worstoffdiagonalrow = i;
          worstoffdiagonalcolumn = j;
          }

        }
      }
    cout << "Worst diagonal value deviation from unity: "
       << maxunitydeviation << " at row/column " << worstdiagonal << endl;
    cout << "Worst off-diagonal value deviation from zero: "
       << maxzerodeviation << " at row = " << worstoffdiagonalrow
       << ", column = " << worstoffdiagonalcolumn << endl;
  }
  void settoproduct(matrix& left, matrix& right)  {
    actualsize = left.getactualsize();
    if ( maxsize < left.getactualsize() )   {
      maxsize = left.getactualsize();
      allocate();
      }
    for ( int i = 0; i < actualsize; i++ )
      for ( int j = 0; j < actualsize; j++ )  {
        D sum = 0.0;
        D leftvalue, rightvalue;
        bool success;
        for (int c = 0; c < actualsize; c++)  {
            left.getvalue(i,c,leftvalue,success);
            right.getvalue(c,j,rightvalue,success);
            sum += leftvalue * rightvalue;
            }
        setvalue(i,j,sum);
        }
    }
  void copymatrix(matrix&  source)  {
    actualsize = source.getactualsize();
    if ( maxsize < source.getactualsize() )  {
      maxsize = source.getactualsize();
      allocate();
      }
    for ( int i = 0; i < actualsize; i++ )
      for ( int j = 0; j < actualsize; j++ )  {
        D value;
        bool success;
        source.getvalue(i,j,value,success);
        data[i*maxsize+j] = value;
        }
    };
  void setactualsize(int newactualsize) {
    if ( newactualsize > maxsize )
      {
        maxsize = newactualsize ; // * 2;  // wastes memory but saves
                                      // time otherwise required for
                                      // operation new[]
        allocate();
      }
    if (newactualsize >= 0) actualsize = newactualsize;
    };
  int getactualsize() { return actualsize; };
  void getvalue(int row, int column, D& returnvalue, bool& success)   {
    if ( (row>=maxsize) || (column>=maxsize)
      || (row<0) || (column<0) )
      {  success = false;
         return;    }
    returnvalue = data[ row * maxsize + column ];
    success = true;
    };
  D getvalue(int row, int column)   {
    return data[ row * maxsize + column ];
    };
  D *getrowpointer(int row) {
    return &data[ row * maxsize ] ;
    };
  bool setvalue(int row, int column, D newvalue)  {
    if ( (row >= maxsize) || (column >= maxsize)
        || (row<0) || (column<0) ) return false;
    data[ row * maxsize + column ] = newvalue;
    return true;
    };
  void invert()  {
    if (actualsize <= 0) return;  // sanity check
    if (actualsize == 1) return;  // must be of dimension >= 2
    for (int i=1; i < actualsize; i++) data[i] /= data[0]; // normalize row 0
    for (int i=1; i < actualsize; i++)  {
      for (int j=i; j < actualsize; j++)  { // do a column of L
        D sum = 0.0;
        for (int k = 0; k < i; k++)
            sum += data[j*maxsize+k] * data[k*maxsize+i];
        data[j*maxsize+i] -= sum;
        }
      if (i == actualsize-1) continue;
      for (int j=i+1; j < actualsize; j++)  {  // do a row of U
        D sum = 0.0;
        for (int k = 0; k < i; k++)
            sum += data[i*maxsize+k]*data[k*maxsize+j];
        data[i*maxsize+j] =
           (data[i*maxsize+j]-sum) / data[i*maxsize+i];
        }
      }
    for ( int i = 0; i < actualsize; i++ )  // invert L
      for ( int j = i; j < actualsize; j++ )  {
        D x = 1.0;
        if ( i != j ) {
          x = 0.0;
          for ( int k = i; k < j; k++ )
              x -= data[j*maxsize+k]*data[k*maxsize+i];
          }
        data[j*maxsize+i] = x / data[j*maxsize+j];
        }
    for ( int i = 0; i < actualsize; i++ )   // invert U
      for ( int j = i; j < actualsize; j++ )  {
        if ( i == j ) continue;
        D sum = 0.0;
        for ( int k = i; k < j; k++ )
            sum += data[k*maxsize+j]*( (i==k) ? 1.0 : data[i*maxsize+k] );
        data[i*maxsize+j] = -sum;
        }
    for ( int i = 0; i < actualsize; i++ )   // final inversion
      for ( int j = 0; j < actualsize; j++ )  {
        D sum = 0.0;
        for ( int k = ((i>j)?i:j); k < actualsize; k++ )
            sum += ((j==k)?1.0:data[j*maxsize+k])*data[k*maxsize+i];
        data[j*maxsize+i] = sum;
        }
    };
};

typedef matrix<double> DoubleMatrix;

//
// Global values
//
static int G_nPoints = 100;
static float *G_coords[3] = {0,0,0};
static float *G_values = 0;

static double *G_Ci = 0;
static double G_r = 1.;
static double G_rSquared = 1.;
static double G_alpha = -0.5;

// ****************************************************************************
// Function: ShepardGlobal
//
// Purpose:
//   Callback function that creates data by interpolating scattered data
//   points using Shepard's global method.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:56:07 PST 2002
//
// Modifications:
//   
// ****************************************************************************

float
ShepardGlobal(float *pt, QuadMesh3D *)
{
    // Find the value using global shepard's method.
    double sum_di2inv = 0.;
    double sum_fi_di2inv = 0.;

    for(int i = 0; i < G_nPoints; ++i)
    {
        double dX = pt[0] - G_coords[0][i];
        double dY = pt[1] - G_coords[1][i];
        double dZ = pt[2] - G_coords[2][i];

        double di2inv = 1. / (dX*dX + dY*dY + dZ*dZ);
        double fi_di2inv = double(G_values[i]) * di2inv;

        // Add them to the sums.
        sum_di2inv += di2inv;
        sum_fi_di2inv += fi_di2inv;
    }

    float S = float(sum_fi_di2inv / sum_di2inv);
    return S;
}

// ****************************************************************************
// Function: HardyGlobal
//
// Purpose:
//   Callback function that creates data by interpolating scattered data
//   points using Hardy's global method.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:56:07 PST 2002
//
// Modifications:
//   
// ****************************************************************************

float
HardyGlobal(float *pt, QuadMesh3D *)
{
    double sum = 0.;

    for(int i = 0; i < G_nPoints; ++i)
    {
        double dX = pt[0] - G_coords[0][i];
        double dY = pt[1] - G_coords[1][i];
        double dZ = pt[2] - G_coords[2][i];
        double di2 = (dX*dX + dY*dY + dZ*dZ);

        // G_alpha is 0.5 or -0.5
        sum += (G_Ci[i] * pow(G_rSquared + di2, G_alpha));
    }

    return float(sum);
}

// ****************************************************************************
// Function: Radial
//
// Purpose: 
//   Callback function that creates data that is the distance from one
//   opposite corner to another.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 4 14:20:37 PST 2002
//
// Modifications:
//   
// ****************************************************************************

float
Radial(float *pt, QuadMesh3D *)
{
    float dX = pt[0] - XMAX;
    float dY = pt[1] - YMAX;
    float dZ = pt[2] - ZMAX;
    return (float)sqrt(dX*dX + dY*dY + dZ*dZ);
}

// ****************************************************************************
// Method: IncreasingX
//
// Purpose: 
//   Callback function that creates scalar data that is the same as the X coord.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 4 14:21:32 PST 2002
//
// Modifications:
//   
// ****************************************************************************

float
IncreasingX(float *pt, QuadMesh3D *)
{
    return pt[0];
}

// ****************************************************************************
//   Functions for creating the material.
// ****************************************************************************

bool
PointInSphere(float pt[3], float origin[3], float rad)
{
    float dX = pt[0] - origin[0];
    float dY = pt[1] - origin[1];
    float dZ = pt[2] - origin[2];
    
    return (sqrt(dX*dX + dY*dY + dZ*dZ) <= rad);
}

void
MatRule(float *pt, int *matnos, double *matVf, int *nMats, int *dims)
{
    float origin1[] = {0.,0.,0.};
    float rad1 = 1.3 * 7.;
    float rad2 = 1.1 * 7.;

    float dX = pt[0] - origin1[0];
    float dY = pt[1] - origin1[1];
    float dZ = pt[2] - origin1[2];
    float dist = sqrt(dX*dX + dY*dY + dZ*dZ);

    float idX = (XMAX-XMIN) / float(dims[0]);
    float idY = (YMAX-YMIN) / float(dims[1]);
    float idZ = (ZMAX-ZMIN) / float(dims[2]);
    float hyp = sqrt(idX*idX + idY*idY + idZ*idZ);

    if(dist < (rad2 - hyp))
    {
        // Clean inner air
        matnos[0] = 1;
        matVf[0] = 1.;
        *nMats = 1;
    }
    else if(dist > (rad1 + hyp))
    {
        // Clean outer air
        matnos[0] = 1;
        matVf[0] = 1.;
        *nMats = 1;
    }
    else
    {
        // probably mixed
        float tetrad = 1.6 * 7.;
        float holerad = 1.2 * 7.;
        float tetdX = tetrad * cos(30. * 6.28/360.);
        float tetdY = tetrad * sin(30. * 6.28/360.);
        float origin_a[] = {origin1[0], origin1[1]+tetrad, origin1[2]};
        float origin_b[] = {origin1[0]-tetdX, origin1[1]-tetdY, origin1[2]+tetdY};
        float origin_c[] = {origin1[0]+tetdX, origin1[1]-tetdY, origin1[2]+tetdY};
        float origin_d[] = {origin1[0], origin1[1]-tetdY, origin1[2]-tetrad};

#define SUBDIV 4.
        int air = 0;
        int chrome = 0;
        int count = 0;

        for(float x = pt[0]; x < pt[0] + idX; x += (idX / SUBDIV))
            for(float y = pt[1]; y < pt[1] + idY; y += (idY / SUBDIV))
                for(float z = pt[2]; z < pt[2] + idY; z += (idZ / SUBDIV))
                {
                    float pt2[] = {x,y,z};
                    if(PointInSphere(pt2, origin1, rad1) &&
                       !PointInSphere(pt2, origin1, rad2))
                    {
                        // The point is in the chrome
                        if(PointInSphere(pt2, origin_a, holerad) ||
                           PointInSphere(pt2, origin_b, holerad) ||
                           PointInSphere(pt2, origin_c, holerad) ||
                           PointInSphere(pt2, origin_d, holerad))
                        {
                            // The point is in an area that is subtracted
                            // away. Make it air.
                            ++air;
                        }
                        else
                            ++chrome;
                    }
                    else
                        ++air;
                    ++count;
                }

         // Figure out the volume fractions
         if(air == 0)
         {
             matnos[0] = 2; // totally chrome
             matVf[0] = 1.;
             *nMats = 1;
         }
         else if(chrome == 0)
         {
             matnos[0] = 1; // totally air
             matVf[0] = 1.;
             *nMats = 1;
         }
         else
         {
             matnos[0] = 1;
             matnos[1] = 2;
             matVf[0] = float(air) / float(count);
             matVf[1] = 1. - matVf[0];
             *nMats = 2;
         }
    }
}

void
CreateMaterial(int i, int j, int k, int *matnos, double *matVf, int *nMats,
    QuadMesh3D *m)
{
    float pt[3];
    pt[0] = m->GetX(i,j,k);
    pt[1] = m->GetY(i,j,k);
    pt[2] = m->GetZ(i,j,k);

    int dims[3];
    dims[0] = m->XDim();
    dims[1] = m->YDim();
    dims[2] = m->ZDim();

    MatRule(pt, matnos, matVf, nMats, dims);
}

float
AirVf(int i, int j, int k, QuadMesh3D *m)
{
    int    matnos[2];
    double matVf[2];
    int    nMats;

    float pt[3];
    pt[0] = m->GetX(i,j,k);
    pt[1] = m->GetY(i,j,k);
    pt[2] = m->GetZ(i,j,k);

    int dims[3];
    dims[0] = m->XDim();
    dims[1] = m->YDim();
    dims[2] = m->ZDim();

    MatRule(pt, matnos, matVf, &nMats, dims);

    float retval = float(matVf[0]);
    if(nMats == 1)
    {
        if(matnos[0] == 1)
            retval = 1.;
        else
            retval = 0.;
    }

    return retval;
}

float
ChromeVf(int i, int j, int k, QuadMesh3D *m)
{
    int    matnos[2];
    double matVf[2];
    int    nMats;

    float pt[3];
    pt[0] = m->GetX(i,j,k);
    pt[1] = m->GetY(i,j,k);
    pt[2] = m->GetZ(i,j,k);

    int dims[3];
    dims[0] = m->XDim();
    dims[1] = m->YDim();
    dims[2] = m->ZDim();

    MatRule(pt, matnos, matVf, &nMats, dims);

    float retval;
    if(nMats == 1)
    {
        if(matnos[0] == 2)
            retval = 1.;
        else
            retval = 0.;
    }
    else
        retval = float(matVf[1]);

    return retval;
}

// ****************************************************************************
// Function: InitializeHardyGlobal
//
// Purpose:
//   Finds coefficients required for Hardy's method.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:57:09 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
InitializeHardyGlobal()
{
    int    i, j;

    //
    // Find coefficients for the system.
    //
    G_rSquared = G_r * G_r;

    // Create the F vector.
    double *F = new double[G_nPoints];
    for(i = 0; i < G_nPoints; ++i)
        F[i] = double(G_values[i]);

    // Create and fill the M matrix.
    DoubleMatrix M(G_nPoints, G_nPoints);
    for(j = 0; j < G_nPoints; ++j)
    {
        float p1[3] = {G_coords[0][j], G_coords[1][j], G_coords[2][j]};

        double *rowptr = M.getrowpointer(j);

        for(i = 0; i < G_nPoints; ++i)
        {
            double dX = p1[0] - G_coords[0][i];
            double dY = p1[1] - G_coords[1][i];
            double dZ = p1[2] - G_coords[2][i];
            double di2 = dX*dX + dY*dY + dZ*dZ;

            // Put the value into the element m[j][i].
            *rowptr++ = pow(G_rSquared + di2, G_alpha);
        }
    }

    // Invert the matrix to find the coefficients Ci.
    M.invert();
    G_Ci = new double[G_nPoints];
    for(j = 0; j < G_nPoints; ++j)
    {
        G_Ci[j] = 0.;
        double *rowptr = M.getrowpointer(j);
        for(i = 0; i < G_nPoints; ++i)
            G_Ci[j] += (rowptr[i] * F[i]);
    }

    delete [] F;
}

// ****************************************************************************
// Function:  WritePoints
//
// Purpose:
//   Writes the point mesh and the point var that represent the scattered data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:58:56 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Jul 21 14:39:02 PST 2004
//   Added units for the pointmesh and pointvar.
//
// ****************************************************************************

void WritePoints(DBfile *db)
{
    // Write the point mesh
    DBoptlist *optlist = DBMakeOptlist(6);
    DBAddOption(optlist, DBOPT_XLABEL, (void*)"Width");
    DBAddOption(optlist, DBOPT_YLABEL, (void*)"Height");
    DBAddOption(optlist, DBOPT_ZLABEL, (void*)"Depth");
    DBAddOption(optlist, DBOPT_XUNITS, (void *)"parsec");
    DBAddOption(optlist, DBOPT_YUNITS, (void *)"parsec");
    DBAddOption(optlist, DBOPT_ZUNITS, (void *)"parsec");
    float *pcoords[3] = {G_coords[0], G_coords[1], G_coords[2]};
    DBPutPointmesh(db, "PointMesh", 3, pcoords, G_nPoints, DB_FLOAT, optlist);
    DBFreeOptlist(optlist);

    // Write the point var
    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Joules");
    float *vals[1] = {G_values};
    DBPutPointvar(db, "PointVar", "PointMesh", 1, vals, G_nPoints, DB_FLOAT, optlist);
    DBFreeOptlist(optlist);
}

// ****************************************************************************
// Function: GetRandomNumber
//
// Purpose: 
//   Returns a random number.
//
// Returns:    A random number.
//
// Note:       This function returns numbers from a table because it was
//             reported that drand48 was not returning the same results on
//             some platforms, which resulted in a different noise database.
//             If more than 400 random numbers are required, disable the
//             PORTABLE_RANDOM_NUMBERS macro.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 2 11:48:41 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

#define PORTABLE_RANDOM_NUMBERS
double GetRandomNumber()
{
#ifdef PORTABLE_RANDOM_NUMBERS
    static int random_counter = 0;
    static const double portable_random_data[] = {
        0.396464773760275, 0.840485369411425, 0.353336097245244, 0.446583434796544,
        0.318692772311881, 0.886428433223031, 0.0155828494083288,
        0.584090220317272, 0.159368626531805, 0.383715874807194,
        0.691004373382196, 0.0588589135927364, 0.899854306161604,
        0.163545950630365, 0.159071502581806, 0.533064714021855,
        0.604144189711239, 0.582699021207219, 0.269971117907016,
        0.390478195463409, 0.293400570118951, 0.742377406033981,
        0.298525606318119, 0.0755380785377824, 0.404982633583334,
        0.857377942708183, 0.941968323291899, 0.662830659789996,
        0.846475779930007, 0.00275508142688352, 0.462379245025485,
        0.532596024438298, 0.78787662089292, 0.265612234971371,
        0.98275226310103, 0.30678513061418, 0.600855136489105,
        0.608715653358658, 0.212438798201187, 0.885895130587606,
        0.304657101745793, 0.15185986406857, 0.337661902873531,
        0.387476950965358, 0.643609828900129, 0.753553275640016,
        0.603616098781568, 0.53162825175081, 0.459360316334315,
        0.652488446971034, 0.32718116385065, 0.946370485960081,
        0.368039867432817, 0.943890339354468, 0.00742826171906685,
        0.516599949702389, 0.272770952753351, 0.024299155634651,
        0.591954502437812, 0.2049635097516, 0.877693349889729,
        0.0593686933802502, 0.260842551926938, 0.302829184161332,
        0.891495219672155, 0.49819805913441, 0.710025580792159,
        0.286413993907622, 0.86492357739947, 0.675540671125631,
        0.458489973232272, 0.95963556238106, 0.774675406127844,
        0.376551280801323, 0.228639116426205, 0.354533877294422,
        0.300318248151815, 0.669765831680721, 0.718966572477935,
        0.56595450872225, 0.82446531320608, 0.390611909814908,
        0.818766311218223, 0.844008460045423, 0.180467770090349,
        0.943395886088908, 0.424886765414069, 0.520665778036708,
        0.065643754874575, 0.913508169204363, 0.882584572720003,
        0.761364126692378, 0.398922546078257, 0.688256841941055,
        0.761548303519756, 0.405008799190391, 0.125251137735066,
        0.484633904711558, 0.222462553152592, 0.873121166037272,
        0.528821897552877, 0.00141396199556709, 0.860513813395496,
        0.0186973975537157, 0.814891941959363, 0.242884021468967,
        0.314571843980655, 0.965732517843072, 0.935560105184653,
        0.80943169502747, 0.492109152867098, 0.220135507787283,
        0.576353738175104, 0.289029057945669, 0.321066976456063,
        0.26132333892145, 0.173987857556103, 0.00181729960961263,
        0.0447841404416316, 0.241174550751133, 0.415451187566848,
        0.701624649114535, 0.221845691021354, 0.503910343695601,
        0.0670302113764372, 0.393063444029124, 0.47947652725605,
        0.218142210865658, 0.21951128495812, 0.91620319329374,
        0.35022175096368, 0.192693936816291, 0.21123497924744,
        0.633681628042254, 0.0535653931591149, 0.783411412752308,
        0.0306739171400388, 0.444096599615705, 0.176412888209864,
        0.932180217909242, 0.909648273496249, 0.47284483467071,
        0.871695461067244, 0.695567358482201, 0.930189621212154,
        0.455090555576618, 0.398566781653198, 0.893304001032,
        0.693546747090654, 0.838734382742629, 0.739699136234915,
        0.651269549260768, 0.678154240201113, 0.577212316619637,
        0.273118280902604, 0.935388053918395, 0.661938885152541,
        0.0474125133813033, 0.373038389580241, 0.61819477787656,
        0.148627985722786, 0.377305582204912, 0.644591400844785,
        0.0256878245887222, 0.841380513085102, 0.0770188061447783,
        0.74263391670635, 0.255986140742344, 0.901844229953472,
        0.377647585319551, 0.319529924909983, 0.211434985403777,
        0.648532499829447, 0.251315083019463, 0.228644316759183,
        0.250922180856598, 0.943221161675812, 0.136715084880649,
        0.270060767431982, 0.548707187140856, 0.324021447266222,
        0.865080020970865, 0.296703347762357, 0.680059335513555,
        0.833147253665036, 0.876307512984511, 0.649664557992974,
        0.0731154781698606, 0.898546969277891, 0.253581256254577,
        0.611318627569684, 0.841889925089895, 0.832019301181166,
        0.372587091455188, 0.757048223143908, 0.109214906307621,
        0.850811976487268, 0.55932932619065, 0.857580462076012,
        0.343096981736231, 0.6917696038328, 0.345197294645459,
        0.893584653390956, 0.958707883428566, 0.121735195187956,
        0.981544356604459, 0.0550247948269735, 0.614776279301502,
        0.038126675578102, 0.376472597943376, 0.525604855838719,
        0.281923955409873, 0.5605379614823, 0.607242965637212,
        0.816439584668299, 0.446534988318398, 0.0271116801286304,
        0.471854013937502, 0.284898305380789, 0.292723682489203,
        0.195680144142489, 0.0175794171291983, 0.829599230575294,
        0.573165442377949, 0.104780601521615, 0.732976480693964,
        0.119097519828312, 0.223888397613173, 0.947252862210423,
        0.739051619278925, 0.821356975303129, 0.8261315647214,
        0.25097997212632, 0.256495729098447, 0.338193196867149,
        0.388221672854662, 0.527318475851974, 0.266409126781255,
        0.401220531998543, 0.870894566377494, 0.0458917928501172,
        0.294573818716259, 0.394246563234287, 0.560002386477102,
        0.310821141481274, 0.822576867147674, 0.47548149041361,
        0.0911435896732868, 0.262401092809885, 0.917044094123582,
        0.978435899004108, 0.332093345723855, 0.902074034670637,
        0.240634874411811, 0.37316437186217, 0.75228603344663,
        0.457827388234367, 0.900627876163043, 0.422894512375148,
        0.565923482910826, 0.97046215686176, 0.353870875923334,
        0.431506514559569, 0.179091358788433, 0.215319112863117,
        0.33717676507672, 0.454367752847968, 0.0447257049004079,
        0.683882454393814, 0.0621491404333625, 0.551375283486319,
        0.316171236642838, 0.268217904607642, 0.500765994969726,
        0.0625369631847654, 0.964826538534915, 0.72957579086405,
        0.806315558174489, 0.151974411416315, 0.705017961093425,
        0.729957445740386, 0.842565841557473, 0.617928583041344,
        0.955447777107601, 0.479388126336563, 0.527085486138574,
        0.0293865902502226, 0.607612114663588, 0.0475693663771644,
        0.59276964764976, 0.40264692966208, 0.904394395449408,
        0.330131069414808, 0.310837454255406, 0.854944744294666,
        0.734149534459185, 0.0986265335186864, 0.340458593324797,
        0.401739493528492, 0.985167063709561, 0.629594780693527,
        0.831962390675372, 0.437119729627849, 0.791226690481601,
        0.0594618604167785, 0.1351349002463, 0.543823899352059,
        0.930451652408131, 0.884899018605683, 0.389894586859256,
        0.414557516192215, 0.856862649939352, 0.286758583530943,
        0.107756722076772, 0.576675995301894, 0.777602183838702,
        0.142250931180048, 0.808677185678292, 0.748094221754794,
        0.410024863886342, 0.475115766840876, 0.344459535440109,
        0.416798845857908, 0.0236477473992593, 0.52581100972505,
        0.71789153822759, 0.236022679375846, 0.69486507501491,
        0.779950605780559, 0.762750408386637, 0.12156271618009,
        0.370517958929419, 0.928251432954529, 0.766006337556078,
        0.289579532526428, 0.983660707931783, 0.428106915559571,
        0.037813085813724, 0.598326448384523, 0.0155956066321394,
        0.756723842570995, 0.130679000016059, 0.374567164495513,
        0.217496165745068, 0.577797001137217, 0.205663393795078,
        0.78702094261676, 0.748369188658025, 0.455336403822049,
        0.285268327435531, 0.850220081227288, 0.450007519290402,
        0.835011918615127, 0.430246027271863, 0.330995768313198,
        0.950872109930351, 0.248872817045555, 0.356794045816386,
        0.417870679229249, 0.497053137824388, 0.885292950313929,
        0.56316534805876, 0.685500243541053, 0.967759368288945,
        0.182368758719225, 0.567805378761605, 0.0296646426917242,
        0.203961872513439, 0.157764928299631, 0.547594382618087,
        0.203990632807585, 0.211286622531027, 0.0672940430302056,
        0.193097376334592, 0.901521526459444, 0.782049466546162,
        0.302589853625339, 0.422007771555116, 0.989525426229175,
        0.79712059742716, 0.387471607480325, 0.181940282553754,
        0.224731393995793, 0.737382928197398, 0.533507071053922,
        0.664735985287706, 0.2018309754509, 0.468823843290416,
        0.566497238542599, 0.0974637872254647, 0.277023691980357,
        0.0173116656720467, 0.563884071527273, 0.886939682843376,
        0.0701900742072148, 0.0620209301768924, 0.253306765884634,
        0.307064570318531, 0.896639855680402, 0.134073245056804
    };
    int index = random_counter % 400;
    random_counter++;
    return portable_random_data[index];
#else
    return drand48();
#endif
}


// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for the noise program.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:57:58 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Jul 29 16:56:21 PST 2002
//   I changed the program so point meshes are written by default and added
//   command line arguments that allow the mesh resolution to be specified.
//
//   Brad Whitlock, Fri Oct 4 11:17:01 PDT 2002
//   I added a couple new scalar variables and a new vector variables.
//
//   Brad Whitlock, Fri Mar 14 09:41:51 PDT 2003
//   I added a slice of hardyglobal.
//
//   Hank Childs, Mon Dec  1 09:12:58 PST 2003
//   Added tensor data.
//
//   Brad Whitlock, Wed Feb 2 11:51:41 PDT 2005
//   I changed drand48 to GetRandomNumber.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int isteps = ISTEPS;
    int jsteps = JSTEPS;
    int ksteps = KSTEPS;
    int driver = DB_PDB;

    // Look through command line args.
    bool writePoints = true;
    for(int j = 1; j < argc; ++j)
    {
        if(std::string(argv[j]) == std::string("-nopoints"))
            writePoints = false;
        else if(std::string(argv[j]) == std::string("-nx") && ((j+1) < argc))
        {
            int tmp;
            if(sscanf(argv[j+1], "%d", &tmp) == 1)
            {
                if(tmp > 0)
                    isteps = tmp;
            }
            ++j;
        }
        else if(std::string(argv[j]) == std::string("-ny") && ((j+1) < argc))
        {
            int tmp;
            if(sscanf(argv[j+1], "%d", &tmp) == 1)
            {
                if(tmp > 0)
                    jsteps = tmp;
            }
            ++j;
        }
        else if(std::string(argv[j]) == std::string("-nz") && ((j+1) < argc))
        {
            int tmp;
            if(sscanf(argv[j+1], "%d", &tmp) == 1)
            {
                if(tmp > 0)
                    ksteps = tmp;
            }
            ++j;
        }
        else if (strcmp(argv[j], "-driver") == 0)
        {
            j++;

            if (strcmp(argv[j], "DB_HDF5") == 0)
            {
                driver = DB_HDF5;
            }
            else if (strcmp(argv[j], "DB_PDB") == 0)
            {
                driver = DB_PDB;
            }
            else
            {
               fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                   argv[j]);
            }
        }
    }

#ifndef PORTABLE_RANDOM_NUMBERS
    // Set the seed for the random number generator.
    srand(87654321);
#endif

    DBfile *db;
    db = DBCreate("noise.silo", DB_CLOBBER, DB_LOCAL, "VisIt noise dataset", driver);

    RectilinearMesh3D B(isteps, jsteps, ksteps);
    B.SetXValues(XMIN, XMAX);
    B.SetYValues(YMIN, YMAX);
    B.SetZValues(ZMIN, ZMAX);

    // Create some scattered points with values.
    G_coords[0] = new float[G_nPoints];
    G_coords[1] = new float[G_nPoints];
    G_coords[2] = new float[G_nPoints];
    G_values = new float[G_nPoints];
    float dX = XMAX - XMIN;
    float dY = YMAX - YMIN;
    float dZ = ZMAX - ZMIN;
    for(int i = 0; i < G_nPoints; ++i)
    {
        // Save the made-up coordinate.
        G_coords[0][i] = (GetRandomNumber() * dX + XMIN);
        G_coords[1][i] = (GetRandomNumber() * dY + YMIN);
        G_coords[2][i] = (GetRandomNumber() * dZ + ZMIN);
        // Fill the value at that coordinate
        G_values[i] = (GetRandomNumber() * 5. + 1.);
    }

    // Initialize the hardy global matrix
    InitializeHardyGlobal();

    B.CreateNodalData("shepardglobal", ShepardGlobal);
    B.CreateNodalData("hardyglobal", HardyGlobal);
    B.CreateGradient("hardyglobal", "grad");
    B.CreateGradientTensor("hardyglobal", "grad_tensor");
    B.CreateNodalData("x", IncreasingX);
    B.CreateNodalData("radial", Radial);
    B.CreateZonalData("airVf", AirVf);
    B.CreateGradient("airVf", "airVfGradient");
    B.CreateZonalData("chromeVf", ChromeVf);
    B.AddMaterial("air");
    B.AddMaterial("chrome");
    B.CreateMaterialData(CreateMaterial);
    B.AddSlice("hardyglobal", "Mesh2D", "hgslice", isteps/2, 0);
    B.WriteFile(db);
    if(writePoints)
        WritePoints(db);
    DBClose(db);


    // Delete the global arrays
    delete [] G_coords[0];
    delete [] G_coords[1];
    delete [] G_coords[2];
    delete [] G_values;
    delete [] G_Ci;

    return 0;
}
