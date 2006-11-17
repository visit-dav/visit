#ifndef MOVIE_SEQUENCE_H
#define MOVIE_SEQUENCE_H
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
#include <string>
#include <qpixmap.h>

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
//   
// ****************************************************************************

class MovieSequence
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
    virtual std::string SequenceMenuName() const = 0;
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
