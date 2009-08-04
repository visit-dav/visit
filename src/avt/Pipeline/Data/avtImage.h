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

// ************************************************************************* //
//                                avtImage.h                                 //
// ************************************************************************* //

#ifndef AVT_IMAGE_H
#define AVT_IMAGE_H
#include <pipeline_exports.h>


#include <ref_ptr.h>

#include <avtDataObject.h>
#include <avtImageRepresentation.h>


// ****************************************************************************
//  Class: avtImage
//
//  Purpose:
//      This is another data object that avt pipelines can handle.  It is an
//      image (vtkImageData).  Further modifications may need to be made to
//      allow for multiple images as in image compositing or tiling.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Nov 21 12:08:42 PST 2001
//    Re-define DerivedCopy.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Mark C. Miller, Wed Feb  4 19:47:30 PST 2004
//    Added Instance, needed by avtDataObject->Clone
//
//    Mark C. Miller, Wed Mar 31 21:10:56 PST 2004
//    Added GetSize method
//
// ****************************************************************************

class PIPELINE_API avtImage : public avtDataObject
{
    friend class              avtImageSink;
    friend class              avtImageSource;

    // avtRayCompositer is made a friend because it is an image sink, but is
    // not declared that way to avoid multiple inheritance from sample points
    // sink and image sink.
    friend class              avtRayCompositer;

  public:
                              avtImage(avtDataObjectSource *);
    virtual                  ~avtImage();

    virtual const char       *GetType(void)  { return "avtImage"; };
    virtual int               GetNumberOfCells(bool polysOnly = false) const;
    virtual avtDataObject    *Instance(void);
    virtual avtDataObjectWriter
                             *InstantiateWriter(void);
    virtual void              ReleaseData(void);
    avtImageRepresentation   &GetImage(void);
    virtual void              GetSize(int *width, int *height) const;
    virtual float             GetCompressionRatio() const
                                  {return image.GetCompressionRatio(); };

  protected:
    avtImageRepresentation    image;

    void                      SetImage(const avtImageRepresentation &);

    virtual void              DerivedCopy(avtDataObject *);
};


typedef ref_ptr<avtImage>  avtImage_p;


#endif


