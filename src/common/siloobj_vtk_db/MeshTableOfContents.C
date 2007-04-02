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
//                            MeshTableOfContents.C                          //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <SiloObjLib.h>
#include <MeshTableOfContents.h>


// ****************************************************************************
//  Method: MeshTableOfContents constructor
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

MeshTableOfContents::MeshTableOfContents()
{
    structuredTopology    = NULL;
    structuredTopologyN   = 0;
    unstructuredTopology  = NULL;
    unstructuredTopologyN = 0;
    values                = NULL;
    valuesN               = 0;
}


// ****************************************************************************
//  Method: MeshTableOfContents destructor
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

MeshTableOfContents::~MeshTableOfContents()
{
    if (structuredTopology != NULL)
    {
        delete [] structuredTopology;
    }
    if (unstructuredTopology != NULL)
    {
        delete [] unstructuredTopology;
    }
    if (values != NULL)
    {
        for (int i = 0 ; i < valuesN ; i++)
        {
            if (values[i] != NULL)
            {
                delete values[i];
            }
        }
        delete [] values;
    }
}


// ****************************************************************************
//  Method: MeshTableOfContents::ReadFile
//
//  Purpose:
//      Reads in the DBobjects from a mesh file.
//
//  Arguments:
//      filename    The name of the file to read.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
MeshTableOfContents::ReadFile(const char *filename)
{
    DBfile *dbfile = GetSILOFile(filename);
    DBtoc  *toc    = DBGetToc(dbfile);

    structuredTopology    = new StructuredTopology[toc->nobj];
    unstructuredTopology  = new UnstructuredTopology_VTK[toc->nobj];
    values                = new Value_VTK*[toc->nobj];
    

    for (int i = 0 ; i < toc->nobj ; i++)
    {
        DBobject  *siloObj = DBGetObject(dbfile, toc->obj_names[i]);

        if (strcmp(siloObj->type, StructuredTopology::SILO_TYPE) == 0)
        {
            structuredTopology[structuredTopologyN].Read(siloObj, dbfile);
            structuredTopologyN++;
        }
        else if (strcmp(siloObj->type, UnstructuredTopology::SILO_TYPE) == 0)
        {
            unstructuredTopology[unstructuredTopologyN].Read(siloObj, dbfile);
            unstructuredTopologyN++;
        }
        else if (strcmp(siloObj->type, Value::SILO_TYPE) == 0)
        {
            values[valuesN] = new Value_VTK;
            values[valuesN]->Read(siloObj, dbfile);
            valuesN++;
        }
 
        DBFreeObject(siloObj);
    }

    //
    // Do not need to close SILO file, since TOC will handle it.
    //
}


// ****************************************************************************
//  Method: MeshTableOfContents::UpdateReferences
//
//  Purpose:
//      Give each object a reference to the TOC (this) so that they can
//      update the pointers to SILO objects they have as data members.  They
//      also need a reference to this so that they can request files.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
MeshTableOfContents::UpdateReferences(void)
{
    int  i = 0;

    for (i = 0 ; i < valuesN ; i++)
    {
        values[i]->UpdateReferences(this);
    }
    for (i = 0 ; i < unstructuredTopologyN ; i++)
    { 
        unstructuredTopology[i].UpdateReferences(this);
    }
}


// ****************************************************************************
//  Method: MeshTableOfContents::GetValue
//   
//  Purpose:
//      Goes through the value objects and returns the one with the correct 
//      name.  Meant for contained objects when UpdateReferences is called.
//
//  Arguments:
//      name    The name of the value object to get.
//
//  Returns:    A pointer to the Value_VTK object, NULL if it can not
//              found.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

Value_VTK *
MeshTableOfContents::GetValue(const char *name)
{
    Value_VTK *rv = NULL;
    int i;
    for (i = 0 ; i < valuesN ; i++)
    {
        if (values[i] == NULL)
        {
            //
            // This is a valid case because we invalidate the entry when it is
            // returned.
            //
            continue;
        }
        if (strcmp(name, values[i]->GetName()) == 0)
        {
            rv = values[i];
            values[i] = NULL;
            break;
        }
    }

    //
    // Take the interval tree out of the list and guarantee that there are not
    // any NULL entries lying in wait.
    //
    for (int j = i ; j < valuesN ; j++)
    {
        values[j] = values[j+1];
    }
    if (i < valuesN)
    {
        valuesN--;
    }

    return rv;
}


// ****************************************************************************
//  Method: MeshTableOfContents::GetZones
//
//  Purpose:
//      Gets the coordinates from the current mesh object.
//
//  Arguments:
//      meshname   The name of the mesh.
//      list       The domains of interest.
//      listN      The number of domains in list.
//      grid       The grid to put the connectivity information into.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
MeshTableOfContents::GetZones(const char *meshname, const int *list, int listN,
                              vtkUnstructuredGrid **grid)
{
    char s[LONG_STRING];

    int i;
    for (i = 0 ; i < unstructuredTopologyN ; i++)
    {
        sprintf(s, "%s%s", meshname, UnstructuredTopology::NAME);
        if (strcmp(unstructuredTopology[i].GetName(), s) == 0)
        {
            break;
        }
    }

    if (i >= unstructuredTopologyN)
    {
        cerr << "Could not resolve topology from " << meshname << "." << endl;
        // throw
        return;
    }

    unstructuredTopology[i].GetZones(list, listN, grid);
}
     

