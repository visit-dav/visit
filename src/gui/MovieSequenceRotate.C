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
#include <MovieSequenceRotate.h>

#include <WidgetDataNode.h>
#include <QvisRotationTransition.h>
#include <DebugStream.h>

#include <icons/ms_srotx.xpm>
#include <icons/ms_sroty.xpm>
#include <icons/ms_srotz.xpm>
#include <icons/ms_rotx.xpm>
#include <icons/ms_roty.xpm>
#include <icons/ms_rotz.xpm>


// ****************************************************************************
// Method: MovieSequenceRotate::MovieSequenceRotate
//
// Purpose: 
//   Constructor for the MovieSequenceRotate base class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:53:43 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequenceRotate::MovieSequenceRotate() : MovieSequence()
{
}

// ****************************************************************************
// Method: MovieSequenceRotate::~MovieSequenceRotate
//
// Purpose: 
//   Destructor for the MovieSequenceRotate base class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:53:43 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequenceRotate::~MovieSequenceRotate()
{
}

// ****************************************************************************
// Method: MovieSequenceRotate::CreateUI
//
// Purpose: 
//   Creates the user interface for the sequence.
//
// Returns:    A pointer to a new widget that can be used as the sequence's
//             user interface within the movie wizard.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:40:11 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 10:34:24 PDT 2008
//   SequenceMenuName now returns QString.
//
//   Brad Whitlock, Tue Oct  7 11:21:26 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QWidget *
MovieSequenceRotate::CreateUI()
{
    QvisRotationTransition *ui = new QvisRotationTransition(Pixmap(), 0);
    ui->setObjectName(GetName().c_str());
    ui->setTitle(SequenceMenuName());

    return ui;
}

// ****************************************************************************
// Method: MovieSequenceRotate::ReadUIValues
//
// Purpose: 
//   Gets the values from the data node and updates the user interface.
//
// Arguments:
//   ui   : The user interface.
//   node : The node in which to insert the sequence's data values.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:40:58 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
MovieSequenceRotate::ReadUIValues(QWidget *ui, DataNode *node)
{
    const char *mName = "MovieSequenceRotate::ReadUIValues: ";

    if(node != 0)
    {
        QvisRotationTransition *UI = (QvisRotationTransition *)ui;

        // Read the start angle and put it into the UI.
        DataNode *saNode = node->GetNode("startAngle");
        if(saNode != 0 && saNode->GetNodeType() == FLOAT_NODE)
            UI->setStartAngle(saNode->AsFloat());                
        else
        {
            debug4 << mName << "startAngle not found or it was the "
                   << "wrong type." << endl;
        }

        // Read the start angle and put it into the UI.
        DataNode *eaNode = node->GetNode("endAngle");
        if(eaNode != 0 && eaNode->GetNodeType() == FLOAT_NODE)
            UI->setEndAngle(eaNode->AsFloat());                
        else
        {
            debug4 << mName << "endAngle not found or it was the "
                   << "wrong type." << endl;
        }

        // Read the number of steps.
        DataNode *nStepsNode = node->GetNode("nSteps");
        if(nStepsNode !=0 && nStepsNode->GetNodeType() == INT_NODE)
        {
            UI->setNSteps(nStepsNode->AsInt());
        }
        else
        {
            debug4 << mName << "nSteps not found in node." << endl;
        }
    }
}

// ****************************************************************************
// Method: MovieSequenceRotate::WriteUIValues
//
// Purpose: 
//   Gets data values from the user interface and inserts them into the
//   data node representation where they can be written to the template
//   specification file.
//
// Arguments:
//   ui   : The user interface.
//   node : The node in which to insert the sequence's data values.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:40:58 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
MovieSequenceRotate::WriteUIValues(QWidget *ui, DataNode *node)
{
    if(node != 0)
    {
        QvisRotationTransition *UI = (QvisRotationTransition *)ui;
        float sa, ea;
        int nSteps;

        nSteps = UI->getNSteps();
        UI->getAngles(sa, ea);

        node->RemoveNode("startAngle");
        node->AddNode(new DataNode("startAngle", sa));
        node->RemoveNode("endAngle");
        node->AddNode(new DataNode("endAngle", ea));
        node->RemoveNode("nSteps");
        node->AddNode(new DataNode("nSteps", nSteps));
    }
}

//
// Pixmap methods for the derived classes.
//

QPixmap
MovieSequenceScreenRotationX::Pixmap()
{
    return QPixmap(ms_srotx_xpm);
}

QString
MovieSequenceScreenRotationX::SequenceMenuName() const
{
    return tr("Screen rotate X");
}

////////////////////////////////////////////////////////////////////////////////

QPixmap
MovieSequenceScreenRotationY::Pixmap()
{
    return QPixmap(ms_sroty_xpm);
}

QString
MovieSequenceScreenRotationY::SequenceMenuName() const
{
    return tr("Screen rotate Y");
}

////////////////////////////////////////////////////////////////////////////////

QPixmap
MovieSequenceScreenRotationZ::Pixmap()
{
    return QPixmap(ms_srotz_xpm);
}

QString
MovieSequenceScreenRotationZ::SequenceMenuName() const
{
    return tr("Screen rotate Z");
}

////////////////////////////////////////////////////////////////////////////////

QPixmap
MovieSequenceRotationX::Pixmap()
{
    return QPixmap(ms_rotx_xpm);
}

QString
MovieSequenceRotationX::SequenceMenuName() const
{
    return tr("Rotate X");
}

////////////////////////////////////////////////////////////////////////////////

QPixmap
MovieSequenceRotationY::Pixmap()
{
    return QPixmap(ms_roty_xpm);
}

QString
MovieSequenceRotationY::SequenceMenuName() const
{
    return tr("Rotate Y");
}

////////////////////////////////////////////////////////////////////////////////

QPixmap
MovieSequenceRotationZ::Pixmap()
{
    return QPixmap(ms_rotz_xpm);
}

QString
MovieSequenceRotationZ::SequenceMenuName() const
{
    return tr("Rotate Z");
}


