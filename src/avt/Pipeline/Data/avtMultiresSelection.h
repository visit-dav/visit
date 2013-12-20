/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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
//                         avtMultiresSelection.h                            //
// ************************************************************************* //

#ifndef AVT_MULTIRES_SELECTION_H
#define AVT_MULTIRES_SELECTION_H 
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtMultiresSelection
//
//  Purpose:
//    Specify a data selection with a multi resolution representation
//    consisting of a desired view frustum and a desired cell size. The
//    data selection also contains the actual view frustum and cell size
//    provided.
//
//  Programmer: Eric Brugger
//  Creation:   December 20, 2013 
//
//  Modifications:
//
// ****************************************************************************

class PIPELINE_API avtMultiresSelection : public avtDataSelection 
{
  public:
                            avtMultiresSelection() {} ;
    virtual                ~avtMultiresSelection() {} ;

    virtual const char *    GetType() const
                                { return "Multi Resolution Data Selection"; }; 
    virtual std::string     DescriptionString(void)
                                { char str[1024];
                                  sprintf(str, "avtMultiresSelection:%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f",
                                      desiredFrustum[0], desiredFrustum[1],
                                      desiredFrustum[2], desiredFrustum[3],
                                      desiredFrustum[4], desiredFrustum[5],
                                      actualFrustum[0], actualFrustum[1],
                                      actualFrustum[2], actualFrustum[3],
                                      actualFrustum[4], actualFrustum[5],
                                      desiredCellSize, actualCellSize); };

    void                    SetDesiredFrustum(const double frust[6])
                                { desiredFrustum[0] = frust[0];
                                  desiredFrustum[1] = frust[1];
                                  desiredFrustum[2] = frust[2];
                                  desiredFrustum[3] = frust[3];
                                  desiredFrustum[4] = frust[4];
                                  desiredFrustum[5] = frust[5]; };
    void                    GetDesiredFrustum(double frust[6]) const
                                { frust[0] = desiredFrustum[0];
                                  frust[1] = desiredFrustum[1];
                                  frust[2] = desiredFrustum[2];
                                  frust[3] = desiredFrustum[3];
                                  frust[4] = desiredFrustum[4];
                                  frust[5] = desiredFrustum[5]; };
    void                    SetActualFrustum(const double frust[6])
                                { actualFrustum[0] = frust[0];
                                  actualFrustum[1] = frust[1];
                                  actualFrustum[2] = frust[2];
                                  actualFrustum[3] = frust[3];
                                  actualFrustum[4] = frust[4];
                                  actualFrustum[5] = frust[5]; };
    void                    GetActualFrustum(double frust[6]) const
                                { frust[0] = actualFrustum[0];
                                  frust[1] = actualFrustum[1];
                                  frust[2] = actualFrustum[2];
                                  frust[3] = actualFrustum[3];
                                  frust[4] = actualFrustum[4];
                                  frust[5] = actualFrustum[5]; };
    void                    SetDesiredCellSize(double size)
                                { desiredCellSize = size; };
    double                  GetDesiredCellSize() const
                                { return desiredCellSize; };
    void                    SetActualCellSize(double size)
                                { actualCellSize = size; };
    double                  GetActualCellSize() const
                                { return actualCellSize; };

  private:
    double desiredFrustum[6];
    double actualFrustum[6];
    double desiredCellSize;
    double actualCellSize;
};

typedef ref_ptr<avtMultiresSelection> avtMultiresSelectiont_p;

#endif
