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

// ************************************************************************* //
//                                 Mesh.h                                    //
// ************************************************************************* //

#ifndef MESH_H
#define MESH_H
#include <siloobj_exports.h>

#include <visitstream.h>
#include <silo.h>


//
// Enumerated Types
//

typedef enum
{
    CURVILINEAR                = 0,
    POINT,                    /* 1 */
    RECTILINEAR,              /* 2 */
    UNSTRUCTURED,             /* 3 */
    UNKNOWN                   /* 4 */
}  MESH_TYPE_e;

    
// ****************************************************************************
//  Class: Mesh
//
//  Purpose:
//      Keeps information about a mesh.
//
//  Data Members:
//      topology                The name of the topology object associated with
//                              the mesh.
//      intervalTree            The name of the interval tree associated with
//                              the coordinate field.
//      coordinateDimension     The dimension of the coordinates.
//      dataType                The data type of the coordinate field.
//      labels                  The labels associated with each coordinate 
//                              axis.
//      units                   The units associated with the coordinate field.
//      nDomains                The number of domains.
//      coordinates             The name of the value object with the mesh
//                              coordinates.
//      meshType                The type of the mesh.
//      
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
//  Modifications:
//      
//      Hank Childs, Sat Mar  4 09:53:12 PST 2000
//      Added meshType data member.
//
// ****************************************************************************

class SILOOBJ_API Mesh
{
  public:
                    Mesh();
    virtual        ~Mesh();

    char           *GetName() { return name; };
  
    void            PrintSelf(ostream &);
  
    void            Read(DBobject *);
    virtual void    Write(DBfile *);

    int             GetCoordinateDimension(void) {return coordinateDimension;};
    int             GetNDomains(void)            {return nDomains;};
    MESH_TYPE_e     GetMeshType(void)            {return meshType;};
    const char     *GetCoordinates(void)         {return coordinates;};

  protected:
    char           *topology;
    char           *intervalTree;
    int             coordinateDimension;
    int             dataType;
    char           *labels[3];
    char           *units[3];
    int             nDomains;
    char           *coordinates;
    MESH_TYPE_e     meshType;

    char           *name;

    // Constants
  public:
    static char * const   SILO_TYPE;
  protected:
    static int    const   SILO_NUM_COMPONENTS;

    static char * const   SILO_COORDINATE_DIMENSION_NAME;
    static char * const   SILO_COORDINATES_NAME;
    static char * const   SILO_DATA_TYPE_NAME;
    static char * const   SILO_INTERVAL_TREE_NAME;
    static char * const   SILO_LABEL_0_NAME;
    static char * const   SILO_LABEL_1_NAME;
    static char * const   SILO_LABEL_2_NAME;
    static char * const   SILO_MESH_TYPE_NAME;
    static char * const   SILO_N_DOMAINS_NAME;
    static char * const   SILO_TOPOLOGY_NAME;
    static char * const   SILO_UNIT_0_NAME;
    static char * const   SILO_UNIT_1_NAME;
    static char * const   SILO_UNIT_2_NAME;
};


#endif


