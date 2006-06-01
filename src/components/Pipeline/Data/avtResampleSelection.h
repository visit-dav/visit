/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                        avtResampleSelection.h                             //
// ************************************************************************* //

#ifndef AVT_RESAMPLE_SELECTION_H
#define AVT_RESAMPLE_SELECTION_H 
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtResampleSelection
//
//  Purpose: Specify data selection by a uniform resampling. The context in
//  which resampling behaves as a data "selection" operation is one in which
//  the input is a non-discrete (e.g. continuous) representation of the data.
//  In that case, resampling has the effect of selecting a finite set of 
//  points (the samples) fron an infinity of points (the continuum) in which
//  the data actually originates.
//
//  The default is a uniform sampling 100 samples in each of X, Y and Z
//  between -10.0 and 10.0 on each axis.
//
//  To specify samples on an orthogonal slice on the Y-axis, for example,
//  you would specify starts[1] = stops[1] = Y-index and counts[1] = 0
//  with other values as desired.
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005 
//
// ****************************************************************************

class PIPELINE_API avtResampleSelection : public avtDataSelection 
{
  public:
                            avtResampleSelection();
    virtual                ~avtResampleSelection() {} ;

    virtual const char *    GetType() const
                                { return "Resample Data Selection"; }; 

    void                    SetNDims(int _ndims)
                                { ndims = _ndims; };
    void                    SetStarts(const double *_starts);
    void                    SetStops(const double *_stops);
    void                    SetCounts(const int *_counts);

    void                    GetNDims(int &_ndims) const
                                { _ndims = ndims; } ; 
    void                    GetStarts(double *_starts) const;
    void                    GetStops(double *_stops) const;
    void                    GetCounts(int *_counts) const;

    bool                    operator==(const avtResampleSelection &) const;

  private:

     int ndims;
     double starts[3];
     double stops[3];
     int counts[3];

};

typedef ref_ptr<avtResampleSelection> avtResampleSelection_p;


#endif
