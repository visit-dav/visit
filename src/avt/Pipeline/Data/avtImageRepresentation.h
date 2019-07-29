// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtImageRepresentation.h                         //
// ************************************************************************* //

#ifndef AVT_IMAGE_REPRESENTATION_H
#define AVT_IMAGE_REPRESENTATION_H
#include <pipeline_exports.h>


class vtkImageData;
class vtkFloatArray;
class vtkUnsignedCharArray;


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
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer. this greatly
//    simplifies zero copy data transfer as the ref-count and data
//    are contained in a single object. Add a method to set rgb and z-buffer
//    Add support for RGBA images. Add a method to get internals
//    as VTK arrays to facilitate zero-copy data transfers.
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to long long
//
//    Burlen Loring, Wed Sep 23 15:29:14 PDT 2015
//    Added an geters for image and z buffer. these bypass marshall
//    from string. the marshal code throws if the image and string
//    are both null. this api is for cases when we know we don't need
//    to worry about marshalling.
//
// ****************************************************************************

class PIPELINE_API avtImageRepresentation
{
  public:
                         avtImageRepresentation();
                         avtImageRepresentation(int w, int h, int chan = 3);
                         avtImageRepresentation(vtkImageData *);
                         avtImageRepresentation(vtkImageData *, vtkFloatArray *);
                         avtImageRepresentation(vtkImageData *, float *, bool = false);
                         avtImageRepresentation(char *, int);
                         avtImageRepresentation(const avtImageRepresentation &);
    virtual             ~avtImageRepresentation();

    avtImageRepresentation    &operator=(const avtImageRepresentation &);

    void                 GetSize(int *rowSize, int *colSize) const;

    vtkImageData        *GetImageVTKDirect(){ return asVTK; }
    vtkFloatArray       *GetZBufferVTKDirect(){ return zbuffer; }

    // these aren't const, because they might call GetImageFromString
    void                 SetImageVTK(vtkImageData *src);
    vtkImageData        *GetImageVTK();
    void                 SetZBufferVTK(vtkFloatArray *z);
    vtkFloatArray       *GetZBufferVTK();
    float               *GetZBuffer();
    int                  GetNumberOfColorChannels();
    unsigned char       *GetRGBBuffer(void);
    vtkUnsignedCharArray *GetRGBBufferVTK();

    unsigned char       *GetImageString(int &);
    unsigned char       *GetCompressedImageString(int &);
    void                 GetSize(int *rowSize, int *colSize);

    void                 SetOrigin(const int rowOrigin, const int colOrigin);
    void                 GetOrigin(int *rowOrigin, int *colOrigin) const;
    virtual long long    GetNumberOfCells(bool polysOnly = false) const;

    float                GetCompressionRatio() const;
    float                GetTimeToCompress() const;
    float                GetTimeToDecompress() const;

    bool                 Valid(void);
    void                 ReleaseData(void);

    static vtkImageData *NewImage(int w, int h, int chan = 3);
    static vtkImageData *NewValueImage(int w, int h);

  protected:
    vtkImageData        *asVTK;
    vtkFloatArray       *zbuffer;
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
    unsigned char       *GetImageString(int &, bool);

    void                 GetImageFromString(unsigned char *str,
                            int strLength, vtkImageData *&img,
                            vtkFloatArray *&zbuf);
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
