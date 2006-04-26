// ************************************************************************* //
//                      avtMesaLabelRenderer.h                               //
// ************************************************************************* //

#ifndef AVT_MESA_LABEL_RENDERER_H
#define AVT_MESA_LABEL_RENDERER_H
#include <avtLabelRenderer.h>

// ****************************************************************************
// Class: avtMesaLabelRenderer
//
// Purpose:
//   Renders the labels using Mesa.
//
// Notes:      The contents of this file are essentially the same as
//             avtOpenGLLabelRenderer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:46:36 PDT 2004
//
// Modifications:
//   Brad Whitlock, Thu Aug 4 10:35:24 PDT 2005
//   Added some new helper methods.
//
//   Brad Whitlock, Tue Apr 25 10:15:48 PDT 2006
//   Moved some point transformation code from the base class to here.
//
// ****************************************************************************

class avtMesaLabelRenderer : public avtLabelRenderer
{
public:
                            avtMesaLabelRenderer();
    virtual                ~avtMesaLabelRenderer();
    virtual void            ReleaseGraphicsResources();

protected:
    void SetupGraphicsLibrary();
    void RenderLabels();
    void DrawLabel(const double *vert, const char *label);
    void DrawLabel2(const double *vert, const char *label);
    void ClearCharacterDisplayLists();

    void DrawLabels2D();
    void DrawAllLabels2D(bool, bool);
    void DrawDynamicallySelectedLabels2D(bool, bool);

    void DrawLabels3D();
    void DrawAllCellLabels3D();
    void DrawAllNodeLabels3D();

    void PopulateBinsWithCellLabels3D();
    void PopulateBinsWithNodeLabels3D();
    void PopulateBinsHelper(const unsigned char *, const char *, const float *,
                            int);
    void ClearZBuffer();
    void InitializeZBuffer(bool, bool);

    void SetColor(int);
    void BeginSize2D(int);
    void EndSize2D();

    float *TransformPoints(const float *inputPoints,
                           const unsigned char *quantizedNormalIndices,
                           int nPoints);

    float                  x_scale;
    float                  y_scale;
    int                    characterDisplayListIndices[256];
    bool                   characterDisplayListsCreated;

    double                 pointXForm[4][4];

    int                    zBufferMode;
    float                 *zBuffer;
    int                    zBufferWidth;
    int                    zBufferHeight;
    float                  zTolerance;
};

#endif
