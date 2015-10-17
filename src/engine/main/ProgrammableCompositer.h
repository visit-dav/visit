/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                       ProgrammableCompositer.h                              //
// ************************************************************************* //

#ifndef ProgrammableCompositer_h
#define ProgrammableCompositer_h

#include <filters_exports.h>
#include <vector>
#include <cstdlib>
#include <avtImage.h>

#ifndef _MSC_VER
#define THREADED_COMPOSITER
#endif
#ifdef THREADED_COMPOSITER
#include <pthread.h>
#endif

template <typename T>
class CompositerThreadManager;

//#define ProgrammableCompositerDEBUG

// ****************************************************************************
//  Class: ProgrammableCompositer
//
//  Purpose:
//      An image compositer for z-buffer and rank ordered alpha compositing.
//
//      The key features are:
//
//          * the communications pattern isn't hard wired, it's provided
//            by a "program" so that various compositing strategies
//            may be easily adopted, and selected at run-time.
//
//          * compositing kernels are written with vector processors
//            and vectorizing compilers in mind.
//
//          * compositing and communication are overlapped by chunking
//            the communication and running the compositing kernels in
//            threads. a thread pool is created for this purpose.
//
//          * when the thread pool has 0 threads the computaions are
//            serialized. However, you still incur the threading overheads
//            of locks, mutexs etc. if you don't want threading then do not
//            compile with it. ie don't define THREADED_COMPOSITER.
//
//      Notes on usage:
//
//          * if a z-buffer is provided in the input z-buffer compositing
//            is performed.
//
//          * when a z-buffer is not provided in the input alpha blending
//            is performed,
//
//          * when alpha blending, a rank order describing back to front
//            render order must be specified.
//
//          * when alpha blending the z-buffer, background color, and
//            background image of the opaque rendering must be provided
//
//          * to get a correct result when alpha blending all non-distributed
//            geometry must be removed from the scene. this includes things
//            like axes, and 3D text, etc. anything that exists only on one
//            node.
//
//          * the inputs and outputs are individual r,g,b,a,z channels
//            to facilitate vectorization.
//
//          * if output buffers are provided then results are written
//            directly to them eliminating a memcpy.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Aug 20 11:46:48 PDT 2015
//
// ****************************************************************************

template <typename T>
class AVTFILTERS_API ProgrammableCompositer
{
public:
         ProgrammableCompositer();
         ~ProgrammableCompositer();

    void Clear();
    void Initialize(int aw, int ah);
    void SetThreadPoolSize(size_t an);
    void SetBlocking(size_t ablocking)
         { blocking = ablocking; }

    void Execute();

    void SetOrder(const std::vector<int> &aorder)
         { order = aorder; }

    int  GetRoot()
         { return order.empty() ? 0 : order.front(); }

    void SetInput(T *ar, T *ag, T *ab, T *aa, float *az, bool aown);

    void SetOutput(T *ar, T *ag, T *ab, T *aa, float *az, bool aown);
    void GetOutput(T *&ar, T *&ag, T *&ab, T *&aa, float *&az, bool take)
         { ar = ro; ag = go; ab = bo; aa = ao; az = zo; ownout = !take; }

    void SetBackground(T *ar, T *ag, T *ab, float *az, bool aown);
    void SetBackgroundColor(const double abg[3]);

    void ApplyBackgroundColor(const double abg[3]);

    template <typename U>
    void ApplyBackgroundImage(const U *ari, const U *agi, const U *abi);

    void ApplyBackgroundImage(const unsigned char *argba);

    void SetBlockSize(int s){ blocking = s; }

    void SetBroadcastColor(bool abcast)
         { bcastrgba = abcast; }

    void SetBroadcastDepth(bool abcast)
         { bcastz = abcast; }

protected:
         ProgrammableCompositer(const ProgrammableCompositer &); // not implemented
    void operator=(const ProgrammableCompositer &); // not implemented

private:
    std::vector<int>        order;      // rank order from which communication tree is constructed
    T *                     ri;         // input image
    T *                     gi;
    T *                     bi;
    T *                     ai;
    float *                 zi;
    T *                     ro;         // output image,if not bcastrgba, only valid on rank 0
    T *                     go;
    T *                     bo;
    T *                     ao;
    float *                 zo;         // output depth buffer, if not bcastz, only valid on rank 0
    T *                     rb;         // background image, required on the root, unused elsewhere
    T *                     gb;
    T *                     bb;
    T *                     ab;
    float *                 zb;
    T                       bgrgba[4];  // background color
    int                     w;          // input,output,bg image size
    int                     h;
    int                     blocking;   // communication is made in chunks of this size
    bool                    ownin;      // if set free input image
    bool                    ownout;     // if set free output image
    bool                    ownbg;      // if set free background image
    bool                    bcastrgba;  // composited image is bcast to other ranks
    bool                    bcastz;     // composited depoth values are bcast to other ranks
    CompositerThreadManager<T> *tmgr;   // thread manager
};

#include <avtImageRepresentation.h>
#include <vtkImageData.h>

#include <iostream>
#include <cstdlib>
#ifndef HAVE_ALIGNED_ALLOC
#define aligned_alloc(_a, _n) \
    malloc(_n)
#endif
#define alignment VISIT_MEM_ALIGN

// if set then new []/delete [] are used for z-bufffer.
// this is because wew can skip a memcpy from VTK if we
// use delete []. however if not using VTK we'd like to
// use aligned_alloc/free so that operations are more
// efficiently vectorized
#define Z_DELETE
inline
void freez(float *z)
{
#if defined(Z_DELETE)
    delete [] z;
#else
    free(z);
#endif
}

inline
float *newz(size_t n)
{
#if defined(Z_DELETE)
    return new float [n];
#else
    return aligned_alloc(alignment, sizeof(float)*n);
#endif
}

// using these to construct simple programs
// that describe the communication pattern
// used in the reduction
typedef std::vector<int> RankOrdering;
typedef std::pair<int,int> RankPair;
typedef std::vector<RankPair> Iteration;
typedef std::vector<Iteration> Program;

AVTFILTERS_API void printProgram(Program &prog);
AVTFILTERS_API Program makeProgram(const RankOrdering &o);
AVTFILTERS_API std::ostream &operator<<(std::ostream &os, const RankPair &p);

// --------------------------------------------------------------------------
template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
{
    size_t n = v.size();
    for (size_t i = 0; i < n; ++i)
        os << v[i] << " ";
    return os;
}

// --------------------------------------------------------------------------
inline
int receiver(const RankPair &p)
{ return p.first; }

// --------------------------------------------------------------------------
inline
int sender(const RankPair &p)
{ return p.second; }




// ****************************************************************************
//  Functions: writeVTK, NewAvtImage
//
//  Purpose:
//      helpers for converting from r,g,b,a,z into avt freindly data
//      structures. I don't want my conpositer to have any AVT dependency
//      so I put these methods outside.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Sep 10 12:51:26 PDT 2015
//
// ****************************************************************************
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkImageData.h>
#include <vtkDataSetWriter.h>
#include <sstream>
#include <string>
#include <iostream>
#ifdef PARALLEL
#include <mpi.h>
#endif
#include <avtParallel.h>

// some templated helper for dealing with VTK types
template <typename T> struct vtk_tt {};

template <> struct vtk_tt<float>
{ typedef vtkFloatArray array_t; };

template <> struct vtk_tt<unsigned char>
{ typedef vtkUnsignedCharArray array_t; };

// pass the array p of length ninto a new VTK array of the coresponding type
// keep is 0 if you want VTK to delete it del is 0 if you want VTK to free it
// and 1 if you want VTK to delete [] it.
template <typename T>
typename vtk_tt<T>::array_t *newVTKArray(const char *name, T *p, size_t n, int keep, int del)
{
    typename vtk_tt<T>::array_t *a = vtk_tt<T>::array_t::New();
    a->SetName(name);
    a->SetArray(p, n, keep, del);
    return a;
}

