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
//                          avtImageRepresentation.h                         //
// ************************************************************************* //

#ifndef AVT_IMAGE_REPRESENTATION_H
#define AVT_IMAGE_REPRESENTATION_H
#include <pipeline_exports.h>


class  vtkImageData;


// ****************************************************************************
//  Class: avtImageRepresentation
//
//  Purpose:
//      Buffers how an image is being stored - as a vtkImageData or as a
//      character string.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jan 25 23:36:25 PST 2001
//    Added NewImage.
//
//    Hank Childs, Tue Feb 13 13:42:06 PST 2001
//    Added concept of z-buffer.
//
//    Mark C. Miller, 25Feb03
//    Added concept of an origin (a row/col offset of this avtImageRep...
//    within some other avtImageRep...). Although VTK's vtkImageData object
//    supports the notion of an origin, I have no idea how that feature is
//    used by VTK. Since I don't plan to use this feature of a vtkImageData
//    object, I have captured knowledge of row/col offset here in AVT.
//
//    Mark C. Miller, Mon Oct 31 18:12:49 PST 2005
//    Added code to support compression of data object string
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added additional compression related data members 
//
//    Hank Childs, Mon Feb  6 14:59:43 PST 2006
//    Allow image to avoid copying the Z-buffer.
//
//    Hank Childs, Tue Dec 18 10:25:09 PST 2007
//    Remove const return type of assignment operator, to prevent the compiler
//    from defining a second assignment operator for the non-const case.
//
// ****************************************************************************

class PIPELINE_API avtImageRepresentation
{
  public:
                         avtImageRepresentation();
                         avtImageRepresentation(vtkImageData *);
                         avtImageRepresentation(vtkImageData *, float *, bool = false);
                         avtImageRepresentation(char *, int);
                         avtImageRepresentation(const avtImageRepresentation &);
    virtual             ~avtImageRepresentation();

    avtImageRepresentation    &operator=(const avtImageRepresentation &);

    void                 GetSize(int *rowSize, int *colSize) const;

    // these aren't const, because they might call GetImageFromString
    vtkImageData        *GetImageVTK(void);
    float               *GetZBuffer(void);
    unsigned char       *GetRGBBuffer(void);
    unsigned char       *GetImageString(int &);
    unsigned char       *GetCompressedImageString(int &);
    void                 GetSize(int *rowSize, int *colSize);

    void                 SetOrigin(const int rowOrigin, const int colOrigin);
    void                 GetOrigin(int *rowOrigin, int *colOrigin) const;
    virtual int          GetNumberOfCells(bool polysOnly = false) const; 

    float                GetCompressionRatio() const;
    float                GetTimeToCompress() const;
    float                GetTimeToDecompress() const;

    bool                 Valid(void);
    void                 ReleaseData(void);

    static vtkImageData *NewImage(int, int);

  protected:
    vtkImageData        *asVTK;
    float               *zbuffer;
    int                 *zbufferRef;
    unsigned char       *asChar;
    int                  asCharLength;
    int                 *asCharRef;

    int                  rowOrigin;
    int                  colOrigin;

    float                compressionRatio;
    float                timeToCompress;
    float                timeToDecompress;

    void                 Copy(const avtImageRepresentation &);
    void                 DestructSelf(void);
    void                 Initialize(void);

  private:
    void                 GetImageFromString(unsigned char *, int,
                            vtkImageData *&, float *&);
    unsigned char       *GetImageString(int &, bool);
};

inline
void avtImageRepresentation::SetOrigin(const int _rowOrigin,
                                       const int _colOrigin)
{
   rowOrigin = _rowOrigin;
   colOrigin = _colOrigin;
}

inline
void avtImageRepresentation::GetOrigin(int *_rowOrigin, int *_colOrigin) const
{
   *_rowOrigin = rowOrigin;
   *_colOrigin = colOrigin;
}

#endif

