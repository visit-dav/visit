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
#include <MovieSequence.h>
#include <DataNode.h>

#include <visit-config.h>

#include <QFile>
#include <QWidget>
#include <QvisUiLoader.h>

#include <WidgetDataNode.h>

// ****************************************************************************
// Method: MovieSequence::MovieSequence
//
// Purpose: 
//   Constructor for MovieSequence, which is the base class for all movie 
//   sequences.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:57:35 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 08:53:15 PDT 2008
//   Added QObject inheritance.
//
//   Brad Whitlock, Tue Oct  7 09:53:13 PDT 2008
//   Added name.
//
// ****************************************************************************

MovieSequence::MovieSequence() : QObject(), uiFile(), name()
{
}

// ****************************************************************************
// Method: MovieSequence::~MovieSequence
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:57:35 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequence::~MovieSequence()
{
}

// ****************************************************************************
// Method: MovieSequence::ProvidesMenu
//
// Purpose: 
//   Returns flag indicating whether sequences provides a menu.
//
// Returns:    True if the sequence provides a menu; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:58:22 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieSequence::ProvidesMenu() const
{
    return true;
}

// ****************************************************************************
// Method: MovieSequence::SubMenuIndex
//
// Purpose: 
//   Returns the index of the submenu, if any, to which the sequence belongs.
//
// Returns:    -1
//
// Note:       By default, sequences are put in the top level menu.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:59:01 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

int
MovieSequence::SubMenuIndex() const
{
    return -1;
}

// ****************************************************************************
// Method: MovieSequence::Pixmap
//
// Purpose:
//   Returns the pixmap associated with this sequence object type.
//
// Returns:    pixmap
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:59:50 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QPixmap
MovieSequence::Pixmap()
{
    return QPixmap();
}

// ****************************************************************************
// Method: MovieSequence::SupportsCustomUI
//
// Purpose: 
//   Returns whether the sequence returns a custom user interface.
//
// Returns:    False.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:00:14 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieSequence::SupportsCustomUI() const
{
    return false;
}

// ****************************************************************************
// Method: MovieSequence::SetName
//
// Purpose: 
//   Lets the user rename the sequence.
//
// Arguments:
//   n : The new name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:00:14 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void 
MovieSequence::SetName(const std::string &n)
{
    name = n;
}

// ****************************************************************************
// Method: MovieSequence::GetName
//
// Purpose: 
//   Returns the sequence's name.
//
// Returns:    The sequence's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:01:20 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

std::string 
MovieSequence::GetName() const
{
    return name;
}

// ****************************************************************************
// Method: MovieSequence::ReadOnly
//
// Purpose: 
//   Returns whether the sequence is "read only".
//
// Returns:    False.
//
// Note:       Derived classes that are "read only" cannot have their names
//             changed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:01:46 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieSequence::ReadOnly() const
{
    return false;
}

// ****************************************************************************
// Method: MovieSequence::SetUIFile
//
// Purpose: 
//   Sets the Qt designer UI file associated with this sequence.
//
// Arguments:
//   ui : The name of the UI file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:02:33 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void 
MovieSequence::SetUIFile(const std::string &ui)
{
    uiFile = ui;
}

// ****************************************************************************
// Method: MovieSequence::GetUIFile
//
// Purpose: 
//   Gets the name of the UI file associated with this sequence.
//
// Returns:    The UI filename.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:03:08 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

std::string 
MovieSequence::GetUIFile() const
{
    return uiFile;
}

// ****************************************************************************
// Method: MovieSequence::InitializeFromValues
//
// Purpose: 
//   Lets the object initialize itself from the sequence node.
//
// Arguments:
//   xmlfile : The name of the template specification XML file that contains
//             the sequence data.
//   node    : The DataNode representation of the template specification file.
//
// Note:       The XML file is passed in so we can construct a path for the
//             UI file in case it was given as a relative path.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:03:32 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
MovieSequence::InitializeFromValues(const std::string &xmlFile, DataNode *node)
{
    if(node != 0 && SupportsCustomUI())
    {
        DataNode *uiNode = node->GetNode("uiFile");
        if(uiNode != 0 && uiNode->GetNodeType() == STRING_NODE)
        {
            // Set uiFile based on the value from the data node.
            uiFile = std::string(uiNode->AsString().c_str());

            // Prepend the path to the XML file to the UI file if the
            // UI file does not have a path.
            if(!(uiFile.size() > 0 && uiFile[0] == SLASH_CHAR) &&
               !(uiFile.size() > 2 && uiFile[1] == ':'))
            {
                std::string::size_type pos = xmlFile.rfind(SLASH_STRING);
                std::string prefix;
                if(pos != std::string::npos)
                    prefix = xmlFile.substr(0, pos+1);

                uiFile = prefix + uiFile;
            }
        }
    }
}

// ****************************************************************************
// Method: MovieSequence::CreateUI
//
// Purpose: 
//   Creates a custom user interface for the sequence if one is needed.
//
// Returns:    A pointer to a custom user interface.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:05:26 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Oct  7 09:50:55 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QWidget *
MovieSequence::CreateUI()
{
    QWidget *ui = 0;
    if(SupportsCustomUI())
    {
        // If we have what could be a valid UI file then try and use it.
        if(uiFile.size() > 0)
        {
            QFile f(uiFile.c_str());
            if(f.open(QIODevice::ReadOnly))
            {
                QvisUiLoader *loader = new QvisUiLoader;
                ui = loader->load(&f);
                delete loader;
            }
        }
    }

    return ui;
}

// ****************************************************************************
// Method: MovieSequence::ReadUIValues
//
// Purpose: 
//   Reads the UI's default values from the sequence node.
//
// Arguments:
//   ui   : A pointer to the user interface.
//   node : The data node representation of the UI values.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:05:53 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
MovieSequence::ReadUIValues(QWidget *ui, DataNode *node)
{
    InitializeWidgetFromDataNode(ui, node);
}

// ****************************************************************************
// Method: MovieSequence::WriteUIValues
//
// Purpose: 
//   Writes the UI's values to the sequence node.
//
// Arguments:
//   ui   : A pointer to the user interface.
//   node : The data node representation of the UI values.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:06:33 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
MovieSequence::WriteUIValues(QWidget *ui, DataNode *node)
{
    // Write the new uiFile node if necessary.
    if(node != 0 && SupportsCustomUI())
    {
        DataNode *uiNode = node->GetNode("uiFile");
        if(uiNode != 0 && uiNode->GetNodeType() == STRING_NODE)
        {
            // See if we need to save the path.
            std::string origUIFile(uiNode->AsString());
            std::string uiNoPath(uiFile.substr(
                uiFile.size() - origUIFile.size(), origUIFile.size()));
            if(uiNoPath != origUIFile)
            {
                // The user must have changed the name of the UI file.
                node->RemoveNode("uiFile");
                node->AddNode(new DataNode("uiFile", uiFile));
            }
        }
        else
        {
            // The node did not have an acceptable uiFile node.
            node->RemoveNode("uiFile");
            node->AddNode(new DataNode("uiFile", uiFile));
        }
    }

    // Initialize the rest of the sequence values from the widgets in ui.
    InitializeDataNodeFromWidget(ui, node);
}
