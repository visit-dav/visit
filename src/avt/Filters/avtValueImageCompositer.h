// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtValueImageCompositer.h                           //
// ************************************************************************* //

#ifndef AVT_VALUE_IMAGE_COMPOSITER_H
#define AVT_VALUE_IMAGE_COMPOSITER_H

#include <filters_exports.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtWholeImageCompositer.h>

// ****************************************************************************
//  Class: avtValueImageCompositer
//
//  Purpose:
//      An image compositor for value images. It's based off of the 
//      avtWholeImageCompositerWithZ class.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 25 13:09:39 PDT 2017
//
// ****************************************************************************

class AVTFILTERS_API avtValueImageCompositer :
    public avtWholeImageCompositer
{
public:
                            avtValueImageCompositer();
    virtual                ~avtValueImageCompositer();

    const char             *GetType(void);
    const char             *GetDescription(void);

    void                    Execute();

    void SetBackground(float);
private:
    float bg_value;

    void                    MergeBuffers(int npixels, bool doParallel,
                                 const float *inz, const float *in,
                                 float *ioz, float *io);

    static void             InitializeMPIStuff();
    static void             FinalizeMPIStuff();

    static int              objectCount;

#ifdef PARALLEL
    static MPI_Datatype     mpiTypeZFPixel;
    static MPI_Op           mpiOpMergeZFPixelBuffers;
#endif

};

inline const char* avtValueImageCompositer::GetType()
{ return "avtValueImageCompositer";}

inline const char* avtValueImageCompositer::GetDescription()
{ return "performing whole-image composite with zbuffer"; }

#endif
