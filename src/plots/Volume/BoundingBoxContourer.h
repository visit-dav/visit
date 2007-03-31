#ifndef BOUNDING_BOX_CONTOURER_H
#define BOUNDING_BOX_CONTOURER_H

// ****************************************************************************
//  Class:  BoundingBoxContourer
//
//  Purpose:
//    Creates OpenGL polygon slices through a bounding box.
//    Currently used for view-aligned slice planes of a 3D texturing
//    volume renderer.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
// ****************************************************************************
class BoundingBoxContourer
{
  public:
    float x[8];
    float y[8];
    float z[8];
    float v[8];
    float r[8];
    float s[8];
    float t[8];
    void   ContourTriangles(float value);

  private:
    struct Coord
    {
        int   edge;     // 1-12 edge index
        int   vertex0;  // 0-7 vertex index
        int   vertex1;  // 0-7 vertex index
        float alpha;    // 0.0 - 1.0 interpolation parameter

        float x,y,z;    // physical x,y,z coordinate
        float r,s,t;    // texture coordinates
    };

    int    CalculateCase(float value);
    Coord  GetEdgeIsoCoord(int edge, float value);
};

#endif
