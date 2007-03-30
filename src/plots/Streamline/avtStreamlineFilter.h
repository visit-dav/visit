// ************************************************************************* //
//                              avtStreamlineFilter.h                        //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_FILTER_H
#define AVT_STREAMLINE_FILTER_H
#include <avtStreamer.h>

class vtkVisItStreamLine;
class vtkTubeFilter;
class vtkPolyData;
class vtkAppendPolyData;

#define STREAMLINE_SOURCE_POINT      0
#define STREAMLINE_SOURCE_LINE       1
#define STREAMLINE_SOURCE_PLANE      2
#define STREAMLINE_SOURCE_SPHERE     3
#define STREAMLINE_SOURCE_BOX        4

// ****************************************************************************
// Class: avtStreamlineFilter
//
// Purpose:
//    This operator is the implied operator associated with a Streamline plot.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 4 15:32:05 PST 2002
//
// Modifications:
//
// ****************************************************************************

class avtStreamlineFilter : public avtStreamer
{
  public:
                              avtStreamlineFilter();
    virtual                  ~avtStreamlineFilter();

    virtual const char       *GetType(void)   { return "avtStreamlineFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating streamlines"; };

    virtual void              ReleaseData(void);
//    bool                      Equivalent(bool, int);

    // Methods to set the filter's attributes.
    void                      SetSourceType(int sourceType);
    void                      SetStepLength(double len);
    void                      SetMaxTime(double mt);

    void                      SetPointSource(double pt[3]);
    void                      SetLineSource(double pt[3], double pt2[3]);
    void                      SetPlaneSource(double O[3], double N[3],
                                             double U[3], double R);
    void                      SetSphereSource(double O[3], double R);
    void                      SetBoxSource(double E[6]);

    void                      SetShowTube(bool);
    void                      SetShowStart(bool);
    void                      SetTubeRadius(double rad);
    void                      SetPointDensity(int den);

  protected:
    int    sourceType;   
    double stepLength;
    double maxTime;
    double tubeRadius;
    bool   showTube;
    bool   showStart;
    int    pointDensity;

    // Various starting locations for streamlines.
    double pointSource[3];
    double lineStart[3], lineEnd[3];
    double planeOrigin[3], planeNormal[3], planeUpAxis[3], planeRadius;
    double sphereOrigin[3], sphereRadius;
    double boxExtents[6];

    // Internal filters.
    vtkVisItStreamLine       *streamline;
    vtkTubeFilter            *tubes;

    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);
    virtual void              RefashionDataObjectInfo(void);

    vtkPolyData              *AddStartSphere(vtkPolyData *, float val,
                                             float pt[3]);
};


#endif