// create a VTK array of the coresponding type from the array given by point p
// of length n. keep is 0 if you want VTK to delete it del is 0 if you want VTK
// to free it and 1 if you want VTK to delete [] it.
template <typename T>
void AddPointData(vtkDataSet *d, const char *name, T *p, size_t n, int keep, int del)
{
    if (!p) return;
    typename vtk_tt<T>::array_t *a = newVTKArray(name, p, n, keep, del);
    d->GetPointData()->AddArray(a);
    a->Delete();
}


#ifdef ProgrammableCompositerDEBUG
#include <vtkImageData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkDataSetWriter.h>
#include <vtkFloatArray.h>
#include <sstream>
#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

template <typename T>
void writeVTK(const char *file, T *r, T *g, T *b, T *a,
    float *z, int w, int h)
{
    vtkImageData *id = vtkImageData::New();
    id->SetExtent(0, w-1, 0, h-1, 0, 0);
    size_t n = w*h;
    AddPointData(id, "0r", r, n, 1, 0);
    AddPointData(id, "1g", g, n, 1, 0);
    AddPointData(id, "2b", b, n, 1, 0);
    AddPointData(id, "3a", a, n, 1, 0);
    AddPointData(id, "4z", z, n, 1, 0);
    std::ostringstream oss;
#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    oss << rank << "_";
#endif
    oss << file;
    vtkDataSetWriter *dw = vtkDataSetWriter::New();
    dw->SetInputData(id);
    dw->SetFileName(oss.str().c_str());
    dw->Write();
    dw->Delete();
    id->Delete();
}


inline
void writeVTK(const char *file, avtImageRepresentation &aim)
{
    vtkImageData *im = aim.GetImageVTK();
    vtkImageData *id = vtkImageData::New();
    id->SetDimensions(im->GetDimensions());
    vtkDataArray *scalars = im->GetPointData()->GetScalars();
    int nComp = scalars->GetNumberOfComponents();
    int nTup = scalars->GetNumberOfTuples();
    const char *names[] = {"0r", "1g", "2b", "3a"};
    for (int i = 0; i < nComp; ++i)
    {
        vtkDataArray *c = scalars->NewInstance();
        c->SetName(names[i]);
        c->SetNumberOfTuples(nTup);
        switch (c->GetDataType())
        {
        vtkTemplateMacro(
            VTK_TT *pScalars = static_cast<VTK_TT*>(scalars->GetVoidPointer(0));
            VTK_TT *pc = static_cast<VTK_TT*>(c->GetVoidPointer(0));
            for (int j = 0; j < nTup; ++j)
                pc[j] = pScalars[j*nComp+i];
        );
        }
        id->GetPointData()->AddArray(c);
        c->Delete();
    }
    if (aim.GetZBufferVTK())
        id->GetPointData()->AddArray(aim.GetZBufferVTK());
    std::ostringstream oss;
#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    oss << rank << "_";
#endif
    oss << file;
    vtkDataSetWriter *dw = vtkDataSetWriter::New();
    dw->SetInputData(id);
    dw->SetFileName(oss.str().c_str());
    dw->Write();
    dw->Delete();
    id->Delete();
}
#endif







// ****************************************************************************
//  Class: color_tt
//
//  Purpose:
//      a type traits class encapsulating conversion to/from
//      color of various precision/formats
//
//  Programmer: Burlen Loring
//  Creation:   Tue Sep  1 09:34:49 PDT 2015
//
// ****************************************************************************

template <typename T> struct color_tt {};

template <>
struct color_tt<float>
{
    static float max() { return 1.0f; }

    static unsigned char asuchar(const float &c) { return c*255.0f; }
    static float asfloat(const float &c) { return c; }
#ifdef PARALLEL
    static MPI_Datatype asmpi(){ return MPI_FLOAT; };
#endif
    static float from(const unsigned char &c) { return c*(1.0f/255.0); }
    static float from(const float &c) { return c; }
    static float from(const double &c) { return c; }
};

template <>
struct color_tt<unsigned char>
{
    static unsigned char max() { return 255u; }

    static unsigned char asuchar(const unsigned char &c) { return c; }
    static float asfloat(const unsigned char &c) { return c*(1.0f/255.0f); }
#ifdef PARALLEL
    static MPI_Datatype asmpi(){ return MPI_BYTE; };
#endif
    static unsigned char from(const unsigned char &c) { return c; }
    static unsigned char from(const float &c) { return c*255.0f; }
    static unsigned char from(const double &c) { return c*255.0; }
};

// ****************************************************************************
//  Helper Functions:
//
//      split -- splits an rgba image into r,g,b,a channels
//      merge -- inverse of split, take r,g,b,a into rgba image,
//      blend -- the porter duff over/under blending operator
//      composite -- z-buffer compositing
//      multiply -- element wise multiplication of 2 arrays
//      remainder -- compute 1-x where x is in the range of 0-1
//
//  Purpose:
//      A set of compiler vectorizable (compile w/ -O3) helper functions
//      that do the compositing maths in native precision (or in float
//      precision for alpha blending)
//
//  Programmer: Burlen Loring
//  Creation:   Tue Sep  1 09:34:49 PDT 2015
//
// ****************************************************************************
// --------------------------------------------------------------------------
// split rgba into r,g,b,a channels
// cs is an int that selects the channel to split, vectorized
template<typename T, int nc>
void split(
    T * __restrict__ c,
    const unsigned char * __restrict__ rgba,
    size_t n, size_t cs)
{
    const unsigned char *prgba = rgba + cs;
    for (size_t i = 0; i < n; ++i)
        c[i] = color_tt<T>::from(prgba[nc*i]);
}

// --------------------------------------------------------------------------
// merge r,g,b,a channels into combined rgba
// cs is an int that selects the channel to merge, not vectorized
template <typename num_t, int nc>
void merge(
    unsigned char * __restrict__ rgba,
    const num_t * __restrict__ c,
    size_t n,
    size_t cs)
{
    unsigned char *prgba = rgba + cs;
    for (size_t i = 0; i < n; ++i)
        prgba[nc*i] = color_tt<num_t>::asuchar(c[i]);
}

// --------------------------------------------------------------------------
// from a compute 1 - a, vectorized
template <typename num_t>
void remainder(
    num_t * __restrict__ r,
    const num_t * __restrict__ a,
    size_t n)
{
    for (size_t i = 0; i < n; ++i)
        r[i] = color_tt<num_t>::max() - a[i];
}

// --------------------------------------------------------------------------
// blend foreground and background channel in float precision
// here raf is 1-alpha foreground, vectorized
template <typename f_num_t, typename b_num_t>
void blend(
    b_num_t * __restrict__ o,
    const f_num_t * __restrict__ f,
    const b_num_t * __restrict__ b,
    const f_num_t * __restrict__ raf,
    size_t n)
{
    for (size_t i = 0; i < n; ++i)
        o[i] = color_tt<b_num_t>::from(color_tt<f_num_t>::asfloat(f[i])
            + color_tt<b_num_t>::asfloat(b[i])*color_tt<f_num_t>::asfloat(raf[i]));
}

// --------------------------------------------------------------------------
// composite a single channel, TODO -- not vectorized.
template <typename num_t>
void composite(
    num_t * __restrict__ oc,
    const num_t * __restrict__ fc,
    const float * __restrict__ fz,
    const num_t * __restrict__ bc,
    const float * __restrict__ bz,
    size_t n)
{
    for (size_t i = 0; i < n; ++i)
        oc[i] = fz[i] < bz[i] ? fc[i] : bc[i];
}

