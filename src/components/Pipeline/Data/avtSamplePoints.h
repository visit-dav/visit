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
//                             avtSamplePoints.h                             //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_H
#define AVT_SAMPLE_POINTS_H
#include <pipeline_exports.h>


#include <ref_ptr.h>

#include <avtDataObject.h>
#include <avtCellList.h>


class  avtSamplePointsSource;
class  avtVolume;


// ****************************************************************************
//  Class: avtSamplePoints
//
//  Purpose:
//      Contains samples, whether they be sample points or cells that will
//      later be converted into sample points.  This is an example of a data
//      object in the avt pipeline.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//     Hank Childs, Sat Jan 27 16:28:44 PST 2001
//     Modified the concept of sample points to be either cells or a volume.
//
//     Hank Childs, Mon Jun  4 08:31:22 PDT 2001
//     Inherited from avtDataObject.
//
//     Hank Childs, Sun Dec  4 19:16:23 PST 2005
//     Add [Get|Set]UseWeightingScheme.
//
// ****************************************************************************

class PIPELINE_API  avtSamplePoints : public avtDataObject
{
  public:
                                avtSamplePoints(avtDataObjectSource *);
    virtual                    ~avtSamplePoints();

    void                        SetNumberOfVariables(int);
    int                         GetNumberOfVariables(void) { return numVars; };
    virtual int                 GetNumberOfCells(bool polysOnly = false) const
                                   { return celllist->GetNumberOfCells(); };
    void                        SetVolume(int, int, int);
    avtVolume                  *GetVolume(void)  { return volume; };

    avtCellList                *GetCellList(void);
    void                        ResetCellList(void);

    virtual const char         *GetType(void)  { return "avtSamplePoints"; };

    void                        SetUseWeightingScheme(bool b)
                                    { useWeightingScheme = b; };
    bool                        GetUseWeightingScheme(void)
                                    { return useWeightingScheme; };

  protected:
    avtVolume                  *volume;
    avtCellList                *celllist;
    int                         numVars;
    bool                        useWeightingScheme;
};


typedef ref_ptr<avtSamplePoints>  avtSamplePoints_p;


#endif


