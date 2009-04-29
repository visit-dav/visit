/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <MovieUtility.h>
#include <visit-config.h>

#include <FileFunctions.h>
#include <InstallationFunctions.h>
#include <MovieTemplateConfig.h>
#include <DebugStream.h>

static void UpperCase(const std::string &src, std::string &dest)
{
    std::string tmp(src);
    for(int i = 0; i < tmp.size(); ++i)
    {
        char c = tmp[i];
        if(tmp[i] >= 'a' && tmp[i] <= 'z')
            tmp[i] -= 'a' - 'A';
    }
    dest = tmp;
}

// ****************************************************************************
// Class: MovieTemplateFileList
//
// Purpose: 
//   Contains the list of movie template files and whether they are user-defined.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 11:13:08 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieTemplateFileList::MovieTemplateFileList() : filenames(), userDefined()
{
}

MovieTemplateFileList::MovieTemplateFileList(const MovieTemplateFileList &obj)
{
    filenames = obj.filenames;
    userDefined = obj.userDefined;
}

MovieTemplateFileList::~MovieTemplateFileList()
{
}

MovieTemplateFileList
MovieTemplateFileList::operator =(const MovieTemplateFileList &obj)
{
    filenames = obj.filenames;
    userDefined = obj.userDefined;
    return *this;
}

// ****************************************************************************
// Function: GetVisItMovieTemplates
//
// Purpose: 
//   Returns the names of the movie template files (*.py) from the system
//   and user movie template directories.
//
// Returns:    A vector of filenames that contain the path to the movie 
//             template files.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 09:02:38 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

static void
AddMovieTemplateCB(void *cbdata, const std::string &filename, bool isDir,
    bool canAccess, long size)
{
    std::vector<std::string> *templateFiles = (std::vector<std::string> *)cbdata;
    if(!isDir && canAccess)
    {
        std::string ext(filename.substr(filename.size() - 3, filename.size()-1));
        UpperCase(ext, ext);
        if(ext == ".MT")
        {
            void **arr = (void **)cbdata;
            MovieTemplateFileList *fl = (MovieTemplateFileList *)arr[0];
            int                    onoff = (int)((long)arr[1]);
            fl->filenames.push_back(filename);
            fl->userDefined.push_back(onoff ? true : false);
        }
    }
}

// ****************************************************************************
// Function: GetVisItMovieTemplates
//
// Purpose: 
//   Reads the VisIt template directories and assembles a template file list.
//
// Returns:    The list of template files.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:47:31 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieTemplateFileList
GetVisItMovieTemplates()
{
#if defined(_WIN32)
    std::string templateDir(GetVisItArchitectureDirectory() + "\\movietemplates");
#else
    std::string templateDir(GetVisItArchitectureDirectory() + "/bin/movietemplates");
#endif

    // Get the names of the VisIt template files in the installed version.
    MovieTemplateFileList fileList;
    void *cb_data[2];
    cb_data[0] = (void *)&fileList;
    cb_data[1] = (void *)0;
    std::vector<std::string> templateFiles;
    debug1 << "GetVisItMovieTemplates: Trying to read system movie templates from " 
           << templateDir.c_str() << endl;
    ReadAndProcessDirectory(templateDir, AddMovieTemplateCB, (void *)cb_data, true);

    // Get the names of the VisIt template files that the user may have.
    templateDir = GetUserVisItDirectory() + "movietemplates";
    cb_data[1] = (void *)1;
    debug1 << "GetVisItMovieTemplates: Trying to read user movie templates from " 
           << templateDir.c_str() << endl;
    ReadAndProcessDirectory(templateDir, AddMovieTemplateCB, (void *)cb_data, true);

    return fileList;
}

// ****************************************************************************
// Function: GetVisItMovieTemplateBaseClass
//
// Purpose: 
//   Returns the name of the VisItMovieTemplate base class PY file that we'll
//   use to handle templates in the general case.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 10 14:27:43 PST 2006
//
// Modifications:
//   Brad Whitlock, Thu Dec 21 17:16:03 PST 2006
//   I made it use the architecture directory.
//
// ****************************************************************************

std::string
GetVisItMovieTemplateBaseClass()
{
#if defined(_WIN32)
    std::string templateFile(GetVisItArchitectureDirectory() + "\\movietemplates\\visitmovietemplate.py");
#else
    std::string templateFile(GetVisItArchitectureDirectory() + "/bin/movietemplates/visitmovietemplate.py");
#endif
    debug1 << "GetVisItMovieTemplateBaseClass = " << templateFile.c_str() << endl;
    return templateFile;
}

