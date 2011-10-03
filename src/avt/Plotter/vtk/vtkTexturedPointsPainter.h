/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
#ifndef __vtkTexturedPointsPainter_h
#define __vtkTexturedPointsPainter_h
#include <plotter_exports.h>

#include <vtkPointsPainter.h>

// ****************************************************************************
// Class: vtkTexturedPointsPainter
//
// Purpose:
//   Subclass of vtkPointsPainter that can do point sprite texturing.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 24 12:53:32 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

class PLOTTER_API vtkTexturedPointsPainter : public vtkPointsPainter
{
public:
  static vtkTexturedPointsPainter* New();
  vtkTypeMacro(vtkTexturedPointsPainter, vtkPointsPainter);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ReleaseGraphicsResources(vtkWindow *);

  virtual void Render(vtkRenderer *renderer, vtkActor *, unsigned long, bool);

  vtkSetMacro(DoTexturing, int);
protected:
  vtkTexturedPointsPainter();
  ~vtkTexturedPointsPainter();

  int DoTexturing;

  class Texturer;
  Texturer *tex;
private:
  vtkTexturedPointsPainter(const vtkTexturedPointsPainter&); // Not implemented.
  void operator=(const vtkTexturedPointsPainter&); // Not implemented.
};

#endif
