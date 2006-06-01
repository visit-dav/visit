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
//                             avtDataObjectReader.h                         //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_READER_H
#define AVT_DATA_OBJECT_READER_H

#include <pipeline_exports.h>

#include <string>
#include <ref_ptr.h>

#include <avtDataSetReader.h>
#include <avtImageReader.h>
#include <avtNullDataReader.h>


// ****************************************************************************
//  Class: avtDataObjectReader
//
//  Purpose:
//      A front end that finds the appropriate image or dataset reader.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectReader
{
  public:
                                 avtDataObjectReader();
    virtual                     ~avtDataObjectReader();

    void                         Read(int size, char *input);

    bool                         InputIsDataset(void);
    bool                         InputIsImage(void);
    bool                         InputIsNullData(void);
    bool                         InputIs(const char *);

    avtDataObject_p              GetOutput(void);
    avtImage_p                   GetImageOutput(void);
    avtDataset_p                 GetDatasetOutput(void);
    avtNullData_p                GetNullDataOutput(void);

    avtDataObjectInformation    &GetInfo(void);

  protected:
    bool                         haveInput;

    bool                         datasetInput;
    bool                         imageInput;
    bool                         nullDataInput;

    avtDataSetReader_p           datasetReader;
    avtImageReader_p             imageReader;
    avtNullDataReader_p          nullDataReader;

  private:
    std::string                  inputTypeStr;
};


typedef  ref_ptr<avtDataObjectReader> avtDataObjectReader_p;


#endif