// --------------------------------------------------------------------------
// composite z-buffer, vectorized
inline
void composite(
    float * __restrict__ oz,
    const float * __restrict__ fz,
    const float * __restrict__ bz,
    size_t n)
{
    for (size_t i = 0; i < n; ++i)
        oz[i] = fz[i] < bz[i] ? fz[i] : bz[i];
}

// --------------------------------------------------------------------------
// return a mask value indicating if the fg wins the depth test, vectorized
template <typename num_t>
num_t *ztest(float * __restrict__ fz, float * __restrict__ bz,  size_t n)
{
    num_t *m = static_cast<num_t*>(aligned_alloc(alignment, n*sizeof(num_t)));
    for (size_t i = 0; i < n; ++i)
        m[i] = fz[i] < bz[i] ? 1 : 0;
    return m;
}

// --------------------------------------------------------------------------
// composite, single channel with mask array, vectorized
// TODO -- this is vectorized in single precision but should be uchar
template <typename num_t>
void composite(
    num_t * __restrict__ oc,
    const num_t * __restrict__ fc,
    const num_t * __restrict__ bc,
    const num_t * __restrict__ zm,
    size_t n)
{
    for (size_t i = 0; i < n; ++i)
        oc[i] = zm[i]*fc[i] + (1 - zm[i])*bc[i];
}

// --------------------------------------------------------------------------
// multiply c by a in place, not vectorized (because in place)
template <typename num_t>
void multiply(
    num_t * __restrict__ c,
    const num_t * __restrict__ a,
    size_t n)
{
    for (size_t i = 0; i < n; ++i)
        c[i] *= a[i];
}



// ****************************************************************************
//  Class ImageBuffer
//
//  Purpose:
//      A simple container for split r,g,b,a,z images. It handles allocation,
//      deallocation and makes it easy to pass these around as one unit.
//
//  Programmer: Burlen Loring
//  Creation:   Tue Sep  1 09:34:49 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename im_t>
class ImageBuffer
{
public:
    // construct a buffer of size an.
    // ac, aa, and az indicates wether or not to
    // allocate rgb colors, alpha channel, and z-buffer.
    explicit ImageBuffer(size_t an, bool ac, bool aa, bool az, bool aown);

    // construct a buffer of size an.
    // initialize with a uniform rgba color
    // no z-buffer is allocated
    explicit ImageBuffer(im_t ar, im_t ag, im_t ab, im_t aa, size_t an);

    // set the rgba and z-buffer of size an directly.
    // aown indicates if we take ownership
    explicit ImageBuffer(im_t *ar, im_t *ag, im_t *ab, im_t *aa, float *az,
        size_t an, bool aown = false) : r(ar), g(ag), b(ab), a(aa),
        z(az), size(an), own(aown) {}

    // construct from an interleaved 4 channel buffer
    explicit ImageBuffer(const unsigned char * __restrict__ rgba,
        const float * __restrict__ z, size_t an, int nchani, int nchano,
        bool premul=false, bool aown=true);

    ~ImageBuffer();

protected:
    ImageBuffer(); // not implemented
    ImageBuffer(const ImageBuffer &); // not implemented
    void operator=(const ImageBuffer &); // not implemented

public:
    im_t *r;
    im_t *g;
    im_t *b;
    im_t *a;
    float *z;
    size_t size;
    bool own;
};

// --------------------------------------------------------------------------
// allocates space to hold an image of size and id
template <typename im_t>
ImageBuffer<im_t>::ImageBuffer(size_t an, bool ac, bool aa, bool az, bool aown)
    : r(0), g(0), b(0), a(0), z(0), size(an), own(aown)
{
    size_t nbytes = sizeof(im_t)*size;
    if (ac)
    {
        r = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
        g = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
        b = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
    }
    if (aa)
        a = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
    if (az)
        z = newz(size);
}

// --------------------------------------------------------------------------
// allocates space to hold an image of size and id
// initializes it to the specified constant color
template <typename im_t>
ImageBuffer<im_t>::ImageBuffer(im_t ar, im_t ag, im_t ab, im_t aa, size_t an)
    : r(0), g(0), b(0), a(0), z(0), size(an), own(true)
{
    size_t nbytes = sizeof(im_t)*size;

    r = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
    for (int i = 0; i < size; ++i)
        r[i] = ar;

    g = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
    for (int i = 0; i < size; ++i)
        g[i] = ag;

    b = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
    for (int i = 0; i < size; ++i)
        b[i] = ab;

    a = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
    for (int i = 0; i < size; ++i)
        a[i] = aa;
}

// --------------------------------------------------------------------------
// convert an unsigned char image, in rgba, to the internally used type
// and split into constituent r,g,b,a channels. if az is provided a depth
// buffer is allocated and az is coppied in. if premulf is set then colors
// are multiplied by their alpha. in VTK this is not needed.
template <typename im_t>
ImageBuffer<im_t>::ImageBuffer(
    const unsigned char * __restrict__ rgba,
    const float * __restrict__ az, size_t an, int nchani, int nchano,
    bool premulf, bool aown) : r(0), g(0), b(0), a(0), z(0),
    size(an), own(aown)
{
    size_t nbytes = sizeof(im_t)*size;
    if (rgba)
    {
        if (nchano == 4)
        {
            a = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
            if (nchani == 4)
                split<im_t,4>(a, rgba, size, 3);
            else
                memset(a, 0, nbytes);
        }

        r = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
        g = static_cast<im_t*>(aligned_alloc(alignment, nbytes));
        b = static_cast<im_t*>(aligned_alloc(alignment, nbytes));

        if (nchani == 4)
        {
            split<im_t,4>(r, rgba, size, 0);
            split<im_t,4>(g, rgba, size, 1);
            split<im_t,4>(b, rgba, size, 2);
        }
        else
        {
            split<im_t,3>(r, rgba, size, 0);
            split<im_t,3>(g, rgba, size, 1);
            split<im_t,3>(b, rgba, size, 2);
        }
        if (premulf)
        {
            multiply(r, a, size);
            multiply(g, a, size);
            multiply(b, a, size);
        }
    }
    if (az)
    {
        z = newz(size);
        memcpy(z, az, size*sizeof(float));
    }
}

// --------------------------------------------------------------------------
template <typename im_t>
ImageBuffer<im_t>::~ImageBuffer()
{
    if (own)
    {
        free(r);
        free(g);
        free(b);
        free(a);
        freez(z);
    }
}

// --------------------------------------------------------------------------
// blend foreground and background images
// store the result in the output image.
template <typename im_t>
void blend(ImageBuffer<im_t> *oim, ImageBuffer<im_t> *fim, ImageBuffer<im_t> *bim)
{
    // images assumed to be the same size
    size_t n = oim->size;

    im_t *raf = static_cast<im_t*>(aligned_alloc(alignment, n*sizeof(im_t)));
    remainder(raf, fim->a, n);

    blend(oim->r, fim->r, bim->r, raf, n);
    blend(oim->g, fim->g, bim->g, raf, n);
    blend(oim->b, fim->b, bim->b, raf, n);
    blend(oim->a, fim->a, bim->a, raf, n);

    free(raf);
}

// --------------------------------------------------------------------------
template <typename im_t>
void composite(ImageBuffer<im_t> *oim, ImageBuffer<im_t> *fim, ImageBuffer<im_t> *bim)
{
    // images assumed to be the same size
    size_t n = oim->size;
    im_t *zm = ztest<im_t>(fim->z, bim->z, n);
    composite(oim->r, fim->r, bim->r, zm, n);
    composite(oim->g, fim->g, bim->g, zm, n);
    composite(oim->b, fim->b, bim->b, zm, n);
    if (fim->a)
        composite(oim->a, fim->a, bim->a, zm, n);
    composite(oim->z, fim->z, bim->z, n);
    free(zm);
}

