/*****************************************************************************
*
* Copyright (c) 2011, Allinea
* All rights reserved.
*
* This file has been contributed to the VisIt project, which is
* Copyright (c) Lawrence Livermore National Security, LLC. For  details, see
* https://visit.llnl.gov/.  The full copyright notice is contained in the 
* file COPYRIGHT located at the root of the VisIt distribution or at 
* http://www.llnl.gov/visit/copyright.html.
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

#include "visitmesh.h"
#include "libsimlibrary.h"
#include "ddtsim.h"

#include <stdio.h>
#include <string.h>

VisItMesh::VisItMesh() : mType(DDTSIM_MESH_RECTILINEAR), mGenerated(false), mNumPoints(0)
{
    mName[0] = '\0';
    mLabelX[0] = '\0';
    mLabelY[0] = '\0';
    mLabelZ[0] = '\0';
    for (int dim=0; dim<3; ++dim)
    {
        mBaseIndex[dim] = 0;
        mIncrement[dim] = 1;
        mDimension[dim] = 1;
        mAxisOrder[dim] = dim;
        mCoords[dim] = 0;
    }
}

VisItMesh::VisItMesh(const VisItMesh &mesh) : mType(mesh.mType), mGenerated(false), mNumPoints(0)
{
    strcpy(mName, mesh.mName);
    strcpy(mLabelX, mesh.mLabelX);
    strcpy(mLabelY, mesh.mLabelY);
    strcpy(mLabelZ, mesh.mLabelZ);
    for (int dim=0; dim<3; ++dim)
    {
        mBaseIndex[dim] = mesh.mBaseIndex[dim];
        mIncrement[dim] = mesh.mIncrement[dim];
        mDimension[dim] = mesh.mDimension[dim];
        mAxisOrder[dim] = mesh.mAxisOrder[dim];
        mCoords[dim] = 0;       // Do not copy - this is generated from the other values
    }

    if (strlen(mName)==0)
        DDTSim::error("(ddtsim,VisItMesh) Warning: zero-length name given to copy constructor\n");
}

VisItMesh::~VisItMesh()
{
    for (int dim=0; dim<3; ++dim) // for each dimension (0=x, 1=y, 2=z)
        delete[] mCoords[dim];
}

bool
VisItMesh::operator ==(const VisItMesh& other) const
{
    if (!isNamed(other.mName) || mType!=other.mType)
        return false;

    for (int dim=0; dim<3; ++dim)
    {
        if (mBaseIndex[dim] != other.mBaseIndex[dim]) return false;
        if (mIncrement[dim] != other.mIncrement[dim]) return false;
        if (mDimension[dim] != other.mDimension[dim]) return false;
        if (mAxisOrder[dim] != other.mAxisOrder[dim]) return false;
        // Do not check mCoords, that is generated from the other values
    }

    return true;
}

//! Determines if this object has the specified name
bool
VisItMesh::isNamed(const char *name) const
{
    return strcmp(name,&mName[0])==0;
}

//! Set the name of this mesh
/*! May be called directly within gdb */
void
VisItMesh::setName(const char *n)
{
    setString(n,mName);
}

void
VisItMesh::setLabelX(const char *n)
{
    setString(n,mLabelX);
}

void
VisItMesh::setLabelY(const char *n)
{
    setString(n,mLabelY);
}

void
VisItMesh::setLabelZ(const char *n)
{
    setString(n,mLabelZ);
}

void
VisItMesh::setString(const char *n, char *dest)
{
    if (strlen(n) > MAX_NAME_LENGTH-1) {
        strncpy(dest,n,MAX_NAME_LENGTH-2);
        dest[MAX_NAME_LENGTH-1] = '\0';
    } else
        strcpy(dest, n);
}

//! Determines if the dynamically generated arrays containing mesh point coordinates have been generated yet
bool
VisItMesh::isGenerated() const
{
    return mGenerated;
}

