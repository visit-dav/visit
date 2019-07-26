// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MOVIE_SEQUENCE_FACTORY_H
#define MOVIE_SEQUENCE_FACTORY_H

#include <map>
#include <string>
#include <QString>

class MovieSequence;
class QPixmap;

// ****************************************************************************
// Class: MovieSequenceFactory
//
// Purpose:
//   Singleton factory class that is used to instantiate MovieSequence
//   objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:28:40 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:09:34 PDT 2008
//   Changed SequenceMenuName so it returns QString.
//
// ****************************************************************************

class MovieSequenceFactory
{
public:
    static MovieSequenceFactory *Instance();

    int NumSequenceTypes() const;
    bool SequenceIdForIndex(int index, int &id) const;
    bool SequenceNameToId(const std::string &name, int &id) const;

    bool SequenceName(int id, std::string &name) const;
    bool SequenceMenuName(int id, QString &name) const;
    bool SequenceSubMenuIndex(int id, int &index) const;
    bool SequenceProvidesMenu(int id) const;
    bool SequencePixmap(int id, QPixmap &pix) const;

    MovieSequence *Create(int id);
 
private:
    MovieSequenceFactory();
    virtual ~MovieSequenceFactory();

    static MovieSequenceFactory *instance;
    std::map<int, MovieSequence *> sequenceTypes;
};

#endif
