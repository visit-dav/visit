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
//                           avtDataObjectString.h                           //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_STRING_H
#define AVT_DATA_OBJECT_STRING_H

#include <pipeline_exports.h>


#include <vector>


// ****************************************************************************
//  Class: avtDataObjectString
//
//  Purpose:
//      A string that a data object can be written to.  This class is not much 
//      more than the STL version (with significantly restricted functionality).
//      It exists because the assembly of the data object string became a
//      serious bottleneck due to the append method.  Re-writing the routines
//      without the append is not worthwhile, so having a class that is less 
//      append-sensitive seems like the best solution.
//
//  Programmer: Hank Childs
//  Creation:   May 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Sep 16 16:04:39 PDT 2001
//    Re-worked to prevent copying of strings.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectString
{
  public:
                              avtDataObjectString();
    virtual                  ~avtDataObjectString();

    typedef enum
    {
        DATA_OBJECT_STRING_DOES_NOT_OWN_REFERENCE,
        DATA_OBJECT_STRING_OWNS_REFERENCE_AFTER_CALL,
        DATA_OBJECT_STRING_SHOULD_MAKE_COPY
    } APPEND_ACTION_ITEM;
        
    void                      Append(char *, int, APPEND_ACTION_ITEM);

    int                       GetNStrings(void);
    void                      GetString(int, char *&, int &);
    int                       GetTotalLength(void);

    void                      GetWholeString(char *&, int &);

  protected:
    std::vector<char *>       strs;
    std::vector<int>          lens;
    std::vector<bool>         ownString;
    char                     *wholeString;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtDataObjectString(const avtDataObjectString &) {;};
    avtDataObjectString &operator=(const avtDataObjectString &) 
                                                            { return *this; };
};


#endif