//! Generates dynamically allocated arrays defining the coordinates of mesh points
void
VisItMesh::generate()
{
    if (mType==DDTSIM_MESH_RECTILINEAR)
    {
        for (int dim=0; dim<3; ++dim) // for each dimension (0=x, 1=y, 2=z)
        {
            // Allocate array for axis increments
            mCoords[dim] = new float[mDimension[dim]];

            int val = mBaseIndex[dim];

            // Populate array;
            float* ptr = mCoords[dim];
            for(int i=0; i<mDimension[dim]; ++i)
            {
                *(ptr++) = val;
                val += mIncrement[dim];
            }
        }
    }
    else if (mType==DDTSIM_MESH_POINT)
    {
        // Point meshes support arranging their points in an order other than
        // increment x first, then y, then z.

        const bool is3D = mDimension[2] > 1;
        const int zLength = is3D? mDimension[2] : 1;

        // How many points will there be in this mesh
        mNumPoints = mDimension[0]*mDimension[1]*zLength;

        // Allocate arrays. Don't allocate one for Z axis if we are a 2D mesh
        mCoords[0] = new float[mNumPoints];
        mCoords[1] = new float[mNumPoints];
        if (is3D)
            mCoords[2] = new float[mNumPoints];
        else
            mCoords[2] = 0;

        // Populate arrays. Each array index represents the the x, y, and (optional) z coordinates
        // of a point in the mesh. Arrange points in grid formation.
        // mAxisOrder defines the order in which the grid should be laid out.
        // The value of mAxisOrder[0] is the axis that should be incremented first (x=0,y=1,z=2),
        // mAxisOrder[1] that to be incremented second etc.
        float *xPtr = mCoords[0];
        float *yPtr = mCoords[1];
        float *zPtr = mCoords[2];

        int count[3];
        int length[3];
        for (int i=0; i<3; ++i)
        {
            count[i]=0;
            if (mAxisOrder[i]==2 && !is3D) // If this is the z axis and we're not doing 3D
                length[i] = 1;
            else
                length[i] = mDimension[mAxisOrder[i]];
        }

        for (count[2]=0;count[2]<length[2];++count[2])
            for (count[1]=0;count[1]<length[1];++count[1])
                for (count[0]=0;count[0]<length[0];++count[0])
                {
                    for (int i=0; i<3; ++i)
                    {
                        switch(mAxisOrder[i])
                        {
                        case 0: *(xPtr++) = count[i] + mBaseIndex[0]; break;
                        case 1: *(yPtr++) = count[i] + mBaseIndex[1]; break;
                        case 2:
                            if (is3D)
                                *(zPtr++) = count[i] + mBaseIndex[2]; break;
                        default:
                            DDTSim::error("(ddtsim,visitmesh::generate) Bad axis order in point mesh\n");
                        }
                    }
                }
    }
    else
    {
        DDTSim::error("(ddtsim,VisItMesh) Unable to generate mesh, unrecognised mesh type\n");
        return;
    }

    mGenerated = true;
}

//! Adds mesh metadata to an existing visit_handle metadata object
/*! \param libsim reference to the libsim library
    \param metadata an allocated metadata visit_handle */
void
VisItMesh::applyMetadata(LibsimLibrary &libsim, VisItSim &visitSim, visit_handle &metadata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if (libsim.MeshMetaData_alloc(&h) == VISIT_OKAY)
    {
        int type;
        switch(mType)
        {
        case DDTSIM_MESH_RECTILINEAR: type = VISIT_MESHTYPE_RECTILINEAR; break;
        case DDTSIM_MESH_POINT:       type = VISIT_MESHTYPE_POINT;       break;
        default:
            DDTSim::error("(ddtsim,VisItMesh) Unknown mesh type\n");
            return;
        }

        const int numDims = mDimension[2]>1? 3 : 2; // Do we have a z-dimension

        libsim.MeshMetaData_setName(h, mName);
        libsim.MeshMetaData_setMeshType(h, type);
        libsim.MeshMetaData_setTopologicalDimension(h, numDims);
        libsim.MeshMetaData_setSpatialDimension(h, numDims);
        libsim.MeshMetaData_setNumDomains(h,visitSim.size());

        if (DDTSim::getInstance()->isLogging())
        {
            fprintf(stderr,"(ddtsim) MeshMetadata> name=%s, type=%i, topological_dims=%i, spatial_dims=%i, num_domains=%i\n",
                    mName,type,numDims,numDims,visitSim.size());
        }

        // Unhelpful in the general case, but could be useful i.e. if one dimension
        // corresponds to process rank.
        //libsim.VisIt_MeshMetaData_setXUnits(h, "cm");
        //libsim.VisIt_MeshMetaData_setYUnits(h, "cm");

        libsim.MeshMetaData_setXLabel(h, mLabelX);
        libsim.MeshMetaData_setYLabel(h, mLabelY);
        libsim.MeshMetaData_setZLabel(h, mLabelZ);

        libsim.SimulationMetaData_addMesh(metadata, h);
    }
    else
        DDTSim::error("(ddtsim,VisItMesh) failed to allocate handle for VisIt mesh metadata\n");
}