// ****************************************************************************
// Function: GetNewTemplateSpecificationName
//
// Purpose: 
//   Returns the name of a new movie template specification file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 10 14:27:43 PST 2006
//
// Modifications:
//   
// ****************************************************************************

std::string
GetNewTemplateSpecificationName()
{
#if defined(_WIN32)
    std::string templateFile(GetUserVisItDirectory() + "movietemplates\\new.mt");
#else
    std::string templateFile(GetUserVisItDirectory() + "movietemplates/new.mt");
#endif
    return templateFile;
}

// ****************************************************************************
// Method: MovieTemplateInformation methods.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 09:23:53 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieTemplateInformation::MovieTemplateInformation() : title(),
    description(), specificationFile(), templateFile(), previewImageFile(), 
    sessionFile()
{
    usesSessionFile = false;
}

MovieTemplateInformation::MovieTemplateInformation(const MovieTemplateInformation &obj)
{
    title = obj.title;
    description = obj.description;
    specificationFile = obj.specificationFile;
    templateFile = obj.templateFile;
    previewImageFile = obj.previewImageFile;
    sessionFile = obj.sessionFile;
    usesSessionFile = obj.usesSessionFile;
}

MovieTemplateInformation::~MovieTemplateInformation()
{
}

MovieTemplateInformation
MovieTemplateInformation::operator =(const MovieTemplateInformation &obj)
{
    title = obj.title;
    description = obj.description;
    specificationFile = obj.specificationFile;
    templateFile = obj.templateFile;
    previewImageFile = obj.previewImageFile;
    sessionFile = obj.sessionFile;
    usesSessionFile = obj.usesSessionFile;

    return *this;
}

// ****************************************************************************
// Method: GetMovieTemplateInformation
//
// Purpose: 
//   Returns some information about the movie template by reading some 
//   information from the header comment in the movie template.
//
// Arguments:
//   filename           : The movie template file.
//   title              : The display title for the movie template.
//   description        : The description to show in the UI when this 
//                        template is activated.
//   defaultsFile       : The path to the XML file that contains default
//                        values for the template.
//   userInterfaceFiles : The list of user interface files that create the 
//                        user interface for the movie template.
//   previewImageFile   : The path to the image file that shows the preview
//                        image for the movie template.
//
// Returns:    True on success, false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 09:04:09 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
GetMovieTemplateInformation(const std::string &filename, MovieTemplateInformation &info)
{
    // Use a MovieTemplateConfig object to get the movie template information.
    MovieTemplateConfig cfg;
    bool retval = cfg.ReadConfigFile(filename.c_str()) != 0;
    DataNode *root = 0;

    if(retval && (root = cfg.GetRootNode()) != 0)
    {
        // Get the path to the filename so we can prepend it to all of
        // the UI filenames.
        std::string::size_type pos = filename.rfind(SLASH_STRING);
        std::string prefix;
        if(pos != std::string::npos)
            prefix = filename.substr(0, pos+1);

        cfg.GetTitle(info.title);
        cfg.GetDescription(info.description);
        info.specificationFile = filename;

        cfg.GetTemplateFile(info.templateFile);
        if(!(info.templateFile.size() > 0 && info.templateFile[0] == SLASH_CHAR) &&
           !(info.templateFile.size() > 2 && info.templateFile[1] == ':'))
        {
             info.templateFile = prefix + info.templateFile;
        }

        cfg.GetPreviewImageFile(info.previewImageFile);
        if(!(info.previewImageFile.size() > 0 && info.previewImageFile[0] == SLASH_CHAR) &&
           !(info.previewImageFile.size() > 2 && info.previewImageFile[1] == ':'))
        {
             info.previewImageFile = prefix + info.previewImageFile;
        }

        info.usesSessionFile = cfg.GetSessionFile(info.sessionFile);
        if(info.usesSessionFile)
        {
            if(!(info.sessionFile.size() > 0 && info.sessionFile[0] == SLASH_CHAR) &&
               !(info.sessionFile.size() > 2 && info.sessionFile[1] == ':'))
            {
                info.sessionFile = prefix + info.sessionFile;
            }
        }
        else
            info.sessionFile = "";
    }
    return retval;
}
