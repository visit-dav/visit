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
//   
// ****************************************************************************

class avtOpenGLAtomTexturer
{
public:
    avtOpenGLAtomTexturer();
    virtual ~avtOpenGLAtomTexturer();

    void BeginSphereTexturing();
    void EndSphereTexturing();

    typedef enum {TextureMode, ShaderMode} TexMode_t;
    TexMode_t GetMode();

private:
    bool modeDetermined;
    TexMode_t mode;
    void *tData;
    void *sData;
};


#endif
