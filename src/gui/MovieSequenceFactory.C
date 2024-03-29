// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <MovieSequenceFactory.h>
#include <MovieSequence.h>
#include <QPixmap>
#include <QPixmapCache>

// Include movie sequence types here.
#include <MovieSequenceCurrentPlots.h>
#include <MovieSequenceScripting.h>
#include <MovieSequenceTransition.h>
#include <MovieSequenceCompositeTransition.h>
#include <MovieSequenceRotate.h>

MovieSequenceFactory *MovieSequenceFactory::instance = 0;

// ****************************************************************************
// Method: MovieSequenceFactory::Instance
//
// Purpose:
//   Static method that makes this class a singleton.
//
// Returns:    The only instance of the class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:29:33 PDT 2006
//
// Modifications:
//
// ****************************************************************************

MovieSequenceFactory *
MovieSequenceFactory::Instance()
{
    if(instance == 0)
        instance = new MovieSequenceFactory;

    return instance;
}

// ****************************************************************************
// Method: MovieSequenceFactory::MovieSequenceFactory
//
// Purpose:
//   Constructor.
//
// Note:       We keep a list of movie sequences and use their NewInstance
//             methods to create new instances for this factory class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:30:02 PDT 2006
//
// Modifications:
//
// ****************************************************************************

MovieSequenceFactory::MovieSequenceFactory() : sequenceTypes()
{
    MovieSequence *s = 0;
#define REGISTER_SEQUENCE_TYPE(T) \
    s = new T();\
    sequenceTypes[s->SequenceId()] = s;

    // Register all of the sequence types that we'll support.
    REGISTER_SEQUENCE_TYPE(MovieSequenceCurrentPlots);
    REGISTER_SEQUENCE_TYPE(MovieSequenceScripting);

    REGISTER_SEQUENCE_TYPE(MovieSequenceFade);
    REGISTER_SEQUENCE_TYPE(MovieSequenceLRWipe);
    REGISTER_SEQUENCE_TYPE(MovieSequenceRLWipe);
    REGISTER_SEQUENCE_TYPE(MovieSequenceTBWipe);
    REGISTER_SEQUENCE_TYPE(MovieSequenceBTWipe);
    REGISTER_SEQUENCE_TYPE(MovieSequenceLRSlide);
    REGISTER_SEQUENCE_TYPE(MovieSequenceRLSlide);
    REGISTER_SEQUENCE_TYPE(MovieSequenceTBSlide);
    REGISTER_SEQUENCE_TYPE(MovieSequenceBTSlide);
    REGISTER_SEQUENCE_TYPE(MovieSequenceCircle);
    REGISTER_SEQUENCE_TYPE(MovieSequenceRipple);
    REGISTER_SEQUENCE_TYPE(MovieSequenceBlock);

    REGISTER_SEQUENCE_TYPE(MovieSequenceCompositeFadeIn);
    REGISTER_SEQUENCE_TYPE(MovieSequenceCompositeFadeOut);
    REGISTER_SEQUENCE_TYPE(MovieSequenceCompositeLRSlide);
    REGISTER_SEQUENCE_TYPE(MovieSequenceCompositeRLSlide);
    REGISTER_SEQUENCE_TYPE(MovieSequenceCompositeTBSlide);
    REGISTER_SEQUENCE_TYPE(MovieSequenceCompositeBTSlide);

    REGISTER_SEQUENCE_TYPE(MovieSequenceScreenRotationX);
    REGISTER_SEQUENCE_TYPE(MovieSequenceScreenRotationY);
    REGISTER_SEQUENCE_TYPE(MovieSequenceScreenRotationZ);
    REGISTER_SEQUENCE_TYPE(MovieSequenceRotationX);
    REGISTER_SEQUENCE_TYPE(MovieSequenceRotationY);
    REGISTER_SEQUENCE_TYPE(MovieSequenceRotationZ);

    //REGISTER_SEQUENCE_TYPE(SequenceTitleScreen);
}

// ****************************************************************************
// Method: MovieSequenceFactory::~MovieSequenceFactory
//
// Purpose:
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:30:59 PDT 2006
//
// Modifications:
//
// ****************************************************************************

MovieSequenceFactory::~MovieSequenceFactory()
{
    for(std::map<int, MovieSequence *>::iterator it = sequenceTypes.begin();
        it != sequenceTypes.end(); ++it)
        delete it->second;
}

// ****************************************************************************
// Method: MovieSequenceFactory::NumSequenceTypes
//
// Purpose:
//   Returns the number of sequence types.
//
// Returns:    Returns the number of sequence types.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:31:11 PDT 2006
//
// Modifications:
//
// ****************************************************************************

int
MovieSequenceFactory::NumSequenceTypes() const
{
    return sequenceTypes.size();
}

// ****************************************************************************
// Method: MovieSequenceFactory::SequenceIdForIndex
//
// Purpose:
//   Returns the sequence id for a given index into the number of registered
//   sequence types.
//
// Arguments:
//   index : An integer [0, sequenceTypes.end()].
//   id    : Int reference in which to return the sequence id.
//
// Returns:    True on success; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:31:38 PDT 2006
//
// Modifications:
//
// ****************************************************************************