//! Allocates and writes mesh information to visit_handle h
/*! \param libsim reference to the libsim library
    \param h an unallocated visit_handle */
void
VisItMesh::applyMesh(LibsimLibrary &libsim, visit_handle &h)
{
    switch(mType)
    {
    case DDTSIM_MESH_RECTILINEAR: applyRectilinearMesh(libsim,h); break;
    case DDTSIM_MESH_POINT:       applyPointMesh(libsim,h);       break;
    default:
        DDTSim::error("(ddtsim,VisItMesh) Unable to apply mesh, unknown mesh type\n");
        return;
    }
}

void
VisItMesh::applyPointMesh(LibsimLibrary &libsim, visit_handle &h)
{
    /* Allocate a point mesh */
    if (libsim.PointMesh_alloc(&h) == VISIT_OKAY)
    {
        /* Define the coordinate system used by this mesh */
        visit_handle hxc;
        libsim.VariableData_alloc(&hxc);
        libsim.VariableData_setDataF(hxc, VISIT_OWNER_SIM, 1, mNumPoints, mCoords[0]);

        visit_handle hyc;
        libsim.VariableData_alloc(&hyc);
        libsim.VariableData_setDataF(hyc, VISIT_OWNER_SIM, 1, mNumPoints, mCoords[1]);

        if (mDimension[2] > 1)
        {   // This is a three dimensional mesh
            visit_handle hzc;
            libsim.VariableData_alloc(&hzc);
            libsim.VariableData_setDataF(hzc, VISIT_OWNER_SIM, 1, mNumPoints, mCoords[2]);
            libsim.PointMesh_setCoordsXYZ(h, hxc, hyc, hzc);
        }
        else
        {
            // This is a two dimensional mesh
            libsim.PointMesh_setCoordsXY(h, hxc, hyc);
        }

        if (DDTSim::getInstance()->isLogging())
        {
            fprintf(stderr,"(ddtsim) point mesh> name=%s, dims=(%i,%i,%i)\n",mName,mDimension[0],mDimension[1],mDimension[2]);
        }
    }
    else
        DDTSim::error("(ddtsim,VisItMesh) failed to allocate VisIt handle for point mesh\n");
}

void
VisItMesh::applyRectilinearMesh(LibsimLibrary &libsim, visit_handle &h)
{
    /* Allocate a rectilinear mesh */
    if (libsim.RectilinearMesh_alloc(&h) == VISIT_OKAY)
    {
        /* Define the coordinate system used by this mesh */
        visit_handle hxc;
        libsim.VariableData_alloc(&hxc);
        libsim.VariableData_setDataF(hxc, VISIT_OWNER_SIM, 1, mDimension[0], mCoords[0]);

        visit_handle hyc;
        libsim.VariableData_alloc(&hyc);
        libsim.VariableData_setDataF(hyc, VISIT_OWNER_SIM, 1, mDimension[1], mCoords[1]);

        if (mDimension[2] > 1)
        {   // This is a three dimensional mesh
            visit_handle hzc;
            libsim.VariableData_alloc(&hzc);
            libsim.VariableData_setDataF(hzc, VISIT_OWNER_SIM, 1, mDimension[2], mCoords[2]);
            libsim.RectilinearMesh_setCoordsXYZ(h, hxc, hyc, hzc);
        }
        else
        {
            // This is a two dimensional mesh
            libsim.RectilinearMesh_setCoordsXY(h, hxc, hyc);
        }

        if (DDTSim::getInstance()->isLogging())
        {
            fprintf(stderr,"(ddtsim) rectilinear mesh> name=%s, dims=(%i,%i,%i)\n",mName,mDimension[0],mDimension[1],mDimension[2]);
        }

        libsim.RectilinearMesh_setBaseIndex(h, mBaseIndex);
    }
    else
        DDTSim::error("(ddtsim,VisItMesh) failed to allocate VisIt handle for rectilinear mesh\n");
}
