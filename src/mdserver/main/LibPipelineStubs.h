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

#ifndef LIB_PIPELINE_STUBS_H
#define LIB_PIPELINE_STUBS_H 

// ****************************************************************************
// LibPipelineStubs 
//
// Purpose: Define references for all the symbols from libpipeline that are
// needed to link the mdserver but which are never referenced in the mdserver's
// normal execution
//
// Programmer: Mark C. Miller 
// Creation:   October 25, 2005 
//
// Modifications:
//   Kathleen Bonnell, Wed May 17 10:58:54 PDT 2006
//   Added stub for vtkWriter::SetInput(vtkDataObject*).
//
// ****************************************************************************

class vtkPolyData;
class vtkDataSet;
class vtkImageData;
class vtkDataObject;

class vtkAppendFilter {
  public:
    void  AddInput(vtkDataSet*);
    void *New();
};

class vtkAppendPolyData {
  public:
    void  AddInput(vtkPolyData*);
    void *New();
};

class vtkCleanPolyData {
  public:
    void *New();
};

class vtkDataSetReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkDataSetWriter {
  public:
    void  SetInput(vtkDataSet*);
    void *New();
};

class vtkDataWriter {
  public:
    char *RegisterAndGetOutputString();
};

class vtkWriter {
  public:
    void  SetInput(vtkDataObject*);
};

class vtkPolyDataReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkRectilinearGridReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkStructuredGridReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkStructuredPointsReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkStructuredPointsWriter {
  public:
    void SetInput(vtkImageData*);
    void *New();
};

class vtkUnstructuredGridReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkCSGGrid {
  public:
    void DiscretizeSpace(int,int,int,double,double,double,double,double,double,double);
    void DiscretizeSurfaces(int, double, double, double, double, double, double, double);
    void DiscretizeSpace(int, double, double, double, double, double, double, double);
};
#endif
