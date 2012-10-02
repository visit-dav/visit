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

#ifndef VISITMESH_H
#define VISITMESH_H

#include "ddtsim_constants.h"
#include <VisItInterfaceTypes_V2.h>
#include "libsimlibrary.h"
#include "visitsim.h"

//! Class representing a single mesh to be visualised with VisIt
/*!
 * Most of the data members are statically allocated, and so can be safely set via gdb.
 * The arrays defining the coordinates of the mesh need to be allocated, this is done by calling
 * generate(). VisItMesh objects that have had their dynamic arrays generated should not be
 * used or deleted via gdb, such operations must be done in the signal handler.
 */
class VisItMesh
{
public:
    VisItMesh();
    VisItMesh(const VisItMesh&);
    ~VisItMesh();

    bool operator==(const VisItMesh&) const;
    bool isNamed(const char *) const;
    void setName(const char *);

    void setLabelX(const char *);
    void setLabelY(const char *);
    void setLabelZ(const char *);

    bool isGenerated() const;           //!< Determines if the dyanamic arrays have been generated
    void generate();                    //!< Generate the dyanmic arrays

    void applyMesh(LibsimLibrary&, visit_handle &mesh);
    void applyPointMesh(LibsimLibrary&, visit_handle &mesh);
    void applyRectilinearMesh(LibsimLibrary&, visit_handle &mesh);
    void applyMetadata(LibsimLibrary&, VisItSim&, visit_handle &metadata);

private:
    void setString(const char*,char*);

    VisItMeshType mType;//!< i.e. rectilinear or point
    bool mGenerated;    //!< true if the dynamic arrays have been allocated
    char mName[MAX_NAME_LENGTH];    //!< Name of the mesh. Hardcoded max-size
    char mLabelX[MAX_NAME_LENGTH];  //!< Label for X axis. Hardcoded max-size
    char mLabelY[MAX_NAME_LENGTH];  //!< Label for Y axis. Hardcoded max-size
    char mLabelZ[MAX_NAME_LENGTH];  //!< Label for Z axis. Hardcoded max-size
    int mBaseIndex[3];  //!< Offset coordinates [x,y,z]
    int mDimension[3];  //!< Size of each [x,y,z] dimension
    int mIncrement[3];  //!< Distance between points on each dimension [x,y,z]
    int mAxisOrder[3];  //!< Axis-order in which point meshes should be populated. i.e. mAxisOrder[0]=1 means the x coordinate (0) should be increment second (1). Only used in point meshes.
    float* mCoords[3];  //!< Pointers to arrays of coordinate values [x,y,z]. May be axis marks along each axis in a rectilinear mesh, or the (x,y,z) coordinates of each point in a point mesh
    int mNumPoints;     //!< Number of points in a point mesh. Set by generate().
};

#endif // VISITMESH_H
