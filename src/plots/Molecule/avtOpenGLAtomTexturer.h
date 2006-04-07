#ifndef AVT_OPENGL_ATOM_TEXTURER_H
#define AVT_OPENGL_ATOM_TEXTURER_H

// ****************************************************************************
// Class: avtOpenGLAtomTexturer
//
// Purpose:
//   Performs sphere shading on imposter quads using different methods.
//
// Notes:      There are 2 methods: texture-based, shader-based.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 28 09:56:02 PDT 2006
//
// Modifications:
//   Brad Whitlock, Fri Apr 7 11:26:48 PDT 2006
//   Added SetHint method.
//
// ****************************************************************************

class avtOpenGLAtomTexturer
{
public:
    avtOpenGLAtomTexturer();
    virtual ~avtOpenGLAtomTexturer();

    void BeginSphereTexturing();
    void EndSphereTexturing();

    static const int HINT_SET_DEPTH;
    static const int HINT_SET_SCREEN_WIDTH;
    static const int HINT_SET_SCREEN_HEIGHT;

    void SetHint(int hint, int value);

    typedef enum {TextureMode, ShaderMode} TexMode_t;
    TexMode_t GetMode();

private:
    bool modeDetermined;
    TexMode_t mode;
    void *tData;
    void *sData;
};


#endif
