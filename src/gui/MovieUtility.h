// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MOVIE_UTILITY_H
#define MOVIE_UTILITY_H

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
