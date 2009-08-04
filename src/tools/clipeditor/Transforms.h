/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

// ----------------------------------------------------------------------------
// File:  Transforms.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
//
// Modifications:
//    Jeremy Meredith, Thu Sep 18 11:28:24 PDT 2003
//    Added quad and triangle transforms.
//
//    Jeremy Meredith, Thu Jun 24 10:38:05 PDT 2004
//    Added voxel and pixel transform.
//
//    Jeremy Meredith, Thu Jun  7 13:21:32 EDT 2007
//    Added line transform.
//
// ----------------------------------------------------------------------------

#ifndef TRANSFORMS_H
#define TRANSFORMS_H

struct HexTransform
{
    int  n[8];
    char e[12];
    bool f;
};

struct VoxTransform
{
    int  n[8];
    char e[12];
    bool f;
};

struct WedgeTransform
{
    int  n[6];
    char e[9];
    bool f;
};

struct PyramidTransform
{
    int  n[5];
    char e[8];
    bool f;
};

struct TetTransform
{
    int  n[4];
    char e[6];
    bool f;
};

struct QuadTransform
{
    int  n[4];
    char e[4];
    bool f;
};

struct PixelTransform
{
    int  n[4];
    char e[4];
    bool f;
};

struct TriTransform
{
    int  n[3];
    char e[3];
    bool f;
};

struct LineTransform
{
    int  n[2];
    char e[2];
    bool f;
};

extern HexTransform     hexTransforms[48];
extern VoxTransform     voxTransforms[48];
extern WedgeTransform   wedgeTransforms[12];
extern PyramidTransform pyramidTransforms[8];
extern TetTransform     tetTransforms[24];
extern QuadTransform    quadTransforms[8];
extern PixelTransform   pixelTransforms[8];
extern TriTransform     triTransforms[6];
extern LineTransform    lineTransforms[2];

#endif
