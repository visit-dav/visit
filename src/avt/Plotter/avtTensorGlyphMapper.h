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
//                            avtTensorGlyphMapper.h                         //
// ************************************************************************* //

#ifndef AVT_TENSOR_GLYPH_MAPPER_H
#define AVT_TENSOR_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtMapper.h>

class     vtkVisItTensorGlyph;
class     vtkAlgorithmOutput;
class     vtkLookupTable;
class     vtkPolyData;
class     vtkVisItPolyDataNormals;


// ****************************************************************************
//  Class: avtTensorGlyphMapper
//
//  Purpose:
//      A mapper for tensor.  This extends the functionality of a mapper by
//      glyphing tensors onto ellipsoids.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Wed May  5 16:23:29 PDT 2004
//    Add normals calculation.
//
//    Eric Brugger, Wed Nov 24 12:58:22 PST 2004
//    Added scaleByMagnitude and autoScale.
//
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005 
//    Use VisIt's version of TensorGlyph so that orignal zone and node
//    arrays can be copied through.
//
//    Kathleen Biagas, Wed Feb 6 19:38:27 PST 2013
//    Changed signature of InsertFilters.
//
//    Kathleen Biagas, Thu Feb 7 08:45:03 PST 2013
//    Changed signature of constructor to accept vtkAlgorithmInput in order
//    to preserve pipeline connections with vtk-6.
//
//    Kathleen Biagas, Thu Mar 14 13:03:50 PDT 2013
//    Remove normalsFilter.
//
// ****************************************************************************

class PLOTTER_API  avtTensorGlyphMapper : public avtMapper
{
  public:
                               avtTensorGlyphMapper(vtkAlgorithmOutput *);
    virtual                   ~avtTensorGlyphMapper();

    void                       ColorByMagOn(void);
    void                       ColorByMagOff(const unsigned char [3]);
    void                       SetScale(double);
    void                       SetScaleByMagnitude(bool);
    void                       SetAutoScale(bool);
    void                       SetLookupTable(vtkLookupTable *lut);

  protected:
    vtkAlgorithmOutput        *glyph;
    vtkLookupTable            *lut;
    bool                       colorByMag;
    unsigned char              glyphColor[3];
    double                     scale;
    bool                       scaleByMagnitude;
    bool                       autoScale;

    vtkVisItTensorGlyph      **tensorFilter;
    int                        nTensorFilters;

    virtual void               CustomizeMappers(void);

    virtual vtkAlgorithmOutput *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);
};


#endif


