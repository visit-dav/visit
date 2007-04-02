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

    const avtImageRepresentation    &operator=(const avtImageRepresentation &);

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

