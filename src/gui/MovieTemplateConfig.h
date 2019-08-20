// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MOVIE_TEMPLATE_CONFIG_H
#define MOVIE_TEMPLATE_CONFIG_H

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
    virtual bool WriteConfigFile(std::ostream& out);
    virtual DataNode *ReadConfigFile(std::istream& in);
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
