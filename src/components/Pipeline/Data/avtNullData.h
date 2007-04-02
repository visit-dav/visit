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
//                             avtNullData.h                                 //
// ************************************************************************* //

#ifndef AVT_NULL_DATA_H
#define AVT_NULL_DATA_H
#include <string>
#include <pipeline_exports.h>
#include <ref_ptr.h>

#include <avtDataObject.h>

class avtDataObjectWriter;

const char * const AVT_NULL_IMAGE_MSG 
                              = "avtImage withheld; under scalable threshold";
const char * const AVT_NULL_DATASET_MSG 
                              = "avtDataset withheld; over scalable threshold";

// ****************************************************************************
//  Class: avtNullData
//
//  Purpose:
//      This is another data object that avt pipelines can handle. It
//      represents no data.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 7, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Mark C. Miller, Wed Feb  4 19:47:30 PST 2004
//    Added Instance & DerivedCopy, needed by avtDataObject->Clone
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Added method SetWriterShouldMergeParallelStreams and assoc. data member 
//
// ****************************************************************************

class PIPELINE_API avtNullData : public avtDataObject
{
  public:
                              avtNullData(avtDataObjectSource *src, const char *_typeStr = NULL)
                                 : avtDataObject(src)
                                 { typeStr = _typeStr ? _typeStr : "avtNullData";
                                   writerShouldMergeParallelStreams = false; };
    virtual                  ~avtNullData();

    virtual const char       *GetType(void)  { return typeStr.c_str(); };
    virtual int               GetNumberOfCells(bool polysOnly = false) const { return 0;};
    virtual void              SetType(char *_typeStr )  { typeStr = _typeStr; };
    void                      SetWriterShouldMergeParallelStreams()
                                  { writerShouldMergeParallelStreams = true; };
    avtDataObject            *Instance(void);
    avtDataObjectWriter      *InstantiateWriter();

  protected:
    virtual void              DerivedCopy(avtDataObject *) {;};
    virtual void              DerivedMerge(avtDataObject *dob) {;};

  private:
     std::string              typeStr;
     bool                     writerShouldMergeParallelStreams;
};

typedef ref_ptr<avtNullData>  avtNullData_p;

#endif
