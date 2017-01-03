/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#ifndef VTK_MULTI_SPLITTER_H
#define VTK_MULTI_SPLITTER_H
#include <visit_vtk_exports.h>

#include "vtkCSGFixedLengthBitField.h"
#include "vtkUnstructuredGridAlgorithm.h"

#include <vector>

class vtkImplicitFunction;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class:  vtkMultiSplitter
//
//  Purpose:
//    Splits a rectilinear dataset using multiple implicit functions,
//    tagging output cells as it splits, and later allowing extraction
//    of various region sets as whole data sets.
//
//  Note: Copied largely from vtkVisItSplitter
//
//  Programmer:  Eric Brugger
//  Creation:    July 23, 2012
//
//  Modifications:
//    Eric Brugger, Thu Apr  3 08:20:06 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
// ****************************************************************************

class VISIT_VTK_API vtkMultiSplitter :
  public vtkUnstructuredGridAlgorithm
{
  public:
    vtkTypeMacro(vtkMultiSplitter,vtkUnstructuredGridAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);

    static vtkMultiSplitter *New();

    virtual void SetTagBitField(std::vector<vtkCSGFixedLengthBitField> *);
    virtual void SetClipFunctions(double *, int);

  protected:
    vtkMultiSplitter();
    ~vtkMultiSplitter();

    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *);
    virtual int FillInputPortInformation(int port, vtkInformation *info);

  private:
    double *bounds;
    int    nBounds;
    std::vector<vtkCSGFixedLengthBitField> *newTags;

    vtkMultiSplitter(const vtkMultiSplitter&);  // Not implemented.
    void operator=(const vtkMultiSplitter&);    // Not implemented.
};

#endif
