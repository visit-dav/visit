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
#include <MovieSequenceTransition.h>

#include <WidgetDataNode.h>
#include <QvisSequenceTransition.h>
#include <DebugStream.h>

#include <icons/ms_fade.xpm>
#include <icons/ms_block.xpm>
#include <icons/ms_bt_slide.xpm>
#include <icons/ms_bt_wipe.xpm>
#include <icons/ms_circle.xpm>
#include <icons/ms_lr_slide.xpm>
#include <icons/ms_lr_wipe.xpm>
#include <icons/ms_ripple.xpm>
#include <icons/ms_rl_slide.xpm>
#include <icons/ms_rl_wipe.xpm>
#include <icons/ms_tb_slide.xpm>
#include <icons/ms_tb_wipe.xpm>


// ****************************************************************************
// Method: MovieSequenceTransition::MovieSequenceTransition
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:39:38 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequenceTransition::MovieSequenceTransition() : MovieSequence()
{
}

// ****************************************************************************
// Method: MovieSequenceTransition::~MovieSequenceTransition
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:39:38 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequenceTransition::~MovieSequenceTransition()
{
}

// ****************************************************************************
// Method: MovieSequenceTransition::CreateUI
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
//   
// ****************************************************************************

QWidget *
MovieSequenceTransition::CreateUI()
{
    QvisSequenceTransition *ui = new QvisSequenceTransition(Pixmap(), 0, GetName().c_str());
    ui->setTitle(SequenceMenuName().c_str());

    return ui;
}

// ****************************************************************************
// Method: MovieSequenceTransition::ReadUIValues
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
MovieSequenceTransition::ReadUIValues(QWidget *ui, DataNode *node)
{
    const char *mName = "MovieSequenceTransition::ReadUIValues: ";

    if(node != 0)
    {
        QvisSequenceTransition *UI = (QvisSequenceTransition *)ui;

        // Read the input method and colors and put into the UI.
        DataNode *input1Node = node->GetNode("input1");
        DataNode *color1Node = node->GetNode("color1");
        if(input1Node != 0 && input1Node->GetNodeType() == STRING_NODE)
        {
            bool frames = input1Node->AsString() == "Frames";
            if(frames || input1Node->AsString() == "Color")
            {
                QColor c;
                DataNodeToQColor(color1Node, c);

                UI->setFromTransition(frames, c);                
            }
            else
            {
                debug4 << mName << "input1 set to invalid type." << endl;
            }
        }
        else
        {
            debug4 << mName << "input1,color1 not found or were the "
                   << "wrong type." << endl;
        }

        // Read the input method and colors and put into the UI.
        DataNode *input2Node = node->GetNode("input2");
        DataNode *color2Node = node->GetNode("color2");
        if(input2Node != 0 && input2Node->GetNodeType() == STRING_NODE)
        {
            bool frames = input2Node->AsString() == "Frames";
            if(frames || input2Node->AsString() == "Color")
            {
                QColor c;
                DataNodeToQColor(color2Node, c);

                UI->setToTransition(frames, c);                
            }
            else
            {
                debug4 << mName << "input2 set to invalid type." << endl;
            }
        }
        else
        {
            debug4 << mName << "input2,color2 not found or were the "
                   << "wrong type." << endl;
        }

        // Read the number of frames.
        DataNode *nFramesNode = node->GetNode("nFrames");
        if(nFramesNode !=0 && nFramesNode->GetNodeType() == INT_NODE)
        {
            UI->setNFrames(nFramesNode->AsInt());
        }
        else
        {
            debug4 << mName << "nFrames not found in node." << endl;
        }
    }
}

// ****************************************************************************
// Method: MovieSequenceTransition::WriteUIValues
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
MovieSequenceTransition::WriteUIValues(QWidget *ui, DataNode *node)
{
    if(node != 0)
    {
        QvisSequenceTransition *UI = (QvisSequenceTransition *)ui;
        bool frames;
        QColor c;
        int nFrames;

        nFrames = UI->getNFrames();

        UI->getFromTransition(frames, c);
        node->RemoveNode("input1");
        if(frames)
            node->AddNode(new DataNode("input1", std::string("Frames")));
        else
            node->AddNode(new DataNode("input1", std::string("Color")));
        QColorToDataNode(node, "color1", c);

        UI->getToTransition(frames, c);
        node->RemoveNode("input2");
        if(frames)
            node->AddNode(new DataNode("input2", std::string("Frames")));
        else
            node->AddNode(new DataNode("input2", std::string("Color")));
        QColorToDataNode(node, "color2", c);

        node->RemoveNode("nFrames");
        node->AddNode(new DataNode("nFrames", nFrames));
    }
}

//
// Pixmap methods for the derived classes.
//

QPixmap
MovieSequenceFade::Pixmap()
{
    return QPixmap(ms_fade_xpm);
}

QPixmap
MovieSequenceLRWipe::Pixmap()
{
    return QPixmap(ms_lr_wipe_xpm);
}

QPixmap
MovieSequenceRLWipe::Pixmap()
{
    return QPixmap(ms_rl_wipe_xpm);
}

QPixmap
MovieSequenceTBWipe::Pixmap()
{
    return QPixmap(ms_tb_wipe_xpm);
}

QPixmap
MovieSequenceBTWipe::Pixmap()
{
    return QPixmap(ms_bt_wipe_xpm);
}

QPixmap
MovieSequenceLRSlide::Pixmap()
{
    return QPixmap(ms_lr_slide_xpm);
}

QPixmap
MovieSequenceRLSlide::Pixmap()
{
    return QPixmap(ms_rl_slide_xpm);
}

QPixmap
MovieSequenceTBSlide::Pixmap()
{
    return QPixmap(ms_tb_slide_xpm);
}

QPixmap
MovieSequenceBTSlide::Pixmap()
{
    return QPixmap(ms_bt_slide_xpm);
}

QPixmap
MovieSequenceCircle::Pixmap()
{
    return QPixmap(ms_circle_xpm);
}

QPixmap
MovieSequenceRipple::Pixmap()
{
    return QPixmap(ms_ripple_xpm);
}

QPixmap
MovieSequenceBlock::Pixmap()
{
    return QPixmap(ms_block_xpm);
}

