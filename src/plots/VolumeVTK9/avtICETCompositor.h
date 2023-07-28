// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtICETCompositor.h                             //
// ************************************************************************* //

#ifndef AVT_ICET_COMPOSITOR_H
#define AVT_ICET_COMPOSITOR_H

#include <avtImage.h>

#include <vtkImageData.h>

#if defined(PARALLEL) && defined(HAVE_ICET)
    #include <IceT.h>
    #include <IceTMPI.h>
#endif

class avtICETCompositor
{
public:
    avtICETCompositor();
    avtICETCompositor(float zDepth, float * bg_color = nullptr);
    ~avtICETCompositor();

    void            Composite(const void *, void * const, const int, const int, const int);
private:
#if defined(PARALLEL) && defined(HAVE_ICET)
    void            InitIceT(float depth);
    void            CompositeIceT(void * const, const int, const int);

    static void     GetProcessRanks(float, const int, IceTInt * const);
    static void     IceTDrawCallback(const IceTDouble*,
                                     const IceTDouble*,
                                     const IceTFloat*,
                                     const IceTInt*,
                                     IceTImage);

    IceTContext     m_icetContext;
    IceTContext     m_prevIceTContext;
#endif

    static const void  *s_colorBuffer;
    static int          s_nColorChannels;
    static int          s_width;
    static int          s_height;

    float               m_zDepth;
    float               m_bgColor[4] {0.0, 0.0, 0.0, 1.0};
};

#endif