bool
MovieSequenceFactory::SequenceIdForIndex(int index, int &id) const
{
    int i = 0;
    for(std::map<int, MovieSequence *>::const_iterator it = sequenceTypes.begin();
        it != sequenceTypes.end(); ++it, ++i)
    {
        if(i == index)
        {
            id = it->second->SequenceId();
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: MovieSequenceFactory::SequenceNameToId
//
// Purpose:
//   Returns a sequence id given a sequence name.
//
// Arguments:
//   name : The sequence name.
//   id   : Int reference in which to return the sequence id.
//
// Returns:    True on success; False otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:32:50 PDT 2006
//
// Modifications:
//
// ****************************************************************************

bool
MovieSequenceFactory::SequenceNameToId(const std::string &name, int &id) const
{
    for(std::map<int, MovieSequence *>::const_iterator it = sequenceTypes.begin();
        it != sequenceTypes.end(); ++it)
    {
        if(it->second->SequenceName() == name)
        {
            id = it->second->SequenceId();
            return true;
        }
    }

    return false;
}

// ****************************************************************************
// Method: MovieSequenceFactory::SequenceName
//
// Purpose:
//   Returns the id'th sequence name.
//
// Arguments:
//   id : The id of the sequence.
//   name : A string reference that will contain the returned sequence name.
//
// Returns:    True on success; False otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:33:38 PDT 2006
//
// Modifications:
//
// ****************************************************************************

bool
MovieSequenceFactory::SequenceName(int id, std::string &name) const
{
    std::map<int, MovieSequence *>::const_iterator it = sequenceTypes.find(id);
    if(it != sequenceTypes.end())
    {
        name = it->second->SequenceName();
        return true;
    }

    return false;
}

// ****************************************************************************
// Method: MovieSequenceFactory::SequenceMenuName
//
// Purpose:
//   Returns the id'th sequence menu name.
//
// Arguments:
//   id : The id of the sequence.
//   name : A string reference that will contain the returned sequence menu name.
//
// Returns:    True on success; False otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:33:38 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:08:56 PDT 2008
//   Changed std::string to QString.
//
// ****************************************************************************

bool
MovieSequenceFactory::SequenceMenuName(int id, QString &name) const
{
    std::map<int, MovieSequence *>::const_iterator it = sequenceTypes.find(id);
    if(it != sequenceTypes.end())
    {
        name = it->second->SequenceMenuName();
        return true;
    }

    return false;
}

// ****************************************************************************
// Method: MovieSequenceFactory::SequenceProvidesMenu
//
// Purpose:
//   Returns whether the id'th sequence provides a menu.
//
// Arguments:
//   id : The id of the sequence.
//
// Returns:    True if the sequence provides a menu; False otherwise
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:33:38 PDT 2006
//
// Modifications:
//
// ****************************************************************************

bool
MovieSequenceFactory::SequenceProvidesMenu(int id) const
{
    bool ret = false;

    std::map<int, MovieSequence *>::const_iterator it = sequenceTypes.find(id);
    if(it != sequenceTypes.end())
        ret = it->second->ProvidesMenu();

    return ret;
}

// ****************************************************************************
// Method: MovieSequenceFactory::SequenceSubMenuIndex
//
// Purpose:
//   Returns the index of the sub menu that a sequence will occupy.
//
// Arguments:
//   id    : The id of the sequence.
//   index : An int reference in which to store the sub menu index.
//
// Returns:    True on success; False otherwise.
//
// Note:       Many sequence occupy a sub-menu (for convenient grouping)
//             but they don't have to.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:33:38 PDT 2006
//
// Modifications:
//
// ****************************************************************************

bool
MovieSequenceFactory::SequenceSubMenuIndex(int id, int &index) const
{
    bool ret = false;

    std::map<int, MovieSequence *>::const_iterator it = sequenceTypes.find(id);
    if(it != sequenceTypes.end())
    {
        ret = true;
        index = it->second->SubMenuIndex();
    }

    return ret;
}

// ****************************************************************************
// Method: MovieSequenceFactory::SequencePixmap
//
// Purpose:
//   Finds the pixmap for the id'th sequence.
//
// Arguments:
//   id  : The id of the sequence.
//   pix : The return pixmap reference.
//
// Returns:    True on success; False otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:37:09 PDT 2006
//
// Modifications:
//   Kathleen Biagas, Thu Jan 21 2021
//   Swap use of QString::sprintf for Qtring.arg as suggested in Qt docs.
//
// ****************************************************************************

bool
MovieSequenceFactory::SequencePixmap(int id, QPixmap &pix) const
{
    std::map<int, MovieSequence *>::const_iterator it = sequenceTypes.find(id);
    if(it != sequenceTypes.end())
    {
        // Look in the cache for it and return
        QString key = QString("%1_%2")
            .arg(it->second->SequenceName().c_str())
            .arg(it->second->SequenceId());

        if(QPixmapCache::find(key, &pix))
        {
            return true;
        }

        // Get the pixmap from the derived sequence object.
        pix = it->second->Pixmap();

        // Store the pixmap in the cache if the pixmap is valid.
        if(!pix.isNull())
        {
            QPixmapCache::insert(key, pix);
        }

        return true;
    }

    return false;
}

// ****************************************************************************
// Method: MovieSequenceFactory::Create
//
// Purpose:
//   Factory method for creating new sequences of a given type.
//
// Arguments:
//   id : The sequence id of the type of sequence that we want to create.
//
// Returns:    A new instance of the specified class.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:38:20 PDT 2006
//
// Modifications:
//
// ****************************************************************************

MovieSequence *
MovieSequenceFactory::Create(int id)
{
    MovieSequence *obj = 0;

    std::map<int, MovieSequence *>::iterator it = sequenceTypes.find(id);
    if(it != sequenceTypes.end())
    {
        obj = it->second->NewInstance();
    }

    return obj;
}
