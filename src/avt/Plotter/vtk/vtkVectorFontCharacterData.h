#ifndef VTK_VECTOR_FONT_CHARACTER_DATA_H
#define VTK_VECTOR_FONT_CHARACTER_DATA_H

//    Jeremy Meredith, Thu Aug  7 14:22:01 EDT 2008
//    Some tris values were >255, so I made it a short.
//
struct vtkVectorFontCharacterData
{
    const unsigned short *pts;
    const unsigned short *tris;
    short                 npts;
    short                 ntris;
    float                 width;
    float                 bias;
    float                 scale;

    static const vtkVectorFontCharacterData *GetCharacterData(
        int charIndex, int family, bool bold, bool italic);
};

#endif
