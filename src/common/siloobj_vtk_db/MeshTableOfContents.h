/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                           MeshTableOfContents.h                           //
// ************************************************************************* //

#ifndef MESH_TABLE_OF_CONTENTS_H
#define MESH_TABLE_OF_CONTENTS_H
#include <siloobj_vtk_exports.h>

#include <vtkUnstructuredGrid.h>

#include <StructuredTopology.h>
#include <TableOfContents.h>
#include <UnstructuredTopology_VTK.h>


// ****************************************************************************
//  Class: MeshTableOfContents
//
//  Purpose:
//      Reads in the meshtvprep object based SILO format for a state.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

class SILOOBJ_VTK_API MeshTableOfContents : public TableOfContents
{
  public:
                                MeshTableOfContents();
    virtual                    ~MeshTableOfContents();

    void                        GetZones(const char *, const int *, int, 
                                         vtkUnstructuredGrid **);

    virtual Field_VTK          *GetField(const char *)        { return NULL; };
    virtual IntervalTree_VTK   *GetIntervalTree(const char *) { return NULL; };
    virtual Value_VTK          *GetValue(const char *);

    virtual void                UpdateReferences(void);

  protected:
    StructuredTopology         *structuredTopology;
    int                         structuredTopologyN;
    UnstructuredTopology_VTK   *unstructuredTopology;
    int                         unstructuredTopologyN;
    Value_VTK                 **values;
    int                         valuesN;

    // Protected Methods
    virtual void                ReadFile(const char *);
};


#endif


