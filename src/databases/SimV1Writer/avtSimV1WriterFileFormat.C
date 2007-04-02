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
//                            avtSimV1WriterFileFormat.C                     //
// ************************************************************************* //

#include <avtSimV1WriterFileFormat.h>

#include <DebugStream.h>
#include <ImproperUseException.h>



// ****************************************************************************
//  Method: avtSimV1 constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 14:15:32 PST 2006
//
//  Modifications:
//
// ****************************************************************************

avtSimV1WriterFileFormat::avtSimV1WriterFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
}

// ****************************************************************************
//  Method: avtSimV1WriterFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 14:15:40 PST 2006
//
// ****************************************************************************

void
avtSimV1WriterFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtSimV1WriterFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Fill the simulation metadata with the parameters from the file for
//      the mdserver.  Get the info from the simulation for the engine.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 14:13:46 PST 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtSimV1WriterFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *)
{
}


// ****************************************************************************
//  Method: avtSimV1WriterFileFormat::GetMesh
//
//  Purpose:
//      For the mdserver, do nothing.
//      For the engine, return a C-api mesh converted to a vtkDataSet.
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 14:13:07 PST 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtSimV1WriterFileFormat::GetMesh(int domain, const char *meshname)
{
    EXCEPTION1(ImproperUseException, "avtSimV1WriterFileFormat::GetMesh: is not to be called.");
    return NULL;
}


// ****************************************************************************
//  Method: avtSimV1WriterFileFormat::GetVar
//
//  Purpose:
//      For the mdserver, do nothing.
//      For the engine, return a scalar converted to a VTK array.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 14:12:37 PST 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtSimV1WriterFileFormat::GetVar(int domain, const char *varname)
{
    EXCEPTION1(ImproperUseException, "avtSimV1WriterFileFormat::GetVar: is not to be called.");
    return NULL;
}


// ****************************************************************************
//  Method: avtSimV1WriterFileFormat::GetVectorVar
//
//  Purpose:
//      For the mdserver, do nothing.
//      For the engine, also do nothing right now.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 14:12:06 PST 2006
//
// ****************************************************************************

vtkDataArray *
avtSimV1WriterFileFormat::GetVectorVar(int domain, const char *varname)
{
    EXCEPTION1(ImproperUseException, "avtSimV1WriterFileFormat::GetVectorVar: is not to be called.");
    return NULL;
}

// ****************************************************************************
//  Method:  avtSimV1WriterFileFormat::PopulateIOInformation
//
//  Purpose:
//    Populate the list of acceptable domains for this processor.
//
//  Arguments:
//    ioinfo     the avtIOInformation containing the output domain list
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Nov 9 14:10:45 PST 2006
//
// ****************************************************************************
void
avtSimV1WriterFileFormat::PopulateIOInformation(avtIOInformation& ioInfo)
{
    EXCEPTION1(ImproperUseException, "avtSimV1WriterFileFormat::PopulateIOInformation: is not to be called.");
}
