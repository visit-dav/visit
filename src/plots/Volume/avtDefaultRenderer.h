/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                              avtDefaultRenderer.h                         //
// ************************************************************************* //

#ifndef AVT_DEFAULT_RENDERER_H
#define AVT_DEFAULT_RENDERER_H

#include <avtVolumeRendererImplementation.h>
#include <VolumeAttributes.h>

#include <vtkSmartPointer.h>
#include <vtkVolume.h>
#include <vtkSmartVolumeMapper.h>

// ****************************************************************************
//  Class: avtDefaultRenderer
//
//  Purpose:
//      A default volume renderer using vtkSmartVolumeMapper. The mapper checks
//      for hardware compatability and rendering parameters to choose which 
//      rendering method to use. 
//
//  Programmer: Alister Maguire
//  Creation:   April  3, 2017
//
//  Modifications:
//
// ****************************************************************************

class avtDefaultRenderer : public avtVolumeRendererImplementation
{
  public:
                            avtDefaultRenderer();
    virtual                ~avtDefaultRenderer();

  protected:
    virtual void            Render(const RenderProperties &props,
                                   const VolumeData &volume);

    vtkSmartPointer<vtkVolume>            curVolume;
    vtkSmartPointer<vtkImageData>         imageToRender;
    vtkSmartPointer<vtkVolumeProperty>    volumeProp;
    vtkSmartPointer<vtkSmartVolumeMapper> mapper;

    VolumeAttributes                      oldAtts;

    bool                                  resetColorMap;
};

#endif 
