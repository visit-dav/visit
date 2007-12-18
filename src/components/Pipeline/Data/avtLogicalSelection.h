/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                        avtLogicalSelection.h                              //
// ************************************************************************* //

#ifndef AVT_LOGICAL_SELECTION_H
#define AVT_LOGICAL_SELECTION_H 
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtLogicalSelection
//
//  Purpose: Specify data selection by a zone-centric logical indexing scheme.
//  That is, all logical coordinates are specified in terms of zones, not
//  nodes. The starts and stops are inclusive. However, if strides are such
//  that an integral number of strides from starts DOES NOT equal the
//  associated value in stops, then stops is non-inclusive.
//
//  The default is a 3D box of stride 1 spanning all possible indices.
//
//  To specify an orthogonal slice on the Y-axis, for example, you would
//  specify starts[1] = stops[1] = Y-index and strides[1] = 0 with other
//  values as desired.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtLogicalSelection : public avtDataSelection 
{
  public:
                            avtLogicalSelection();
    virtual                ~avtLogicalSelection() {} ;

    virtual const char *    GetType() const
                                { return "Logical Data Selection"; }; 

    void                    SetNDims(int _ndims)
                                { ndims = _ndims; };
    void                    SetStarts(const int *_starts);
    void                    SetStops(const int *_stops);
    void                    SetStrides(const int *_strides);

    void                    GetNDims(int &_ndims) const
                                { _ndims = ndims; } ; 
    void                    GetStarts(int *_starts) const;
    void                    GetStops(int *_stops) const;
    void                    GetStrides(int *_strides) const;

    bool                    operator==(const avtLogicalSelection &) const;

    void                    Compose(const avtLogicalSelection& sel);
                                          
    bool                    FactorBestPowerOf2(avtLogicalSelection& pow2Sel,
                                               avtLogicalSelection& otherSel) const;

  private:

     int ndims;
     int starts[3];
     int stops[3];
     int strides[3];

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtLogicalSelection(const avtLogicalSelection &) {;};
    avtLogicalSelection &operator=(const avtLogicalSelection &) 
                                                            { return *this; };
};

typedef ref_ptr<avtLogicalSelection> avtLogicalSelection_p;


#endif


