// ----------------------------------------------------------------------------
// File:  Transforms.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#ifndef TRANSFORMS_H
#define TRANSFORMS_H

struct HexTransform
{
    int  n[8];
    char e[12];
    bool f;
};

struct WedgeTransform
{
    int  n[6];
    char e[9];
    bool f;
};

struct PyramidTransform
{
    int  n[5];
    char e[8];
    bool f;
};

struct TetTransform
{
    int  n[4];
    char e[6];
    bool f;
};


extern HexTransform     hexTransforms[48];
extern WedgeTransform   wedgeTransforms[12];
extern PyramidTransform pyramidTransforms[8];
extern TetTransform     tetTransforms[24];

#endif
