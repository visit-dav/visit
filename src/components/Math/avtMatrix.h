#ifndef MATRIX_H
#define MATRIX_H
#include <math_exports.h>
#include <visitstream.h>

class avtVector;

// ****************************************************************************
//  Class:  avtMatrix
//
//  Purpose:
//    Encapsulation of a 4x4 matrix.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Oct 16 16:12:32 PST 2001
//    Got rid of vtkMatrix4x4.
//
//    Kathleen Bonnell, Tue Mar  4 09:54:46 PST 2003 
//    Added methods to convert between frames. 
//
//    Eric Brugger, Tue Feb 10 08:28:05 PST 2004
//    Added the optional argument lhs (left handed coordinate system) to
//    MakeTrackball and CreateTrackball.
//
//    Hank Childs, Mon Nov  1 11:17:25 PST 2004
//    I added [Make|Create]Rotation, which were originally coded by Jeremy
//    Meredith for me to help with shadows (and I am checking in).
//
// ****************************************************************************

class MATH_API avtMatrix
{
  public:
    avtMatrix();
    avtMatrix(const avtMatrix&);
    avtMatrix(const double *);
    ~avtMatrix();

    // index operators
    double       *operator[](int i);
    const double *operator[](int i) const;

    // assignment operator
    void   operator=(const avtMatrix&);

    // multiply matrix*matrix
    avtMatrix operator*(const avtMatrix&) const;
    // transform point
    avtVector operator*(const avtVector&) const;
    // transform vector
    avtVector operator^(const avtVector&) const;

    void   Inverse();
    void   Transpose();

    // utility
    void   MakeIdentity();
    void   MakeZero();
    void   MakeTrackball(double,double, double,double, bool lhs=false);
    void   MakeTranslate(double, double, double);
    void   MakeTranslate(const avtVector&);
    void   MakeRBT(const avtVector&, const avtVector&, const avtVector&);
    void   MakeRotation(const avtVector&, const avtVector&, const avtVector&);
    void   MakeScale(double,double,double);
    void   MakeScale(double);
    void   MakePerspectiveProjection(double,double, double, double);
    void   MakeOrthographicProjection(double, double,double, double);
    void   MakeView(const avtVector&, const avtVector&, const avtVector&);

    void   MakeFrameToFrameConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &, 
               const avtVector &, const avtVector&, const avtVector&, const avtVector &);
    void   MakeCartesianToFrameConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &);
    void   MakeFrameToCartesianConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &);
    // static utility
    static avtMatrix CreateIdentity();
    static avtMatrix CreateZero();
    static avtMatrix CreateTrackball(double,double, double,double, bool lhs=false);
    static avtMatrix CreateTranslate(double, double, double);
    static avtMatrix CreateTranslate(const avtVector&);
    static avtMatrix CreateRBT(const avtVector&, const avtVector&, const avtVector&);
    static avtMatrix CreateRotation(const avtVector&, const avtVector&, const avtVector&);
    static avtMatrix CreateScale(double,double,double);
    static avtMatrix CreateScale(double);
    static avtMatrix CreatePerspectiveProjection(double,double, double, double);
    static avtMatrix CreateOrthographicProjection(double, double,double, double);
    static avtMatrix CreateView(const avtVector&, const avtVector&, const avtVector&);
    static avtMatrix CreateFrameToFrameConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &, 
               const avtVector &, const avtVector&, const avtVector&, const avtVector &);
    static avtMatrix CreateCartesianToFrameConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &);
    static avtMatrix CreateFrameToCartesianConversion(
               const avtVector &, const avtVector&, const avtVector&, const avtVector &);

    const double *GetElements() const;

    // friends
    friend ostream& operator<<(ostream&,const avtMatrix&);
  private:
    double m[4][4];
};

inline double *
avtMatrix::operator[](int i)
{
    return &(m[i][0]);
}

const inline double *
avtMatrix::operator[](int i) const
{
    return &(m[i][0]);
}

const inline double *
avtMatrix::GetElements() const
{
    return (const double *)m;
}

#endif
