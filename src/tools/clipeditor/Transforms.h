// ----------------------------------------------------------------------------
// File:  Transforms.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
//
// Modifications:
//    Jeremy Meredith, Thu Sep 18 11:28:24 PDT 2003
//    Added quad and triangle transforms.
//
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

struct QuadTransform
{
    int  n[4];
    char e[4];
    bool f;
};

struct TriTransform
{
    int  n[3];
    char e[3];
    bool f;
};

extern HexTransform     hexTransforms[48];
extern WedgeTransform   wedgeTransforms[12];
extern PyramidTransform pyramidTransforms[8];
extern TetTransform     tetTransforms[24];
extern QuadTransform    quadTransforms[8];
extern TriTransform     triTransforms[6];

#endif
