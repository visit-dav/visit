// ************************************************************************* //
//                      avtOpenGLLabelRenderer.h                             //
// ************************************************************************* //

#ifndef AVT_OPENGL_LABEL_RENDERER_H
#define AVT_OPENGL_LABEL_RENDERER_H
#include <avtLabelRenderer.h>

// ****************************************************************************
// Class: avtOpenGLLabelRenderer
//
// Purpose:
//   This renderer renders labels using OpenGL.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:47:43 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 15:58:06 PST 2004
//   Moved a method from the base class to this class.
//
//   Brad Whitlock, Thu Aug 4 10:35:24 PDT 2005
//   Added some new helper methods.
//
// ****************************************************************************

class avtOpenGLLabelRenderer : public avtLabelRenderer
{
public:
                            avtOpenGLLabelRenderer();
    virtual                ~avtOpenGLLabelRenderer();
    virtual void            ReleaseGraphicsResources();

protected:
    void SetupGraphicsLibrary();
    void RenderLabels();
    void DrawLabel(const float *vert, const char *label);
    void DrawLabel2(const float *vert, const char *label);
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

    float                  x_scale;
    float                  y_scale;
    int                    characterDisplayListIndices[256];
    bool                   characterDisplayListsCreated;

    int                    zBufferMode;
    float                 *zBuffer;
    int                    zBufferWidth;
    int                    zBufferHeight;
    float                  zTolerance;
    static bool            zBufferWarningIssued;
};

#endif
