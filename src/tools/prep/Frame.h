// ************************************************************************* //
//                                 Frame.h                                   //
// ************************************************************************* //

#ifndef FRAME_H
#define FRAME_H


// ****************************************************************************
//  Class: Frame
//
//  Purpose:
//      A Frame (three basis vectors and a point) that has routines to
//      translate a point in Cartesian coordinates to coordinates in its
//      Frame.
//
//  Programmer: Hank Childs
//  Creation:   June 25, 2000
//
// ****************************************************************************

class Frame
{
  public:
                    Frame();

    void            GetCoordsInFrame(const float[3], float &, float &, float&);
    void            SetFrame(const float [3], const float [3], const float [3],
                             const float [3]);

  private:
    float           b0[3], b1[3], b2[3];
    float           origin[3];
    float           e[4][4];

    void            CalculateTransformationMatrix(void);
};


#endif