// --------------------------------------------------------------------------
// TODO -- this version was not vectorized by gcc, the above is but
// uses float rather than char ops.
template <typename im_t>
void composite2(ImageBuffer<im_t> *oim, ImageBuffer<im_t> *fim, ImageBuffer<im_t> *bim)
{
    // images assumed to be the same size
    size_t n = oim->size;

    composite(oim->r, fim->r, fim->z, bim->r, bim->z, n);
    composite(oim->g, fim->g, fim->z, bim->g, bim->z, n);
    composite(oim->b, fim->b, fim->z, bim->b, bim->z, n);
    if (fim->a)
        composite(oim->a, fim->a, fim->z, bim->a, bim->z, n);
    composite(oim->z, fim->z, bim->z, n);
}




// ****************************************************************************
//  Class CompositerThreadManager
//
//  Purpose:
//
//      A thread pool for compositing work. worklets are triplets
//      containing pointers to forgeground, background and output
//      buffers to different segments of the same image. work is
//      performed using 0 or more threads.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
struct CompositerWorklet
{
    CompositerWorklet() : out(NULL), fg(NULL), bg(NULL) {}

    CompositerWorklet(ImageBuffer<T> *aout, ImageBuffer<T> *afg,
        ImageBuffer<T> *abg) : out(aout), fg(afg), bg(abg) {}

    ImageBuffer<T> *out;
    ImageBuffer<T> *fg;
    ImageBuffer<T> *bg;
};

template <typename T>
class CompositerThreadManager
{
public:
    CompositerThreadManager();
    ~CompositerThreadManager();

    // create the thread pool
    void SetThreadPoolSize(size_t nThreads);
    size_t GetNumberOfThreads();

    // destroy the thread pool. blocks until all workers have completed.
    void FinalizeThreads();

    // block until the pending work has been finished
    void WaitFinished();

    // test to see if all work has been done
    bool Finished();

    // push an item onto the back of the queue
    // and signal that there is new work to be done
    void AddWorklet(ImageBuffer<T> *out, ImageBuffer<T> *fg, ImageBuffer<T> *bg);

    // push an item onto the back of the queue
    // and signal that there is new work to be done
    void AddWorklet(const CompositerWorklet<T> &work);

    // get an item from the back of the queue.
    // this call blocks until the queue is not empty. the item
    // is removed from the queue and the pending count is
    // incremented.
    CompositerWorklet<T> GetWorklet();

    // decrement the pending count when work has been completed
    void WorkletComplete();

private:
#ifdef THREADED_COMPOSITER
    pthread_mutex_t m_mutex;
    pthread_cond_t m_have;
    std::vector<pthread_t> m_workers;
#endif
    std::vector<CompositerWorklet<T> > m_queue;
    size_t m_pending;
};

// ****************************************************************************
//  Function CompositerThread
//
//  Purpose:
//      this is the function that all threads in the thread pool
//      execute. it waits for work and when available it does it.
//      An empty worklet indicates that the method should return.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

#ifdef THREADED_COMPOSITER
template <typename T>
void *CompositerThread(void *arg)
{
    CompositerThreadManager<T> *tmgr =
        static_cast<CompositerThreadManager<T>*>(arg);

    while (1)
    {
        // block until there is some work to do
        CompositerWorklet<T> work = tmgr->GetWorklet();
#ifdef ProgrammableCompositerDEBUG
        std::cerr << ".";
#endif
        // terminate the thread if we encounter an empty output
        if (!work.out)
            break;

        // do the work. either depth composite or alpha blend
        if (work.out->z)
            composite(work.out, work.fg, work.bg);
        else
            blend(work.out, work.fg, work.bg);

        // decrement the pending count
        tmgr->WorkletComplete();
    }
    return NULL;
}
#endif


// ****************************************************************************
//  Method CompositerThreadManager
//
//  Purpose:
//      constructor
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
CompositerThreadManager<T>::CompositerThreadManager() :
#ifdef THREADED_COMPOSITER
    m_mutex(), m_have(), m_workers(),
#endif
    m_queue(), m_pending(0)
{
#ifdef THREADED_COMPOSITER
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_have, NULL);
#endif
}


// ****************************************************************************
//  Method ~CompositerThreadManager
//
//  Purpose:
//      destructor
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
CompositerThreadManager<T>::~CompositerThreadManager()
{
#ifdef THREADED_COMPOSITER
    FinalizeThreads();
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_have);
#endif
}

// ****************************************************************************
//  Method SetThreadPoolSize
//
//  Purpose:
//      creates (or resizes) the thread pool
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void CompositerThreadManager<T>::SetThreadPoolSize(size_t nThreads)
{
#ifndef THREADED_COMPOSITER
    (void)nThreads;
#else
   if (nThreads == m_workers.size())
       return;

    if (m_workers.size())
        FinalizeThreads();

    m_workers.resize(nThreads);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (size_t i = 0; i < nThreads; ++i)
        pthread_create(&m_workers[i], &attr, CompositerThread<T>, (void *)this);
    pthread_attr_destroy(&attr);
#ifdef ProgrammableCompositerDEBUG
    std::cerr << "created thread pool with " << nThreads << std::endl;
#endif
#endif
}

// ****************************************************************************
//  Method FinalizeThreads
//
//  Purpose:
//      shuts down and cleans up the thread pool
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void CompositerThreadManager<T>::FinalizeThreads()
{
#ifdef THREADED_COMPOSITER
    WaitFinished();

    // tell each worker to exit by sending an empty worklet
    CompositerWorklet<T> null;
    size_t nthreads = m_workers.size();
    for (size_t i = 0; i < nthreads; ++i)
        AddWorklet(null);

    // wait for workers to exit
    for (size_t i = 0; i < nthreads; ++i)
        pthread_join(m_workers[i], NULL);

    m_pending = 0;
    m_workers.clear();
#endif
}

// ****************************************************************************
//  Method GetNumberOfThreads
//
//  Purpose:
//      returns the thread pool size
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
size_t CompositerThreadManager<T>::GetNumberOfThreads()
{
#ifndef THREADED_COMPOSITER
    return 0;
#else
    return m_workers.size();
#endif
}

// ****************************************************************************
//  Method WaitFinished
//
//  Purpose:
//      Block until all of the work in the queue is complete. if there
//      is any remaining work then the calling thread pitches in and
//      does it.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void CompositerThreadManager<T>::WaitFinished()
{
    while (!m_queue.empty() || m_pending)
    {
#ifdef THREADED_COMPOSITER
        pthread_mutex_lock(&m_mutex);
#endif
        if (!m_queue.empty())
        {
            CompositerWorklet<T> work = m_queue.back();
            m_queue.pop_back();
#ifdef THREADED_COMPOSITER
            pthread_mutex_unlock(&m_mutex);
#endif
            if (work.out->z)
                composite(work.out, work.fg, work.bg);
            else
                blend(work.out, work.fg, work.bg);
        }
#ifdef THREADED_COMPOSITER
        else
        {
            pthread_mutex_unlock(&m_mutex);
        }
#endif
    }
}

// ****************************************************************************
//  Method Finished
//
//  Purpose:
//      test if all the work is finished.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
bool CompositerThreadManager<T>::Finished()
{
#ifdef THREADED_COMPOSITER
    pthread_mutex_lock(&m_mutex);
#endif
    bool e = m_queue.empty() && (m_pending == 0);
#ifdef THREADED_COMPOSITER
    pthread_mutex_unlock(&m_mutex);
#endif
    return e;
}

// ****************************************************************************
//  Method AddWorklet
//
//  Purpose:
//      construct and push an item onto the back of the queue
//      and signal that there is new work to be done
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void CompositerThreadManager<T>::AddWorklet(
    ImageBuffer<T> *out, ImageBuffer<T> *fg, ImageBuffer<T> *bg)
{
    AddWorklet(CompositerWorklet<T>(out, fg, bg));
}

