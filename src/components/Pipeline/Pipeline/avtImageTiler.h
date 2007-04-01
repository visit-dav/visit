#ifndef AVT_IMAGE_TILER_H
#define AVT_IMAGE_TILER_H
#include <avtImage.h>

// ****************************************************************************
// Class: avtImageTiler
//
// Purpose:
//   Creates a new tiled image out of several avtImage objects and destroys
//   the input images.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 15 15:18:17 PST 2004
//
// Modifications:
//   
// ****************************************************************************

class PIPELINE_API avtImageTiler
{
public:
    avtImageTiler(int nImg);
    virtual ~avtImageTiler();

    void AddImage(avtImage_p img);
    int GetNumberOfColumnsForNTiles(int nTiles) const;
    int GetNumberOfRowsForNTiles(int nTiles) const;

    avtImage_p CreateTiledImage();
private:
    avtImage_p *images;
    int        nImages;
    int        maxImages;
};

#endif
