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

#ifndef QUALIFIED_FILENAME_H
#define QUALIFIED_FILENAME_H
#include <string>
#include <vector>

// ****************************************************************************
// Class: QualifiedFilename
//
// Purpose:
//   This class contains filename information that includes: host,
//   path, filename. It should be used anywhere a filename is needed.
//
// Notes:      
//   This class was created to deprecate the usage of strings as
//   filenames. This is because it was often unclear how the file
//   information was stored in the string. This way, we always know
//   what is in a filename.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:31:16 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 26 17:04:06 PST 2002
//   I added a separator.
//
//   Brad Whitlock, Fri Mar 28 12:01:51 PDT 2003
//   I added a flag that contains the file's access and virtual bits.
//
// ****************************************************************************

struct QualifiedFilename
{
    QualifiedFilename();
    QualifiedFilename(std::string host_, std::string path_,
                      std::string filename_, bool access = true,
                      bool isVirtual = false);
    QualifiedFilename(const std::string &fullname);
    QualifiedFilename(const QualifiedFilename &qf);
    ~QualifiedFilename();
    void operator = (const QualifiedFilename &qf);
    bool operator == (const QualifiedFilename &qf) const;
    bool operator < (const QualifiedFilename &qf) const;
    bool operator > (const QualifiedFilename &qf) const;
    bool operator != (const QualifiedFilename &qf) const;
    bool Empty() const;

    void SetFromString(const std::string &str);
    std::string FullName() const;
    std::string PathAndFile() const;

    bool CanAccess() const    { return (flag & 1) == 1; }
    void SetAccess(bool val)  { flag = (flag & 0xfe | (val?1:0)); }
    bool IsVirtual() const    { return (flag & 2) == 2; }
    void SetVirtual(bool val) { flag = (flag & 0xfd | (val?2:0)); }

    std::string host;
    std::string path;
    std::string filename;
    char        separator;
private:
    char DetermineSeparator(const std::string &p) const;

    // Access  is bit 0
    // Virtual is bit 1
    unsigned char flag;
};

typedef std::vector<QualifiedFilename> QualifiedFilenameVector;

// Function to combine two QualifiedFilenameVector so that there are no
// duplicate filenames.
QualifiedFilenameVector
CombineQualifiedFilenameVectors(const QualifiedFilenameVector &a,
    const QualifiedFilenameVector &b);

#endif
