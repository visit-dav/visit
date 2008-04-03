#ifndef VTK_VECTOR_FONT_CHARACTER_DATA_H
#define VTK_VECTOR_FONT_CHARACTER_DATA_H

struct vtkVectorFontCharacterData
{
    const unsigned short *pts;
    const unsigned char  *tris;
    short                 npts;
    short                 ntris;
    float                 width;
    float                 bias;
    float                 scale;

    static const vtkVectorFontCharacterData *GetCharacterData(
        int charIndex, int family, bool bold, bool italic);
};

#endif
