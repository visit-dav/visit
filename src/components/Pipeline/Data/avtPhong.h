// ************************************************************************* //
//                                avtPhong.h                                 //
// ************************************************************************* //

#ifndef AVT_PHONG_H
#define AVT_PHONG_H
#include <pipeline_exports.h>


#include <avtLightingModel.h>


// ****************************************************************************
//  Method: avtPhong
//
//  Purpose:
//      Performs phong shading with assumption about light position, camera
//      location, projection.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
// ****************************************************************************

class PIPELINE_API avtPhong : public avtLightingModel
{
  public:
                           avtPhong();
    virtual               ~avtPhong() {;};

    virtual double         GetShading(double, const double[3]) const;

    void                   SetAmbient(double a)   { ambient = a; };
    void                   SetDiffuse(double d)   { diffuse = d; };
    void                   SetSpecular(double s)  { specular = s; };
    void                   SetGlossiness(int g)   { glossiness = g; };
    void                   SetLightDirection(double [3]);

    virtual bool           NeedsGradients(void) { return true; };

  protected:
    double                 ambient;
    double                 diffuse;
    double                 specular;
    int                    glossiness;
    double                 lightDirection[3];
    double                 half[3];
};


#endif


