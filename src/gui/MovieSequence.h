// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MOVIE_SEQUENCE_H
#define MOVIE_SEQUENCE_H

#include <QObject>
#include <string>
#include <QPixmap>

class DataNode;
class QWidget;

// ****************************************************************************
// Class: MovieSequence
//
// Purpose: 
//   Movie sequence base class.
//
// Note:       This is the base class for movie sequences, which are sequences
//             of frames that can appear in a movie. This class primarily
//             deals with providing different user interfaces for the various
//             types of sequences and includes methods to set/get the
//             sequence values from the movie template file's DataNodes.
//
//             New movie sequence types are added by adding a new instance
//             of the movie sequence in the MovieSequenceFactory constructor.
//             Since that's the only place they are added, we could in theory
//             make these be plugins.
//
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:52:36 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 08:50:52 PDT 2008
//   I made SequenceMenuName return a QString so we can tr() it.
//
// ****************************************************************************

class MovieSequence : public QObject
{
public:
    MovieSequence();
    virtual ~MovieSequence();

    // Creates a new instance of the movie sequence object.
    virtual MovieSequence *NewInstance() const = 0;

    // Returns the sequence Id that will be used to identify the
    // type of sequence object this is.
    virtual int SequenceId() const = 0;

    // Returns the name of the sequence as it will appear in the XML file.
    virtual std::string SequenceName() const = 0;

    // Returns the name of the sequence as it will appear in menus.
    virtual QString SequenceMenuName() const = 0;
    virtual bool ProvidesMenu() const;
    virtual int SubMenuIndex() const;

    // Returns the pixmap associated with this sequence object type.
    virtual QPixmap Pixmap();

    // Returns whether the sequence supports a custom UI.
    virtual bool SupportsCustomUI() const;

    // Set the name for the sequence.  
    void SetName(const std::string &);
    std::string GetName() const;

    virtual bool ReadOnly() const;

    // Set the UI file for the sequence.  
    void SetUIFile(const std::string &);
    std::string GetUIFile() const;

    // Lets the object initialize itself from the sequence node.
    virtual void InitializeFromValues(const std::string &xmlFile, DataNode *node);

    // Creates a custom user interface for the sequence if one is needed.
    virtual QWidget *CreateUI();

    // Reads the UI's default values from the sequence node.
    virtual void ReadUIValues(QWidget *ui, DataNode *);

    // Writes the UI's values to the sequence node.
    virtual void WriteUIValues(QWidget *ui, DataNode *);

protected:
    std::string  uiFile;
    std::string  name;
};

#endif