// ****************************************************************************
//  Method AddWorklet
//
//  Purpose:
//      push an item onto the back of the queue
//      and signal that there is new work to be done
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void CompositerThreadManager<T>::AddWorklet(const CompositerWorklet<T> &work)
{
#ifdef THREADED_COMPOSITER
    pthread_mutex_lock(&m_mutex);
#endif
    m_queue.push_back(work);
#ifdef THREADED_COMPOSITER
    pthread_cond_signal(&m_have);
    pthread_mutex_unlock(&m_mutex);
#endif
}

// ****************************************************************************
//  Method GetWorklet
//
//  Purpose:
//      get an item from the back of the queue. this call blocks until the
//      queue is not empty. the item is then removed from the queue and the
//      pending count is incremented. the pending count should be decremented
//      when the work is finished.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
CompositerWorklet<T> CompositerThreadManager<T>::GetWorklet()
{
#ifdef THREADED_COMPOSITER
    pthread_mutex_lock(&m_mutex);
    while (m_queue.empty())
        pthread_cond_wait(&m_have, &m_mutex);
#endif
    CompositerWorklet<T> work = m_queue.back();
    m_queue.pop_back();
    ++m_pending;
#ifdef THREADED_COMPOSITER
    pthread_mutex_unlock(&m_mutex);
#endif
    return work;
}

// ****************************************************************************
//  Method WorkletComplete
//
//  Purpose:
//      decrement the pending count when work has been completed
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 10:59:07 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void CompositerThreadManager<T>::WorkletComplete()
{
#ifdef THREADED_COMPOSITER
    pthread_mutex_lock(&m_mutex);
#endif
    --m_pending;
#ifdef THREADED_COMPOSITER
    pthread_mutex_unlock(&m_mutex);
#endif
}



// ****************************************************************************
//  Method: ProgrammableCompositer
//
//  Programmer: Burlen Loring
//  Creation:   Thu Aug 20 13:12:38 PDT 2015
//
// ****************************************************************************

template <typename T>
ProgrammableCompositer<T>::ProgrammableCompositer() : order(),
    ri(NULL), gi(NULL), bi(NULL), ai(NULL), zi(NULL),
    ro(NULL), go(NULL), bo(NULL), ao(NULL), zo(NULL),
    rb(NULL), gb(NULL), bb(NULL), ab(NULL),
    bgrgba(), w(0), h(0), blocking(0x3FFFFFFF),
    ownin(false), ownout(false), ownbg(false),
    bcastrgba(false), bcastz(false), tmgr(NULL)
{
#ifdef THREADED_COMPOSITER
    tmgr = new CompositerThreadManager<T>;
    blocking = 65536;
#endif
}

// ****************************************************************************
//  Method: ~ProgrammableCompositer
//
//  Programmer: Burlen Loring
//  Creation:   Thu Aug 20 13:12:38 PDT 2015
//
// ****************************************************************************

template <typename T>
ProgrammableCompositer<T>::~ProgrammableCompositer()
{
    Clear();
#ifdef THREADED_COMPOSITER
    delete tmgr;
#endif
}

// ****************************************************************************
//  Method: Clear
//
//  Purpose: Free any memory owned and initialize to a default state.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 21:36:31 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void
ProgrammableCompositer<T>::Clear()
{
    if (ownin)
    {
        free(ri);
        free(gi);
        free(bi);
        free(ai);
        freez(zi);
    }
    if (ownout)
    {
        free(ro);
        free(go);
        free(bo);
        free(ao);
        freez(zo);
    }
    if (ownbg)
    {
        free(rb);
        free(gb);
        free(bb);
        freez(zb);
    }
    free(ab);
    order.clear();
    ri = NULL;
    gi = NULL;
    bi = NULL;
    ai = NULL;
    zi = NULL;
    ro = NULL;
    go = NULL;
    bo = NULL;
    ao = NULL;
    zo = NULL;
    rb = NULL;
    gb = NULL;
    bb = NULL;
    ab = NULL;
    zb = NULL;
    //memset(bgrgba, 0, 4*sizeof(T));
    w = 0;
    h = 0;
    ownin = false;
    ownout = false;
    ownbg = false;
    bcastrgba = false;
    bcastz = false;
}


// ****************************************************************************
//  Method: Initialize
//
//  Purpose: prepare for compositing an aw by ah image.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 21:36:31 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void
ProgrammableCompositer<T>::Initialize(int aw, int ah)
{
    w = aw;
    h = ah;
}

// ****************************************************************************
//  Method: SetThreadPoolSize
//
//  Purpose: Initialize members to default state in
//      preparation for compositing an aw by ah image.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Sep 28 21:36:31 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void
ProgrammableCompositer<T>::SetThreadPoolSize(size_t n)
{
#ifndef THREADED_COMPOSITER
    (void)n;
#else
    tmgr->SetThreadPoolSize(n);
#endif
}

// ****************************************************************************
//  Method: SetInput
//
//  Purpose: Set this ranks input image. if the own flag is
//           true then the data will be free'd when
//           the class no longer needs it.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Aug 20 13:12:38 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void
ProgrammableCompositer<T>::SetInput(
    T *ar, T *ag, T *ab, T *aa, float *az,
    bool aown)
{
    if (ownin)
    {
        free(ri);
        free(gi);
        free(bi);
        free(ai);
        freez(zi);
    }
    ri = ar;
    gi = ag;
    bi = ab;
    ai = aa;
    zi = az;
    ownin = aown;
}

// ****************************************************************************
//  Method: SetOutput
//
//  Purpose: Set this ranks output image. if the own flag is
//           true then the data will be free'd when
//           the class no longer needs it.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Aug 20 13:12:38 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void
ProgrammableCompositer<T>::SetOutput(
    T *ar, T *ag, T *ab, T *aa, float *az,
    bool aown)
{
    if (ownout)
    {
        free(ro);
        free(go);
        free(bo);
        free(ao);
        freez(zo);
    }
    ro = ar;
    go = ag;
    bo = ab;
    ao = aa;
    zo = az;
    ownout = aown;
}

// ****************************************************************************
//  Method: SetBackground
//
//  Purpose: Set the background image. only rank 0 needs it.
//           if the own flag is true then the data will be
//           free'd when the class no longer needs it.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Aug 20 13:12:38 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void
ProgrammableCompositer<T>::SetBackground(
    T *a_r, T *a_g, T *a_b, float *a_z,
    bool aown)
{
    if (ownbg)
    {
        free(rb);
        free(gb);
        free(bb);
        freez(zb);
    }
    free(ab);

    rb = a_r;
    gb = a_g;
    bb = a_b;
    zb = a_z;

    // background is always opaque.
    size_t n = w*h;
    ab = static_cast<T*>(aligned_alloc(alignment, n*sizeof(T)));
    for (int i = 0; i < n; ++i)
        ab[i] = color_tt<T>::max();

    ownbg = aown;
}

// ****************************************************************************
//  Method: SetBackgroundColor
//
//  Purpose: Set the background color. only rank 0 needs it.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Aug 20 13:12:38 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void
ProgrammableCompositer<T>::SetBackgroundColor(const double rgb[3])
{
    bgrgba[0] = color_tt<T>::from(rgb[0]);
    bgrgba[1] = color_tt<T>::from(rgb[1]);
    bgrgba[2] = color_tt<T>::from(rgb[2]);
    bgrgba[3] = color_tt<T>::max();
}

