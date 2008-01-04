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
//                             avtImageMapper.C                              //
// ************************************************************************* //

#include <avtImageMapper.h>

#include <vtkActor2D.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkRenderer.h>

#include <avtImageDrawable.h>


// ****************************************************************************
//  Method: avtImageMapper constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

avtImageMapper::avtImageMapper()
{
    mapper = vtkImageMapper::New();
    mapper->SetColorWindow(255);
    mapper->SetColorLevel(127);
    actor  = vtkActor2D::New();
    actor->SetMapper(mapper);
    drawable = new avtImageDrawable(actor);
}


// ****************************************************************************
//  Method: avtImageMapper destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

avtImageMapper::~avtImageMapper()
{
    if (mapper != NULL)
    {
        mapper->Delete();
    }
    if (actor != NULL)
    {
        actor->Delete();
    }
}


// ****************************************************************************
//  Method: avtImageMapper::ChangedInput
//
//  Purpose:
//      This is a hook from avtImageSink to tell its derived types (us) that
//      the input has changed.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

void
avtImageMapper::ChangedInput(void)
{
    vtkImageData *img = GetImageRep().GetImageVTK();
    mapper->SetInput(img);
}


// ****************************************************************************
//  Method: avtImageMapper::Render
//
//  Purpose:
//      Renders an image to the screen.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
//  Modifications:
//      Mark C. Miller, 29Jan03
//      Due to fact that vtkActor2D now does all its relevant rendering work
//      in the RenderOverlay call, I added that call here.
//      
//  
// ****************************************************************************

void
avtImageMapper::Draw(vtkRenderer *ren)
{
    actor->RenderOverlay(ren);
    actor->RenderOpaqueGeometry(ren);
    actor->RenderTranslucentGeometry(ren);
}


