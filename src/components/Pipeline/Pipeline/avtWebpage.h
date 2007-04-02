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

#ifndef AVT_WEBPAGE_H
#define AVT_WEBPAGE_H

#include <pipeline_exports.h>
#include <visitstream.h>


// ****************************************************************************
// Class: avtWebpage
//
// Purpose:
//     Creates webpages that are custom to AVT.
//
// Programmer: Hank Childs
// Creation:   December 21, 2006
//
// ****************************************************************************

class PIPELINE_API avtWebpage
{
  public:
                  avtWebpage(const char *filename);
    virtual      ~avtWebpage();
    
    void          InitializePage(const char *heading);
    void          WriteTitle(const char *title);
    void          FinalizePage(void);

    void          AddLink(const char *, const char *);

    void          AddHeading(const char *);
    void          AddSubheading(const char *);
    void          AddEntry(const char *);
    void          AddSubentry(const char *);
    void          StartTable(void);
    void          AddTableHeader2(const char *, const char *);
    void          AddTableEntry2(const char *, const char *);
    void          AddTableHeader3(const char *, const char *, const char *);
    void          AddTableEntry3(const char *, const char *, const char *);
    void          AddTableHeader4(const char *, const char *, const char *,
                                  const char *);
    void          AddTableEntry4(const char *, const char *, const char *,
                                 const char *);
    void          EndTable(void);

  protected:
    ofstream     *ofile;
};


#endif


