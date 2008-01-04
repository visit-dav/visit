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
//                         UnstructuredTopology.h                            //
// ************************************************************************* //

#ifndef UNSTRUCTURED_TOPOLOGY_H
#define UNSTRUCTURED_TOPOLOGY_H
#include <siloobj_exports.h>

#include <visitstream.h>
#include <silo.h>

#include <SiloObjLib.h>
#include <Value.h>


// ****************************************************************************
//  Class: UnstructuredTopology
//
//  Purpose:
//      Keeps information about an unstructured topology.
//
//  Data Members:
//      nodeList    -  The name of the node list value object.
//      shapeCount  -  The name of the shape count value object.
//      shapeType   -  The name of the shape type value object.
//      shapeSize   -  The name of the shape size value object.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

class SILOOBJ_API UnstructuredTopology
{
  public:
                     UnstructuredTopology();
                     UnstructuredTopology(FOR_DERIVED_TYPES_e);
    virtual         ~UnstructuredTopology();
  
    char            *GetName(void)   { return name; };

    void             PrintSelf(ostream &);
    void             Read(DBobject *, DBfile *);
    virtual void     Write(DBfile *);
   
  protected:
    char            *nodeList;
    char            *shapeCount;
    char            *shapeType;
    char            *shapeSize;

    char            *name;

    Value           *nodeListValue;
    Value           *shapeCountValue;
    Value           *shapeTypeValue;
    Value           *shapeSizeValue;

    // Protected Methods
    void             CreateValues(void);
    void             RealConstructor(void);
    
    // Class-scoped constants
  public:
    static char * const   NAME;
    static char * const   SILO_TYPE;
  protected:
    static char * const   SILO_NAME;
    static int    const   SILO_NUM_COMPONENTS;
    static char * const   SILO_NODE_LIST_NAME;
    static char * const   SILO_SHAPE_COUNT_NAME;
    static char * const   SILO_SHAPE_TYPE_NAME;
    static char * const   SILO_SHAPE_SIZE_NAME;
};


#endif