// ****************************************************************************
//  Method: ApplyBackgroundColor
//
//  Purpose:
//      looks for untouched z-buffer values (ie 1.0) and set
//      the background color to the given values. we have to
//      render with a black background for correct blending.
//      this fixes it.
//
//      helpers:
//          zmask -- generate a mask array that identifies which
//          pixels need to have the background color. the mask
//          is non-zero for pixels with a z value coresponding to
//          the far clipping plane, ie, not occluded by opaque
//          geometry.
//
//          applyZmask -- applies the background color to pixels
//          that have a non-zero mask value.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Sep 10 21:05:16 PDT 2015
//
//  Modifications:
//
//      Burlen Loring, Wed Sep 30 13:28:07 PDT 2015
//      applyZMask was originally written like: c[i] = zm[i] ? bc : c[i]
//      where zm was an int array either 0 or 1 but gcc failed to vectorize
//      that. the only way I found that worked was to encode it as floating
//      point multiplication. both are now vectorized by gcc 4.9
//
//      Burlen Loring, Wed Oct  7 13:02:29 PDT 2015
//      Pass in the background color rather than storing it in a
//      member variable
//
// ****************************************************************************

inline
float *zmask(float * __restrict__ z, size_t n)
{
    float *m = static_cast<float*>(
        aligned_alloc(alignment, n*sizeof(float)));
    for (size_t i = 0; i < n; ++i)
        m[i] = ((1.0f - z[i]) < 1e-4f) ? 1.0f : 0.0f;
    return m;
}

template <typename T>
void applyZMask(T * __restrict__ c, float * __restrict__ zm, size_t n, T bc)
{
    for (size_t i = 0; i < n; ++i)
        c[i] = color_tt<T>::from(zm[i]*color_tt<T>::asfloat(bc) +
            (1.0f - zm[i])*color_tt<T>::asfloat(c[i]));
}

template <typename T>
void
ProgrammableCompositer<T>::ApplyBackgroundColor(const double argb[3])
{
    T bgrgba[] = {color_tt<T>::from(argb[0]), color_tt<T>::from(argb[1]),
        color_tt<T>::from(argb[2])};

    size_t npix = w*h;
    float *zm = zmask(zb, npix);
    applyZMask(rb, zm, npix, bgrgba[0]);
    applyZMask(gb, zm, npix, bgrgba[1]);
    applyZMask(bb, zm, npix, bgrgba[2]);
    free(zm);
}

// ****************************************************************************
//  Method: ApplyBackgroundImage
//
//  Purpose:
//      looks for untouched z-buffer values (ie 1.0) and set
//      copies value from the background image. we have to render
//      with a black background for correct blending. this fixes
//      it.
//
//      helpers:
//          zmask -- generate a mask array that identifies which
//          pixels need to have the background color. the mask
//          is non-zero for pixels with a z value coresponding to
//          the far clipping plane, ie, not occluded by opaque
//          geometry.
//
//          applyZmask -- applies the background color to pixels
//          that have a non-zero mask value.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Sep 10 21:05:16 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T, typename U>
void applyZMask(T * __restrict__ c, float * __restrict__ zm, size_t n, const U *bc)
{
    for (size_t i = 0; i < n; ++i)
        c[i] = color_tt<T>::from(zm[i]*color_tt<U>::asfloat(bc[i]) +
            (1.0f - zm[i])*color_tt<T>::asfloat(c[i]));
}

template <typename T>
template <typename U>
void
ProgrammableCompositer<T>::ApplyBackgroundImage(
    const U *ari, const U *agi, const U *abi)
{
    size_t npix = w*h;
    float *zm = zmask(zb, npix);
    applyZMask(rb, zm, npix, ari);
    applyZMask(gb, zm, npix, agi);
    applyZMask(bb, zm, npix, abi);
    free(zm);
}

// ****************************************************************************
//  Method: ApplyBackgroundImage
//
//  Purpose:
//      looks for untouched z-buffer values (ie 1.0) and set
//      copies value from the background image. we have to render
//      with a black background for correct blending. this fixes
//      it.
//
//      helpers:
//          zmask -- generate a mask array that identifies which
//          pixels need to have the background color. the mask
//          is non-zero for pixels with a z value coresponding to
//          the far clipping plane, ie, not occluded by opaque
//          geometry.
//
//          applyZmask -- applies the background color to pixels
//          that have a non-zero mask value.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Sep 10 21:05:16 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T, int C>
void applyZMask(T * __restrict__ c, float * __restrict__ zm, size_t n,
    const unsigned char *rgba)
{
    const unsigned char *prgba = rgba + C;
    for (size_t i = 0; i < n; ++i)
        c[i] = color_tt<T>::from(zm[i]*color_tt<T>::asfloat(prgba[4*i]) +
            (1.0f - zm[i])*color_tt<T>::asfloat(c[i]));
}

template <typename T>
void
ProgrammableCompositer<T>::ApplyBackgroundImage(const unsigned char *argba)
{
    size_t npix = w*h;
    float *zm = zmask(zb, npix);
    applyZMask<T,1>(rb, zm, npix, argba);
    applyZMask<T,2>(gb, zm, npix, argba);
    applyZMask<T,3>(bb, zm, npix, argba);
    free(zm);
}


