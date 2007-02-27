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
//                                avtWebpage.C                               //
// ************************************************************************* //

#include <avtWebpage.h>


// ****************************************************************************
//  Method: avtWebpage constructor
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

avtWebpage::avtWebpage(const char *filename)
{
    ofile = new ofstream(filename);
}


// ****************************************************************************
//  Method: avtWebpage destructor
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

avtWebpage::~avtWebpage()
{
    if (ofile != NULL)
        delete ofile;
}


// ****************************************************************************
//  Method: avtWebpage::InitializePage
//
//  Purpose:
//      Initializes the webpage.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::InitializePage(const char *heading)
{
    *ofile << "<!doctype html public \"-//w3c//dtd html 4.0"
           << " transitional//en\">" << endl;
    *ofile << "<html>" << endl;
    *ofile << "<head>" << endl;
    *ofile << "   <title>" << heading << "</title>" << endl;
    *ofile << "</head>" << endl;
    *ofile << "<body>" << endl;
}


// ****************************************************************************
//  Method: avtWebpage::WriteTitle
//
//  Purpose:
//      Writes the title.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::WriteTitle(const char *title)
{
    *ofile << "<center><b><font size=\"10\">" << title << "</font>"
             << "</b></center>" << endl;
}


// ****************************************************************************
//  Method: avtWebpage::FinalizePage
//
//  Purpose:
//      Finishes the webpage.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::FinalizePage(void)
{
    *ofile << "</body>" << endl;
    *ofile << "</html>" << endl;
    ofile->close();
}


// ****************************************************************************
//  Method: avtWebpage::AddLink
//
//  Purpose:
//      Adds a link to another webpage.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddLink(const char *file, const char *listing)
{
    *ofile << "<p><a href=\"" << file << "\">" << listing << "</a></p>" 
           << endl;
}


// ****************************************************************************
//  Method: avtWebpage::AddHeading
//
//  Purpose:
//      Adds a heading.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddHeading(const char *heading)
{
    *ofile << "<p><b><font size=\"8\">" << heading << "</font></b></p>" 
             << endl;
}


// ****************************************************************************
//  Method: avtWebpage::AddSubheading
//
//  Purpose:
//      Adds a sub-heading.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddSubheading(const char *heading)
{
    *ofile << "<p><b><font size=\"6\">" << heading << "</font></b></p>" 
             << endl;
}


// ****************************************************************************
//  Method: avtWebpage::AddEntry
//
//  Purpose:
//      Adds an entry to the webpage.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddEntry(const char *heading)
{
    *ofile << "<p><b><font size=\"4\">" << heading << "</font></b></p>" 
             << endl;
}


// ****************************************************************************
//  Method: avtWebpage::StartTable
//
//  Purpose:
//      Starts a table.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::StartTable(void)
{
    *ofile << "<table border>" << endl;
}


// ****************************************************************************
//  Method: avtWebpage::EndTable
//
//  Purpose:
//      Ends a table.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::EndTable(void)
{
    *ofile << "</table>" << endl;
}


// ****************************************************************************
//  Method: avtWebpage::AddTableEntry2
//
//  Purpose:
//      Adds a table entry with two columns
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddTableEntry2(const char *a1, const char *a2)
{
    const char *b1 = (a1 != NULL ? a1 : "");
    const char *b2 = (a2 != NULL ? a2 : "");
    *ofile << "<tr><td>" << b1 << "</td><td>" << b2 << "</td>" << endl;
}


// ****************************************************************************
//  Method: avtWebpage::AddTableEntry3
//
//  Purpose:
//      Adds a table entry with three columns
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddTableEntry3(const char *a1, const char *a2, const char *a3)
{
    const char *b1 = (a1 != NULL ? a1 : "");
    const char *b2 = (a2 != NULL ? a2 : "");
    const char *b3 = (a3 != NULL ? a3 : "");
    *ofile << "<tr><td>" << b1 << "</td><td>" << b2 << "</td><td>" << b3
           << "</td>" << endl;
}


// ****************************************************************************
//  Method: avtWebpage::AddTableEntry4
//
//  Purpose:
//      Adds a table entry with four columns
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddTableEntry4(const char *a1, const char *a2, const char *a3,
                           const char *a4)
{
    const char *b1 = (a1 != NULL ? a1 : "");
    const char *b2 = (a2 != NULL ? a2 : "");
    const char *b3 = (a3 != NULL ? a3 : "");
    const char *b4 = (a4 != NULL ? a4 : "");
    *ofile << "<tr><td>" << b1 << "</td><td>" << b2 << "</td><td>" << b3
           << "</td><td>" << b4 << "</td>" << endl;
}


// ****************************************************************************
//  Method: avtWebpage::AddTableHeader2
//
//  Purpose:
//      Adds a table header with two columns
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddTableHeader2(const char *a1, const char *a2)
{
    const char *b1 = (a1 != NULL ? a1 : "");
    const char *b2 = (a2 != NULL ? a2 : "");
    *ofile << "<tr><td><b><i>" << b1 << "</b></i></td><td><b><i>" 
           << b2 << "</b></i></td>" << endl;
}

// ****************************************************************************
//  Method: avtWebpage::AddTableHeader3
//
//  Purpose:
//      Adds a table header with three columns
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddTableHeader3(const char *a1, const char *a2, const char *a3)
{
    const char *b1 = (a1 != NULL ? a1 : "");
    const char *b2 = (a2 != NULL ? a2 : "");
    const char *b3 = (a3 != NULL ? a3 : "");
    *ofile << "<tr><td><b><i>" << b1 << "</b></i></td><td><b><i>" 
           << b2 << "</b></i></td><td><b><i>" << b3
           << "</b></i></td>" << endl;
}

// ****************************************************************************
//  Method: avtWebpage::AddTableHeader4
//
//  Purpose:
//      Adds a table header with four columns
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtWebpage::AddTableHeader4(const char *a1, const char *a2, const char *a3,
                           const char *a4)
{
    const char *b1 = (a1 != NULL ? a1 : "");
    const char *b2 = (a2 != NULL ? a2 : "");
    const char *b3 = (a3 != NULL ? a3 : "");
    const char *b4 = (a4 != NULL ? a4 : "");
    *ofile << "<tr><td><b><i>" << b1 << "</b></i></td><td><b><i>" 
           << b2 << "</b></i></td><td><b><i>" << b3
           << "</b></i></td><td><b><i>" <<  b4 << "</b></i></td>" << endl;
}


