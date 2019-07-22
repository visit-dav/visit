// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef IMAGE_OBJECT_H
#define IMAGE_OBJECT_H

// ****************************************************************************
// Class: ImageObject
//
// Purpose:
//   Class for reading and writing image files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 21 17:39:16 PST 2006
//
// Modifications:
//    Kathleen Biagas, Mon Jan 12 16:36:30 PST 2015
//    Add optional 'format' argument to 'Write' method.
//
// ****************************************************************************

class ImageObject
{
public:
    ImageObject();
    ImageObject(const ImageObject &);
    ImageObject(int width, int height);
    ImageObject(int width, int height, int depth);
    ~ImageObject();

    unsigned char *Pixel(int x, int y)
    {
        return pixels + ((y * w) + x) * depth;
    }

    const unsigned char *Pixel(int x, int y) const
    {
        return pixels + ((y * w) + x) * depth;
    }

    int Width() const
    { 
        return w;
    }

    int Height() const
    { 
        return h;
    }

    int Depth() const
    { 
        return depth;
    }

    bool Write(const char *filename, const char *format= 0);
    bool Read(const char *filename);

    void SetAllPixelsToColor(const int *rgb);
    void SetBlockToColor(int x0, int y0, int x1, int y1, const int *rgb);
    void Copy(const ImageObject &);
    void CopyBlock(const ImageObject &, int, int, int, int, int, int);
private:
    int w, h, depth;
    unsigned char *pixels;
};

#endif
