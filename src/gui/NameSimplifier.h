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

#ifndef NAME_SIMPLIFIER_H
#define NAME_SIMPLIFIER_H
#include <gui_exports.h>
#include <QualifiedFilename.h>
#include <vectortypes.h>

// ****************************************************************************
// Class: NameSimplifier
//
// Purpose:
//   This is a simple class that takes a group of host qualified file names
//   and removes the bits that it can to make shorter names.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 4 14:22:58 PST 2004
//
// Modifications:
//    Jeremy Meredith, Tue Apr  6 14:16:54 PDT 2004
//    Keep track of the filenames in a way made for adding the minimal amount
//    of text needed for ensuring they are unique.  Added UniqueFileName.
//
//    Jeremy Meredith, Wed Apr  7 12:12:48 PDT 2004
//    Account for the fact that not all input names had full paths.
//
// ****************************************************************************

class GUI_API NameSimplifier
{
  private:
    // Class: UniqueFileName
    //
    // Purpose:
    //    Structure to add a minimal amount of path/host information
    //    to a filename to ensure it is unique w.r.t. other filenames
    class UniqueFileName
    {
      public:
        UniqueFileName(const QualifiedFilename &qfn);
        std::string GetAsString() const;

        static bool Unique(const UniqueFileName &a, const UniqueFileName &b);
        static void Uniquify(UniqueFileName &a, UniqueFileName &b);
      private:
        std::string  host;        // host name
        stringVector path;        // path components, split by separator
        int          pathLen;     // length of the 'path' vector
        std::string  file;        // raw file name without path
        bool         leadingSlash;// true if it was a full path originally
        char         separator;   // separator character

        bool         useHost;     // true if we need the host for uniqueness
        int          pathCount;   // number of needed path segment prefixes
    };

  public:
    NameSimplifier();
    ~NameSimplifier();

    void AddName(const std::string &n);
    void AddName(const QualifiedFilename &n);
    void ClearNames();
    void GetSimplifiedNames(stringVector &n) const;
private:
    std::vector<UniqueFileName> names;
};

#endif
