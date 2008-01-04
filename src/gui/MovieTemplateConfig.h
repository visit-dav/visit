#ifndef MOVIE_TEMPLATE_CONFIG_H
#define MOVIE_TEMPLATE_CONFIG_H
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
#include <ConfigManager.h>
#include <map>

class MovieSequence;

// ****************************************************************************
// Class: MovieTemplateConfig
//
// Purpose:
//   Reads and writes movie template config files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 10:11:56 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

class MovieTemplateConfig : public ConfigManager
{
public:
    MovieTemplateConfig();
    virtual ~MovieTemplateConfig();

    void CreateDefaultConfig();
    virtual bool WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);
    DataNode *GetRootNode() const;

    //
    // Set/Get top-level template information
    //
    bool GetTitle(std::string &) const;
    bool SetTitle(const std::string &);
    bool GetDescription(std::string &) const;
    bool SetDescription(const std::string &);
    bool GetTemplateFile(std::string &) const;
    bool SetTemplateFile(const std::string &);
    bool GetPreviewImageFile(std::string &) const;
    bool SetPreviewImageFile(const std::string &);
    bool GetSessionFile(std::string &) const;
    bool SetSessionFile(const std::string &);
    bool GetSources(stringVector &) const;
    bool SetSources(const stringVector &);

    //
    // Viewport management functions
    //
    int GetNumberOfViewports() const;
    bool ViewportAdd(const std::string &name, 
                     float llx, float lly, float urx, float ury);
    bool ViewportRemove(const std::string &name);
    bool ViewportRemoveAll();
    bool ViewportActivate(const std::string &name);
    bool ViewportGetActiveName(std::string &);
    bool ViewportGetIndexForName(const std::string &, int &) const;
    bool ViewportGetNameForIndex(int, std::string &) const;


    bool ViewportSetCoordinates(float llx, float lly, float urx, float ury);
    bool ViewportSetCoordinates(const std::string &name,
                                float llx, float lly, float urx, float ury);

    bool ViewportGetCoordinates(float &llx, float &lly, float &urx, float &ury);
    bool ViewportGetCoordinates(const std::string &name,
                                float &llx, float &lly,
                                float &urx, float &ury) const;

    bool ViewportSetCompositing(int comp);
    bool ViewportSetCompositing(const std::string &, int comp);
    bool ViewportGetCompositing(int &comp);
    bool ViewportGetCompositing(const std::string &, int &comp) const;

    bool ViewportSetOpacity(float opacity);
    bool ViewportSetOpacity(const std::string &, float opacity);
    bool ViewportGetOpacity(float &opacity);
    bool ViewportGetOpacity(const std::string &, float &opacity) const;

    bool ViewportSetReplaceColor(const int *replaceColor);
    bool ViewportSetReplaceColor(const std::string &, const int *replaceColor);
    bool ViewportGetReplaceColor(int *replaceColor);
    bool ViewportGetReplaceColor(const std::string &, int *replaceColor) const;

    bool ViewportSetDropShadow(bool dropShadow);
    bool ViewportSetDropShadow(const std::string &name, bool dropShadow);
    bool ViewportGetDropShadow(bool &dropShadow);
    bool ViewportGetDropShadow(const std::string &name, bool &dropShadow) const;

    bool ViewportSetSequenceList(const stringVector &);
    bool ViewportSetSequenceList(const std::string &, const stringVector &);
    bool ViewportGetSequenceList(stringVector &);
    bool ViewportGetSequenceList(const std::string &, stringVector &) const;

    //
    // Sequence management functions
    //
    int GetNumberOfSequences() const;
    int GetNumberOfMappedSequences() const;
    bool SequenceGetNameForIndex(int, std::string &) const;
    bool SequenceGetIndexForName(const std::string &, int &) const;
    MovieSequence *GetActiveSequence();
    MovieSequence *GetSequence(const std::string &) const;
    bool SequenceAdd(int type, std::string &name);
    bool SequenceAdd(const std::string &name, int type);
    bool SequenceRemove(const std::string &name);
    bool SequenceRemoveAll();
    bool SequenceActivate(const std::string &name);
    bool SequenceRename(const std::string &name, const std::string &newName);

    bool SequenceUnmap(const std::string &seqName);
    bool SequenceMapToViewport(const std::string &seqName,
                               const std::string &vpName);
    bool SequenceGetViewport(const std::string &seqName,
                             std::string &vpName);

private:
    DataNode *GetViewport(const std::string &name) const;
    DataNode *GetActiveViewport();
    void CreateSequenceObjects(DataNode *, const char *, int);

    typedef std::map<std::string, MovieSequence *> StringMovieSequenceMap;

    DataNode               *root;
    DataNode               *activeViewport;
    MovieSequence          *activeSequence;
    StringMovieSequenceMap  sequences;
};

#endif
