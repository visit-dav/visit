// ----------------------------------------------------------------------------
// File:  Matrix.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#ifndef MATRIX_H
#define MATRIX_H

class Vector;
class ostream;

// ****************************************************************************
//  Class:  Matrix
//
//  Purpose:
//    Encapsulation of a 4x4 matrix.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
class Matrix
{
  public:
    float m[4][4];
    float openglm[16];
  public:
    Matrix();
    Matrix(const Matrix&);

    // assignment operator
    void   operator=(const Matrix&);

    // multiply matrix*matrix
    Matrix operator*(const Matrix&) const;
    // transform point
    Vector operator*(const Vector&) const;
    // transform vector
    Vector operator^(const Vector&) const;

    void   Inverse();
    void   Transpose();

    // utility
    void   CreateIdentity();
    void   CreateZero();
    void   CreateTrackball(float,float, float,float);
    void   CreateTranslate(float, float, float);
    void   CreateRBT(const Vector&, const Vector&, const Vector&);
    void   CreateScale(float,float,float);
    void   CreateScale(float);
    void   CreatePerspectiveProjection(float,float, float, float);
    void   CreateOrthographicProjection(float, float,float, float);
    void   CreateView(const Vector&, const Vector&, const Vector&);

    float* GetOpenGLMatrix();

    // friends
    friend ostream& operator<<(ostream&,const Matrix&);
};

#endif