// ****************************************************************************
//  Method: Execute
//
//  Purpose: Perform the composite
//
//  Programmer: Burlen Loring
//  Creation:   Thu Aug 20 13:12:38 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
void
ProgrammableCompositer<T>::Execute()
{
    int rank = 0;
#ifdef PARALLEL
    int nranks = 1;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    MPI_Comm_size(VISIT_MPI_COMM, &nranks);

    // if not doing alpha-blending then we are free
    // to specify the order. this will make the tree
    // rooted at rank 0 which is what we want.
    if ((zi) && !order.size())
    {
        for (int i = 0; i < nranks; ++i)
            order.push_back(i);
    }

    // create the communication program that structures the
    // communication as a tree that honors the requested
    // compositing order. this works by making use of the
    // associativity of the blending operation.
    Program program = makeProgram(order);
#ifdef ProgrammableCompositerDEBUG
    if (rank == 0)
    {
        std::cerr << "order = " << order << std::endl;
        printProgram(program);
        std::cerr << std::endl;
    }
#endif
#endif

    size_t npix = w*h;

    /*// VTK sets the GL blend operations up so that this is not necessary!
    // for alpha belnding do the pre-multiplication
    if (!zi)
    {
        multiply(ri, ai, npix);
        multiply(gi, ai, npix);
        multiply(bi, ai, npix);
    } */

    // break the communication into blocks to overlap computation
    // and communication
    size_t nlt = npix/blocking;
    size_t rem = npix%blocking;
    size_t nt = nlt + (rem ? 1 : 0);

    std::vector<size_t> blocksize(nt);
    std::vector<size_t> locs(nt);
    std::vector<ImageBuffer<T>*> bg(nt, NULL);
    size_t loc = 0;
    for (size_t i = 0; i < nt; ++i)
    {
        size_t bs = i < nlt ? blocking : rem;
        bg[i] = new ImageBuffer<T>(ri+loc, gi+loc, bi+loc,
             ai ? ai+loc : ai, zi ? zi+loc : zi, bs, false);
        blocksize[i] = bs;
        locs[i] = loc;
        loc += bs;
    }

#ifdef PARALLEL
#ifdef THREADED_COMPOSITER
    std::vector<ImageBuffer<T>*> fg(nt, NULL);
    std::vector<ImageBuffer<T>*> tmp(nt, NULL);
#endif

    // execute the program. in any given iteration each rank
    // is either a sender or a receiver or has nothing to do.
    size_t nit = program.size();
    for (size_t i = 0; i < nit; ++i)
    {
        // in each iteration some number of pairs comminucate
        // in each pair one process sends the otrher its data
        // a given process may receive in multiple iterations
        Iteration &it = program[i];
        size_t npairs = it.size();
        for (size_t j = 0; j < npairs; ++j)
        {
            const RankPair &rp = it[j];

            int to = receiver(rp);
            int from = sender(rp);

            if (rank == to)
            {
                for (size_t k = 0; k < nt; ++k)
                {
                    size_t bs = blocksize[k];

                    ImageBuffer<T> *fim = new ImageBuffer<T>(bs, true, ai, zi, true);

                    MPI_Recv(fim->r, bs, color_tt<T>::asmpi(), from, 12341, VISIT_MPI_COMM, MPI_STATUS_IGNORE);
                    MPI_Recv(fim->g, bs, color_tt<T>::asmpi(), from, 12342, VISIT_MPI_COMM, MPI_STATUS_IGNORE);
                    MPI_Recv(fim->b, bs, color_tt<T>::asmpi(), from, 12343, VISIT_MPI_COMM, MPI_STATUS_IGNORE);
                    if (ai)
                       MPI_Recv(fim->a, bs, color_tt<T>::asmpi(), from, 12344, VISIT_MPI_COMM, MPI_STATUS_IGNORE);
                    if (zi)
                       MPI_Recv(fim->z, bs, MPI_FLOAT, from, 12345, VISIT_MPI_COMM, MPI_STATUS_IGNORE);

                    ImageBuffer<T> *tim = new ImageBuffer<T>(bs, true, ai, zi, true);
#ifdef THREADED_COMPOSITER
                    tmgr->AddWorklet(tim, fim, bg[k]);
                    tmp[k] = tim;
                    fg[k] = fim;
#else
                    if (zi)
                        composite(tim, fim, bg[k]);
                    else
                       blend(tim, fim, bg[k]);

                    delete bg[k];
                    bg[k] = tim;
                    delete fim;
#endif
                }
#ifdef THREADED_COMPOSITER
                tmgr->WaitFinished();
                for (size_t k = 0; k < nt; ++k)
                {
                    delete fg[k];
                    fg[k] = NULL;
                    delete bg[k];
                    bg[k] = tmp[k];
                    tmp[k] = NULL;
                }
#endif
            }
            else
            if (rank == from)
            {
                for (size_t k = 0; k < nt; ++k)
                {
                    size_t bs = blocksize[k];

                    MPI_Send(bg[k]->r, bs, color_tt<T>::asmpi(), to, 12341, VISIT_MPI_COMM);
                    MPI_Send(bg[k]->g, bs, color_tt<T>::asmpi(), to, 12342, VISIT_MPI_COMM);
                    MPI_Send(bg[k]->b, bs, color_tt<T>::asmpi(), to, 12343, VISIT_MPI_COMM);
                    if (ai)
                        MPI_Send(bg[k]->a, bs, color_tt<T>::asmpi(), to, 12344, VISIT_MPI_COMM);
                    if (zi)
                        MPI_Send(bg[k]->z, bs, MPI_FLOAT, to, 12345, VISIT_MPI_COMM);

                    delete bg[k];
                    bg[k] = NULL;
                }
            }
        }
    }

    // only rank 0 needs the result
    // if the result isn't on rank 0 send it there
    int last = nit ? receiver(program.back().front()) : 0;
    if (last != 0)
    {
        if (rank == last)
        {
            for (size_t k = 0; k < nt; ++k)
            {
                size_t bs = blocksize[k];

                MPI_Send(bg[k]->r, bs, color_tt<T>::asmpi(), 0, 12341, VISIT_MPI_COMM);
                MPI_Send(bg[k]->g, bs, color_tt<T>::asmpi(), 0, 12342, VISIT_MPI_COMM);
                MPI_Send(bg[k]->b, bs, color_tt<T>::asmpi(), 0, 12343, VISIT_MPI_COMM);
                if (ai)
                   MPI_Send(bg[k]->a, bs, color_tt<T>::asmpi(), 0, 12344, VISIT_MPI_COMM);
                if (zi)
                   MPI_Send(bg[k]->z, bs, MPI_FLOAT, 0, 12345, VISIT_MPI_COMM);

                delete bg[k];
                bg[k] = NULL;
            }
        }
        else
        if (rank == 0)
        {
            for (size_t k = 0; k < nt; ++k)
            {
                size_t bs = blocksize[k];

                ImageBuffer<T> *fim = new ImageBuffer<T>(bs, true, ai, zi, true);

                MPI_Recv(fim->r, bs, color_tt<T>::asmpi(), last, 12341, VISIT_MPI_COMM, MPI_STATUS_IGNORE);
                MPI_Recv(fim->g, bs, color_tt<T>::asmpi(), last, 12342, VISIT_MPI_COMM, MPI_STATUS_IGNORE);
                MPI_Recv(fim->b, bs, color_tt<T>::asmpi(), last, 12343, VISIT_MPI_COMM, MPI_STATUS_IGNORE);
                if (ai)
                    MPI_Recv(fim->a, bs, color_tt<T>::asmpi(), last, 12344, VISIT_MPI_COMM, MPI_STATUS_IGNORE);
                if (zi)
                    MPI_Recv(fim->z, bs, MPI_FLOAT, last, 12345, VISIT_MPI_COMM, MPI_STATUS_IGNORE);

                if (!zi)
                {
                    // if we are alpha compositing rank 0 blends with background
                    size_t loc = locs[k];

                    ImageBuffer<T> *tim = new ImageBuffer<T>(bs, true, true, false, true);
                    ImageBuffer<T> *bim = rb ? new ImageBuffer<T>(rb+loc, gb+loc, bb+loc, ab+loc, NULL, bs, false)
                        : new ImageBuffer<T>(bgrgba[0], bgrgba[1], bgrgba[2], bgrgba[3], bs);

#ifdef THREADED_COMPOSITER
                    tmgr->AddWorklet(tim, fim, bim);
                    tmp[k] = tim;
                    fg[k] = fim;
                    bg[k] = bim;
#else
                    blend(tim, fim, bim);

                    delete bim;
                    delete fim;
                    bg[k] = tim;
#endif
                }
            }
#ifdef THREADED_COMPOSITER
            tmgr->WaitFinished();
            for (size_t k = 0; k < nt; ++k)
            {
                delete fg[k];
                fg[k] = NULL;
                delete bg[k];
                bg[k] = tmp[k];
                tmp[k] = NULL;
            }
#endif
        }
    }
    else if (!zi && (rank == 0))
    {
        // the result is already on rank 0, need to blend with the background
        for (size_t k = 0; k < nt; ++k)
        {
            size_t bs = blocksize[k];
            size_t loc = locs[k];

            ImageBuffer<T> *tim = new ImageBuffer<T>(bs, true, true, false, true);
            ImageBuffer<T> *bim = rb ? new ImageBuffer<T>(rb+loc, gb+loc, bb+loc, ab+loc, NULL, bs, false)
                : new ImageBuffer<T>(bgrgba[0], bgrgba[1], bgrgba[2], bgrgba[3], bs);

            blend(tim, bg[k], bim);

            delete bim;
            delete bg[k];
            bg[k] = tim;
        }
    }

    if (bcastrgba || bcastz)
    {
        // all process need the final result
        // push the result out to all
        for (size_t k = 0; k < nt; ++k)
        {
            size_t bs = blocksize[k];
            if (rank != 0)
            {
                ImageBuffer<T> *im = new ImageBuffer<T>(bs, bcastrgba, bcastrgba&&ai, bcastz&&zi, true);
                bg[k] = im;
            }
            if (bcastrgba)
            {
                MPI_Bcast(bg[k]->r, bs, color_tt<T>::asmpi(), 0, VISIT_MPI_COMM);
                MPI_Bcast(bg[k]->g, bs, color_tt<T>::asmpi(), 0, VISIT_MPI_COMM);
                MPI_Bcast(bg[k]->b, bs, color_tt<T>::asmpi(), 0, VISIT_MPI_COMM);
                if (ai)
                   MPI_Bcast(bg[k]->a, bs, color_tt<T>::asmpi(), 0, VISIT_MPI_COMM);
            }
            if (bcastz && zi)
               MPI_Bcast(bg[k]->z, bs, MPI_FLOAT, 0, VISIT_MPI_COMM);
        }
    }
#endif
    // construct the output
    if (bcastrgba || bcastz || (rank == 0))
    {
        bool color = bcastrgba || (rank == 0);
        bool depth = (bcastz || (rank == 0)) && zi;
        bool alpha = color && ai && zi;

        // copy into a user provided buffer or allocate one for him
        ImageBuffer<T> *tmp = ro||zo ? new ImageBuffer<T>(ro, go, bo, ao, zo, npix, false) :
            new ImageBuffer<T>(npix, color, alpha, depth, false);

        // if the user provided it keep his setting, else we own it until
        // he takes it
        ownout = ro||zo ? ownout : true;

        // update the output buffers (if they were allocated)
        ro = tmp->r;
        go = tmp->g;
        bo = tmp->b;
        ao = tmp->a;
        zo = tmp->z;

        // copy each block into its spot in the output buffer
        for (size_t k = 0; k < nt; ++k)
        {
            size_t bs = blocksize[k];
            size_t loc = locs[k];
            size_t nbytes = bs*sizeof(T);

            if (color)
            {
                memcpy(tmp->r+loc, bg[k]->r, nbytes);
                memcpy(tmp->g+loc, bg[k]->g, nbytes);
                memcpy(tmp->b+loc, bg[k]->b, nbytes);
                if (alpha)
                    memcpy(tmp->a+loc, bg[k]->a, nbytes);
            }
            if (depth)
                memcpy(tmp->z+loc, bg[k]->z, bs*sizeof(float));

            delete bg[k];
            bg[k] = NULL;
        }

        delete tmp;
    }
}



