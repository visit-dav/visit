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
class  avtWebpage;


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
//     Hank Childs, Thu Jun 21 11:19:02 PDT 2007
//     Add DebugDump.
//
// ****************************************************************************

class PIPELINE_API  avtSamplePoints : public avtDataObject
{
  public:
                                avtSamplePoints(avtDataObjectSource *);
    virtual                    ~avtSamplePoints();

    void                        SetNumberOfVariables(std::vector<int> &, 
                                                   std::vector<std::string> &);
    int                         GetNumberOfVariables(void);
    int                         GetNumberOfRealVariables(void) 
                                                   { return varnames.size(); };
    const std::string          &GetVariableName(int);
    int                         GetVariableSize(int);
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

    virtual void                DebugDump(avtWebpage *, const char *);

  protected:
    avtVolume                  *volume;
    avtCellList                *celllist;
    std::vector<std::string>    varnames;
    std::vector<int>            varsize;
    bool                        useWeightingScheme;
};


typedef ref_ptr<avtSamplePoints>  avtSamplePoints_p;


#endif


