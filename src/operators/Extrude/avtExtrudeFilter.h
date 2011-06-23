/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//  File: avtExtrudeFilter.h
// ************************************************************************* //

#ifndef AVT_EXTRUDE_FILTER_H
#define AVT_EXTRUDE_FILTER_H


#include <avtPluginDataTreeIterator.h>
#include <ExtrudeAttributes.h>

class vtkDataSet;
class vtkPoints;
class vtkPointSet;

// ****************************************************************************
//  Class: avtExtrudeFilter
//
//  Purpose:
//      A plugin operator for Extrude.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
//  Modifications:
//
// ****************************************************************************

class avtExtrudeFilter : public avtPluginDataTreeIterator
{
  public:
                         avtExtrudeFilter();
    virtual             ~avtExtrudeFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtExtrudeFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Extrude"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    ExtrudeAttributes   atts;

    virtual vtkDataSet   *ExecuteData(vtkDataSet *, int, std::string);

    virtual void          UpdateDataObjectInfo(void);

    void                  CopyVariables(vtkDataSet *in_ds, vtkDataSet *out_ds, 
                                        int nSteps, const int *cellReplication = NULL) const;
    vtkPoints            *CreateExtrudedPoints(vtkPoints *oldPoints, 
                                               int nSteps) const;
    void                  ExtrudeExtents(double *dbounds) const;
    vtkDataSet           *ExtrudeToRectilinearGrid(vtkDataSet *in_ds) const;
    vtkDataSet           *ExtrudeToStructuredGrid(vtkDataSet *in_ds) const;
    vtkDataSet           *ExtrudeToUnStructuredGrid(vtkPointSet *in_ds) const;
};


#endif
