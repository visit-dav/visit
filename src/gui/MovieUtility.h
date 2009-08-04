#ifndef MOVIE_UTILITY_H
#define MOVIE_UTILITY_H
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
#include <string>
#include <vector>

// ****************************************************************************
// Class: MovieTemplateFileList
//
// Purpose:
//   This class contains a list of filenames and a list of flags indicating
//   whether the files in the filenames list are user-defined movie templates.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:25:30 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

class MovieTemplateFileList
{
public:
    MovieTemplateFileList();
    MovieTemplateFileList(const MovieTemplateFileList &);
    ~MovieTemplateFileList();
    MovieTemplateFileList operator =(const MovieTemplateFileList &);

    std::vector<std::string> filenames;
    std::vector<bool>        userDefined;
};

// ****************************************************************************
// Class: MovieTemplateInformation
//
// Purpose:
//   This class contains basic information about a movie template.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:26:17 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

class MovieTemplateInformation
{
public:
    MovieTemplateInformation();
    MovieTemplateInformation(const MovieTemplateInformation &);
    ~MovieTemplateInformation();
    MovieTemplateInformation operator =(const MovieTemplateInformation &);

    std::string title;
    std::string description;
    std::string specificationFile;
    std::string templateFile;
    std::string previewImageFile;
    std::string sessionFile;
    bool        usesSessionFile;
};

//
// Functions for accessing movie templates.
//
MovieTemplateFileList GetVisItMovieTemplates();
std::string GetVisItMovieTemplateBaseClass();
std::string GetNewTemplateSpecificationName();
bool GetMovieTemplateInformation(const std::string &filename, 
                                 MovieTemplateInformation &info);

#endif