// ****************************************************************************
//  Helper Functions:
//
//      Split -- split an avtImage into r,g,b,a,z arrays
//      Merge -- create an avtImage from r,g,b,a,z arrays
//      Insert -- zero-copy package r,g,b,a,z into an avtImage in a
//              way that is incompatible with VisIt.
//      Extract -- un-package r,g,b,a,z arrays from an avtImage
//              that were packed with Insert method
//      Free -- free/delete the buffer arrays
//
//  Purpose:
//      These functions provide support for converting to/from efficient
//      internal representations to VisIt's native represnetations.
//
//
//  Programmer: Burlen Loring
//  Creation:   Tue Sep  1 09:34:49 PDT 2015
//
// ****************************************************************************

// ----------------------------------------------------------------------------
// given an avt image img split it into r,g,b,a,z arrays
// return these and the size w,h.
template <typename T>
int Split(T *&r, T *&g, T *&b, T *&a,
    float *&z, int &w, int &h, int nchano,
    avtImage_p img)
{
    vtkImageData *im = img->GetImage().GetImageVTK();
    if (!im)
        return -1;

    int dims[3];
    im->GetDimensions(dims);
    w = dims[0];
    h = dims[1];
    size_t npix = w*h;

    vtkUnsignedCharArray *rgba =
        dynamic_cast<vtkUnsignedCharArray*>(
            im->GetPointData()->GetArray("ImageScalars"));

    // a VisIt compatible image, allocate a buffers and split
    // and potentially convert
    int nchani = rgba?rgba->GetNumberOfComponents():0;

    vtkFloatArray *zb = img->GetImage().GetZBufferVTK();

    ImageBuffer<T> *tmp = new ImageBuffer<T>(rgba?rgba->GetPointer(0):NULL,
        zb?zb->GetPointer(0):NULL, npix, nchani, nchano, false, false);

    // caller takes the buffers, and must free them later
    r = tmp->r;
    g = tmp->g;
    b = tmp->b;
    a = tmp->a;
    z = tmp->z;

    delete tmp;

    return 0;
}

// ----------------------------------------------------------------------------
// merge r,g,b,a,z arrays into a VisIt compatible data
// structure. this involves copying the data. if take is
// set then we take ownership of the passed in arrays
template <typename T>
void Merge(avtImage_p img, T *r, T *g, T *b, T *a, float *z,
    int w, int h, bool take)
{
    int nchan = z&&a ? 4 : 3;
    size_t npix = w*h;

    vtkImageData *rgb = NULL;
    if (r)
    {
        // the VisIt compatible route, make a copy
        rgb = avtImageRepresentation::NewImage(w, h, nchan);

        unsigned char *prgb
            = static_cast<unsigned char *>(rgb->GetScalarPointer(0, 0, 0));

        if (nchan == 4)
        {
            merge<T,4>(prgb, r, npix, 0);
            merge<T,4>(prgb, g, npix, 1);
            merge<T,4>(prgb, b, npix, 2);
            if (a&&z)
                merge<T,4>(prgb, a, npix, 3);
        }
        else
        {
            merge<T,3>(prgb, r, npix, 0);
            merge<T,3>(prgb, g, npix, 1);
            merge<T,3>(prgb, b, npix, 2);
            if (a&&z)
                merge<T,3>(prgb, a, npix, 3);
        }
    }
    else
    {
        rgb = vtkImageData::New();
        rgb->SetDimensions(w,h,1);
    }

    // caller wants us to manage memory. and we no longer need these
    if (take)
    {
        free(r);
        free(g);
        free(b);
        free(a);
    }

    // pass in scalars and z-buffer, image takes ownership of z-buffer
    img->SetImage(avtImageRepresentation(rgb,z,take?1:0));
    rgb->Delete();
}

// ----------------------------------------------------------------------------
// zero-copy insert the r,g,b,a,z arrays into the avt image.
// this is not strictly compatible with VisIt
template <typename T>
void Insert(avtImage_p img, T *r, T *g, T *b, T *a, float *z,
    int w, int h, bool take)
{
    vtkImageData *im = vtkImageData::New();
    im->SetDimensions(w,h,1);

    size_t npix = w*h;

    AddPointData(im, "r", r, npix, take?0:1, 0);
    AddPointData(im, "g", g, npix, take?0:1, 0);
    AddPointData(im, "b", b, npix, take?0:1, 0);
    AddPointData(im, "a", a, npix, take?0:1, 0);
    AddPointData(im, "z", z, npix, take?0:1, 1);

    img->SetImage(avtImageRepresentation(im, NULL, 0));
    im->Delete();

    return img;
}

// ----------------------------------------------------------------------------
// zero-copy out r,g,b,a,z arrays from an avt image that has
// been packaged with Insert call.
template <typename T>
int Extract(T *&r, T *&g, T *&b, T *&a,
    float *&z, int &w, int &h, avtImage_p img)
{
    vtkImageData *im = vtkImageData::New();
    im->SetDimensions(w, h, 1);

    // if it's not a VisIt compatible image, try to zero-copy the channels
    //TODO --  we could also handle conversion of types, as we did for VisIt compatible
    typename vtk_tt<T>::array_t *c = NULL;
    if ((c = dynamic_cast<typename vtk_tt<T>::array_t*>(im->GetPointData()->GetArray("r"))))
        r = c->GetPointer(0);
    if ((c = dynamic_cast<typename vtk_tt<T>::array_t*>(im->GetPointData()->GetArray("g"))))
        g = c->GetPointer(0);
    if ((c = dynamic_cast<typename vtk_tt<T>::array_t*>(im->GetPointData()->GetArray("b"))))
        b = c->GetPointer(0);
    if ((c = dynamic_cast<typename vtk_tt<T>::array_t*>(im->GetPointData()->GetArray("a"))))
        a = c->GetPointer(0);

    vtkFloatArray *zi = NULL;
    if ((zi = dynamic_cast<vtkFloatArray*>(im->GetPointData()->GetArray("z"))))
        z = zi->GetPointer(0);

    // we're OK if we at least have r,g, and b
    return (r && g && b) ? 0 : -1;
}

// ----------------------------------------------------------------------------
template <typename T>
void Free(T *r, T *g, T *b, T *a, float *z)
{
    free(r);
    free(g);
    free(b);
    free(a);
    delete [] z;
}

#endif
