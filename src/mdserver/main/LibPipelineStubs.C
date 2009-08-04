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

#include <LibPipelineStubs.h>
#include <StubReferencedException.h>

#define DEFSTUB(retArg, clName, methName, methArgs, retVal)    \
retArg clName::methName  methArgs                              \
{                                                              \
    EXCEPTION1(StubReferencedException, #clName #methName);    \
    return retVal;                                             \
}

DEFSTUB(void, vtkAppendFilter, AddInput, (vtkDataSet*), /*void*/);
DEFSTUB(void*, vtkAppendFilter, New, (), 0);

DEFSTUB(void, vtkAppendPolyData,AddInput, (vtkPolyData*), /*void*/);
DEFSTUB(void*, vtkAppendPolyData, New, (), 0);

DEFSTUB(void*, vtkCleanPolyData, New, (), 0);

DEFSTUB(void*, vtkDataSetReader, GetOutput, (), 0);
DEFSTUB(void*, vtkDataSetReader, New, (), 0);

DEFSTUB(void, vtkDataSetWriter, SetInput, (vtkDataSet*), /*void*/);
DEFSTUB(void*, vtkDataSetWriter, New, (), 0);

DEFSTUB(char*, vtkDataWriter, RegisterAndGetOutputString, (), 0);

DEFSTUB(void, vtkWriter, SetInput, (vtkDataObject*), /*void*/);

DEFSTUB(void*, vtkPolyDataReader, GetOutput, (), 0);
DEFSTUB(void*, vtkPolyDataReader, New, (), 0);

DEFSTUB(void*, vtkRectilinearGridReader, GetOutput, (), 0);          
DEFSTUB(void*, vtkRectilinearGridReader, New, (), 0);          

DEFSTUB(void*, vtkStructuredGridReader, GetOutput, (), 0);          
DEFSTUB(void*, vtkStructuredGridReader, New, (), 0);          

DEFSTUB(void*, vtkStructuredPointsReader, GetOutput, (), 0);          
DEFSTUB(void*, vtkStructuredPointsReader, New, (), 0);          

DEFSTUB(void, vtkStructuredPointsWriter, SetInput, (vtkImageData*), /*void*/);
DEFSTUB(void*, vtkStructuredPointsWriter, New, (), 0);          

DEFSTUB(void*, vtkUnstructuredGridReader, GetOutput, (), 0);          
DEFSTUB(void*, vtkUnstructuredGridReader, New, (), 0);          

DEFSTUB(void, vtkCSGGrid, DiscretizeSpace, (int,int,int,double,double,double,double,double,double,double), /*void*/);
DEFSTUB(void, vtkCSGGrid, DiscretizeSpace2, (int,int,int,double,double,double,double,double,double,double), /*void*/);
DEFSTUB(void, vtkCSGGrid, DiscretizeSpace3, (int,int,int,double,double,double,double,double,double,double,double), /*void*/);
DEFSTUB(void, vtkCSGGrid, DiscretizeSurfaces, (int,double,double,double,double,double,double,double), /*void*/);
DEFSTUB(void, vtkCSGGrid, DiscretizeSpace, (int,double,double,double,double,double,double,double), /*void*/);
DEFSTUB(bool, vtkCSGGrid, operator==, (const vtkCSGGrid&) const, false);
DEFSTUB(void*, vtkCSGGrid, New, (), 0);
