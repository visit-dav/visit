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

// ****************************************************************************
// Purpose:  This file is a set of routines that can set/get values from 
//           QWidget/DataNode. This allows us to use custom UI components. We
//           could even use them to create DataNodes that AttributeSubjects
//           could use to initialize themselves, which would open the door to
//           plots with UI's created with Qt designer. It would also mean that
//           we might not need Qt to build new UI's at all, which would be a
//           plus on systems where you have to pay for QT.
//   
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 6 17:35:57 PST 2006
//
// Modifications:
//   
// ****************************************************************************


#include <DataNode.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <QvisColorButton.h>
#include <QvisDialogLineEdit.h>
#include <QvisVariableButton.h>

#include <DebugStream.h>

void
ConvertTextToDataNodeSettings(const QString &text, DataNode *node)
{
}

// ****************************************************************************
// Function: DataNodeToBool
//
// Purpose: 
//   Converts a data node value into a bool
//
// Arguments:
//   node : The node to evaluate.
//
// Returns:    The converted bool value.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 10:37:19 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

static bool
DataNodeToBool(DataNode *node)
{
    bool ret = false;
    switch(node->GetNodeType())
    {
    case CHAR_NODE:
        ret = node->AsChar() > 0;
        break;
    case UNSIGNED_CHAR_NODE:
        ret = node->AsUnsignedChar() > 0;
        break;
    case INT_NODE:
        ret = node->AsInt() > 0;
        break;
    case LONG_NODE:
        ret = node->AsLong() > 0L;
        break;
    case FLOAT_NODE:
        ret = node->AsFloat() > 0.f;
        break;
    case DOUBLE_NODE:
        ret = node->AsDouble() > 0.;
        break;
    case STRING_NODE:
        ret = node->AsString() == "TRUE" || node->AsString() == "true";
        break;
    case BOOL_NODE:
        ret = node->AsBool();
        break;
    default:
        ret = false;
    }

    return ret;
}

// ****************************************************************************
// Function: DataNodeToInt
//
// Purpose: 
//   Converts a data node into an int.
//
// Arguments:
//   node : The node to convert.
//
// Returns:    The int value of the data node.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 10:37:54 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

static int
DataNodeToInt(DataNode *node)
{
    int ret = 0;
    switch(node->GetNodeType())
    {
    case CHAR_NODE:
        ret = int(node->AsChar());
        break;
    case UNSIGNED_CHAR_NODE:
        ret = int(node->AsUnsignedChar());
        break;
    case INT_NODE:
        ret = node->AsInt();
        break;
    case LONG_NODE:
        ret = int(node->AsLong());
        break;
    case FLOAT_NODE:
        ret = int(node->AsFloat());
        break;
    case DOUBLE_NODE:
        ret = int(node->AsDouble());
        break;
    case STRING_NODE:
        {
            int tmp;
            if(sscanf(node->AsString().c_str(), "%d", &tmp) == 1)
                ret = tmp;
            else
                ret = 0;
        }
        break;
    case BOOL_NODE:
        ret = node->AsBool() ? 1 : 0;
        break;
    default:
        ret = 0;
    }

    return ret;
}

// ****************************************************************************
// Function: DataNodeToQString
//
// Purpose: 
//   Converts the data node into a QString.
//
// Arguments:
//   node : The data node to convert.
//
// Returns:    The QString representation of the data node.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 10:38:27 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QString
DataNodeToQString(const DataNode *node)
{
    QString s, tmp;
    int i;
#define ARRAY_TO_STRING(Type, Method, Fmt, suffix)\
        {\
            const Type *ptr = node->Method();\
            for(i = 0; i < node->GetLength(); ++i)\
            {\
                tmp.sprintf(Fmt, ptr[i] suffix);\
                if(i > 0)\
                    s += " ";\
                s += tmp;\
            }\
        }

#define VECTOR_TO_STRING(Type, Method, Fmt, suffix)\
        {\
            const Type &vec = node->Method();\
            for(size_t i = 0; i < vec.size(); ++i)\
            {\
                tmp.sprintf(Fmt, vec[i] suffix);\
                if(i > 0)\
                    s += " ";\
                s += tmp;\
            }\
        }
    switch(node->GetNodeType())
    {
    case INTERNAL_NODE:
        break;
    case CHAR_NODE:
        s.sprintf("%d", (int)node->AsChar());
        break;
    case UNSIGNED_CHAR_NODE:
        s.sprintf("%d", (int)node->AsUnsignedChar());
        break;
    case INT_NODE:
        s.sprintf("%d", node->AsInt());
        break;
    case LONG_NODE:
        s.sprintf("%ld", node->AsLong());
        break;
    case FLOAT_NODE:
        s.sprintf("%f", node->AsFloat());
        break;
    case DOUBLE_NODE:
        s.sprintf("%lg", node->AsDouble());
        break;
    case STRING_NODE:
        s.sprintf("%s", node->AsString().c_str());
        break;
    case BOOL_NODE:
        if(node->AsBool()) s = "true"; else s = "false";
        break;
    case CHAR_ARRAY_NODE:
        {
            const char *cptr = node->AsCharArray();
            for(i = 0; i < node->GetLength(); ++i)
            {
                tmp.sprintf("%d", (int)cptr[i]);
                if(i > 0)
                    s += " ";
                s += tmp;
            }
        }
        break;
    case UNSIGNED_CHAR_ARRAY_NODE:
        {
            const unsigned char *cptr = node->AsUnsignedCharArray();
            for(i = 0; i < node->GetLength(); ++i)
            {
                tmp.sprintf("%d", (int)cptr[i]);
                if(i > 0)
                    s += " ";
                s += tmp;
            }
        }
        break;
    case INT_ARRAY_NODE:
        ARRAY_TO_STRING(int, AsIntArray, "%d",);
        break;
    case LONG_ARRAY_NODE:
        ARRAY_TO_STRING(long, AsLongArray, "%ld",);
        break;
    case FLOAT_ARRAY_NODE:
        ARRAY_TO_STRING(float, AsFloatArray, "%f",);
        break;
    case DOUBLE_ARRAY_NODE:
        ARRAY_TO_STRING(double, AsDoubleArray, "%lg",);
        break;
    case STRING_ARRAY_NODE:
        ARRAY_TO_STRING(std::string, AsStringArray, "\"%s\"", .c_str());
        break;
    case BOOL_ARRAY_NODE:
        {
            const bool *ptr = node->AsBoolArray();
            for(i = 0; i < node->GetLength(); ++i)
            {
                if(ptr[i])
                    tmp = "true";
                else
                    tmp = "false";
                if(i > 0)
                    s += " ";
                s += tmp;
            }
        }
        break;
    case CHAR_VECTOR_NODE:
        {
            const charVector &vec = node->AsCharVector();
            for(size_t i = 0; i < vec.size(); ++i)
            {
                tmp.sprintf("%d", (int)vec[i]);
                if(i > 0)
                    s += " ";
                s += tmp;
            }
        }
        break;
    case UNSIGNED_CHAR_VECTOR_NODE:
        {
            const unsignedCharVector &vec = node->AsUnsignedCharVector();
            for(size_t i = 0; i < vec.size(); ++i)
            {
                tmp.sprintf("%d", (int)vec[i]);
                if(i > 0)
                    s += " ";
                s += tmp;
            }
        }
        break;
    case INT_VECTOR_NODE:
        VECTOR_TO_STRING(intVector, AsIntVector, "%d",);
        break;
    case LONG_VECTOR_NODE:
        VECTOR_TO_STRING(longVector, AsLongVector, "%ld",);
        break;
    case FLOAT_VECTOR_NODE:
        VECTOR_TO_STRING(floatVector, AsFloatVector, "%f",);
        break;
    case DOUBLE_VECTOR_NODE:
        VECTOR_TO_STRING(doubleVector, AsDoubleVector, "%lg",);
        break;
    case STRING_VECTOR_NODE:
        VECTOR_TO_STRING(stringVector, AsStringVector, "\"%s\"", .c_str());
        break;
    default:
        break;
    }

#undef ARRAY_TO_STRING
#undef VECTOR_TO_STRING

    return s;
}

// ****************************************************************************
// Function: DataNodeToQColor
//
// Purpose: 
//   Converts a data node representation of color into a QColor.
//
// Arguments:
//   node : The data to convert to QColor.
//   color : The return QColor object.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 13:34:12 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool DataNodeToQColor(DataNode *node, QColor &color)
{
    bool  retval = true;
    int   i, rgb[3] = {0, 0, 0};
    float f_rgb[3] = {0., 0., 0.};
    bool  fp = false;
    
#define ARRAY_TO_RGB(Rgb, Type, Method, Cast) \
        if(node->GetLength() >= 3) \
        { \
            const Type *ptr = node->Method();\
            for(i = 0; i < 3; ++i) \
                Rgb[i] = Cast ptr[i]; \
        }

#define VECTOR_TO_RGB(Rgb, Type, Method, Cast) \
        { \
            const Type &vec = node->Method(); \
            if(vec.size() >= 3) \
            { \
                for(i = 0; i < 3; ++i) \
                    Rgb[i] = Cast vec[i]; \
            } \
        }

    switch(node->GetNodeType())
    {
    case CHAR_ARRAY_NODE:
        ARRAY_TO_RGB(rgb, char, AsCharArray, (int));
        break;
    case UNSIGNED_CHAR_ARRAY_NODE:
        ARRAY_TO_RGB(rgb, unsigned char, AsUnsignedCharArray, (int));
        break;
    case INT_ARRAY_NODE:
        ARRAY_TO_RGB(rgb, int, AsIntArray, (int));
        break;
    case LONG_ARRAY_NODE:
        ARRAY_TO_RGB(rgb, long, AsLongArray, (int));
        break;
    case FLOAT_ARRAY_NODE:
        ARRAY_TO_RGB(f_rgb, float, AsFloatArray, (float));
        fp = true;
        break;
    case DOUBLE_ARRAY_NODE:
        ARRAY_TO_RGB(f_rgb, double, AsDoubleArray, (float));
        fp = true;
        break;
    case CHAR_VECTOR_NODE:
        VECTOR_TO_RGB(rgb, charVector, AsCharVector, (int));
        break;
    case UNSIGNED_CHAR_VECTOR_NODE:
        VECTOR_TO_RGB(rgb, unsignedCharVector, AsUnsignedCharVector, (int));
        break;
    case INT_VECTOR_NODE:
        VECTOR_TO_RGB(rgb, intVector, AsIntVector, (int));
        break;
    case LONG_VECTOR_NODE:
        VECTOR_TO_RGB(rgb, longVector, AsLongVector, (int));
        break;
    case FLOAT_VECTOR_NODE:
        VECTOR_TO_RGB(f_rgb, floatVector, AsFloatVector, (float));
        fp = true;
        break;
    case DOUBLE_VECTOR_NODE:
        VECTOR_TO_RGB(f_rgb, doubleVector, AsDoubleVector, (float));
        fp = true;
        break;
    default:
        retval = false;
        break;
    }

#undef ARRAY_TO_RGB
#undef VECTOR_TO_RGB
#define COLOR_CLAMP(C) (((C < 0) ? 0 : C) > 255) ? 255 : ((C < 0) ? 0 : C);

    if(retval)
    {
        if(fp)
        {
            rgb[0] = int(f_rgb[0] * 255.);
            rgb[1] = int(f_rgb[1] * 255.);
            rgb[2] = int(f_rgb[2] * 255.);
        }
        rgb[0] = COLOR_CLAMP(rgb[0]);
        rgb[1] = COLOR_CLAMP(rgb[1]);
        rgb[2] = COLOR_CLAMP(rgb[2]);
        color = QColor(rgb[0], rgb[1], rgb[2]);
    }
#undef COLOR_CLAMP

    return retval;
}

// ****************************************************************************
// Method: QColorToDataNode
//
// Purpose: 
//   This function inserts a color into a data node.
//
// Arguments:
//   node : The node into which we're inserting the color.
//   key  : The name of the new color node.
//   c    : The color to insert.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 13:35:18 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QColorToDataNode(DataNode *node, const char *key, const QColor &c)
{
    int tmp[3];
    tmp[0] = c.red();
    tmp[1] = c.green();
    tmp[2] = c.blue();

    node->RemoveNode(key);
    node->AddNode(new DataNode(key, tmp, 3));
}

// ****************************************************************************
// Method: InitializeQComboBoxFromDataNode
//
// Purpose: 
//   Initializes a QComboBox from a DataNode
//
// Arguments:
//   obj  : The QComboBox to initialize.
//   node : The DataNode to use for initialization.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 14:07:48 PST 2006
//
// Modifications:
//   
// ****************************************************************************

static void
InitializeQComboBoxFromDataNode(QComboBox *co, DataNode *node)
{
    if(node->GetNodeType() == INT_NODE)
    {
        int index = node->AsInt();
        if(index < 0 || index >= co->count())
        {
            debug1 << node->GetKey().c_str() << " is out of range [0,"
                   << co->count() << "]" << endl;
            index = 0;
        }

        co->setCurrentItem(index);
    }
    else if(node->GetNodeType() == STRING_NODE)
    {
        int index = 0;
        for(int i = 0; i < co->count(); ++i)
        {
            if(std::string(co->text(i).latin1()) == node->AsString())
            {
                co->setCurrentItem(i);
                return;
            }
        }

        debug1 << node->GetKey().c_str() << " value of " << node->AsString().c_str()
               << " does not match any of the items in the QComboBox." << endl;
    }
    else
    {
        debug1 << "InitializeQComboBoxFromDataNode: only supports INT_NODE, STRING_NODE"
               << endl;
    }
}

// ****************************************************************************
// Function: InitializeDataNodeFromQComboBox
//
// Purpose: 
//   Initializes a data node from the active item in a QComboBox.
//
// Arguments:
//   co   : The combo box that we're considering.
//   node : The parent of the node that we'll create.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 14:16:16 PST 2006
//
// Modifications:
//   
// ****************************************************************************

static void
InitializeDataNodeFromQComboBox(QComboBox *co, DataNode *node)
{
    DataNode *currentNode = node->GetNode(co->name());
    if(currentNode != 0)
    {
        // Use int or string, depending on what the node was initially.
        NodeTypeEnum t = currentNode->GetNodeType();
        if(t != INT_NODE && t != STRING_NODE)
        {
            debug1 << "InitializeDataNodeFromQComboBox: only supports INT_NODE, STRING_NODE"
                   << endl;
            t = INT_NODE;
        }

        node->RemoveNode(co->name());

        if(t == INT_NODE)
            node->AddNode(new DataNode(co->name(), co->currentItem()));
        else if(t == STRING_NODE)
        {
            node->AddNode(new DataNode(co->name(), 
                std::string(co->text(co->currentItem()).latin1())));
        }
    }
    else
    {
        // There's no preference on which type to use so use int.
        node->AddNode(new DataNode(co->name(), co->currentItem()));
    }
}

// ****************************************************************************
// Function: InitializeQCheckBoxFromDataNode
//
// Purpose: 
//   Initializes a QCheckBox from a data node.
//
// Arguments:
//   co   : The check box to initialize.
//   node : The data node to use for values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 10:40:22 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

static void
InitializeQCheckBoxFromDataNode(QCheckBox *co, DataNode *node)
{
    co->setChecked(DataNodeToBool(node));
}

// ****************************************************************************
// Method: InitializeDataNodeFromQCheckBox
//
// Purpose: 
//   Initialize a data node from a QCheckBox.
//
// Arguments:
//   co : The check box from which to get the bool.
//   node : The node that will get the new bool value.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 10:41:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

static void
InitializeDataNodeFromQCheckBox(QCheckBox *co, DataNode *node)
{
    node->RemoveNode(co->name());
    node->AddNode(new DataNode(co->name(), co->isChecked()));
}

// ****************************************************************************
// Function: InitializeQButtonGroupFromDataNode
//
// Purpose: 
//   Initializes a QButtonGroup from a data node.
//
// Arguments:
//   co : The button group to initialize.
//   node : The data node to use for initialization.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 10:41:50 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

static void
InitializeQButtonGroupFromDataNode(QButtonGroup *co, DataNode *node)
{
    int index = DataNodeToInt(node);
    co->setButton(index);
}

// ****************************************************************************
// Method: InitializeDataNodeFromQButtonGroup
//
// Purpose: 
//   Initializes a data node from a QButtonGroup.
//
// Arguments:
//   co   : The button group widget.
//   node : The data node to populate.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 10:39:23 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

static void
InitializeDataNodeFromQButtonGroup(QButtonGroup *co, DataNode *node)
{
    DataNode *currentNode = node->GetNode(co->name());
    QButton *sb = co->selected();

    if(currentNode != 0)
    {
        // Use int or string, depending on what the node was initially.
        NodeTypeEnum t = currentNode->GetNodeType();
        node->RemoveNode(co->name());

        if(t == STRING_NODE)
        {
            if(sb != 0)
                node->AddNode(new DataNode(co->name(), std::string(sb->text().latin1())));
            else
                node->AddNode(new DataNode(co->name(), int(0)));
        }
        else
        {
            int index = sb ? co->id(sb) : 0;
            node->AddNode(new DataNode(co->name(), index));
        }
    }
    else
    {
        // There's no preference on which type to use so use int
        int index = sb ? co->id(sb) : 0;
        node->AddNode(new DataNode(co->name(), index));
    }
}

// ****************************************************************************
// Function: InitializeWidgetFromDataNode
//
// Purpose: 
//   This function initializes a Qt widget (and its children) using a data
//   node, allowing us to initialize our custom UI's from data node from
//   saved settings, session files, etc.
//
// Arguments:
//   ui   : The parent widget to initialize.
//   node : The node that contains the data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 13:36:10 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
InitializeWidgetFromDataNode(QWidget *ui, DataNode *node)
{
    const char *mName = "InitializeWidgetFromDataNode";

    // Iterate over the objects in the custom page and try and find
    // a setting in the config file with the same name. If we find
    // a compatible value then set it.
    QObjectList *objList = ui->queryList();
    QObjectListIt it(*objList);
    QObject *obj = 0;
    while((obj = it.current()) != 0)
    {
        ++it;
        DataNode *objValues = node->GetNode(obj->name());
        if(objValues)
        {
            debug4 << "Found values for object "
                   << obj->name() << endl;
            if(obj->inherits("QButtonGroup"))
            {
                InitializeQButtonGroupFromDataNode((QButtonGroup*)obj, node);
            }
            else if(obj->inherits("QCheckBox"))
            {
                InitializeQCheckBoxFromDataNode((QCheckBox*)obj, node);
            }
            else if(obj->inherits("QComboBox"))
            {
                InitializeQComboBoxFromDataNode((QComboBox*)obj, node);
            }
            if(obj->inherits("QLineEdit"))
            {
                QLineEdit *co = (QLineEdit *)obj;
                co->setText(DataNodeToQString(objValues));
            }
            else if(obj->inherits("QSpinBox"))
            {
                QSpinBox *co = (QSpinBox *)obj;
                int ival = DataNodeToInt(objValues);
                co->setValue(ival);
            }
            else if(obj->inherits("QTextEdit"))
            {
                QTextEdit *co = (QTextEdit *)obj;
                co->setText(DataNodeToQString(objValues));
            }

            // Fill in the rest of these ...

            // Do some of VisIt's custom widgets.
            else if(obj->inherits("QvisColorButton"))
            {
                QvisColorButton *cb = (QvisColorButton *)obj;
                QColor color;
                if(DataNodeToQColor(objValues, color))
                {
                    debug4 << "Setting " << obj->name()
                           << " to color (" << color.red()
                           << ", " << color.green()
                           << ", " << color.blue() << ")" << endl;
                    cb->setButtonColor(color);
                }
                else
                {
                    debug4 << mName << "There was no acceptable "
                           << "conversion from the data node type "
                           << "to QColor for " << obj->name()
                           << ".\n";
                }
            }
            else if(obj->inherits("QvisDialogLineEdit"))
            {
                QvisDialogLineEdit *co = (QvisDialogLineEdit *)obj;
                co->setText(DataNodeToQString(objValues));
            }
            else if(obj->inherits("QvisVariableButton"))
            {
                QvisVariableButton *co = (QvisVariableButton *)obj;
                co->setVariable(DataNodeToQString(objValues));
            }
            else
            {
                debug4 << "There is currently no support for "
                       << obj->name() << " widgets." << endl;
            }
        }
        else
        {
            debug4 << "Did not find values for object "
                   << obj->name() << endl;
        }
    }

    delete objList;
}

// ****************************************************************************
// Function: InitializeDataNodeFromWidget
//
// Purpose: 
//   This function adds the widget data values into the data node structure.
//
// Arguments:
//   ui   : The parent widget.
//   node : The node that will get the widget data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 13:38:15 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
InitializeDataNodeFromWidget(QWidget *ui, DataNode *node)
{
    // Iterate over the objects in the custom page and try and find
    // a setting in the config file with the same name. If we find
    // a compatible value then set it.
    QObjectList *objList = ui->queryList();
    QObjectListIt it(*objList);
    QObject *obj = 0;
    while((obj = it.current()) != 0)
    {
        ++it;
        DataNode *objValues = node->GetNode(obj->name());
        if(objValues)
        {
            // objValues points to the node in the settings that 
            // contains the settings for the named widget.
            if(obj->inherits("QButtonGroup"))
            {
                InitializeDataNodeFromQButtonGroup((QButtonGroup *)obj, node);
            }
            else if(obj->inherits("QCheckBox"))
            {
                InitializeDataNodeFromQCheckBox((QCheckBox *)obj, node);
            }
            else if(obj->inherits("QComboBox"))
            {
                InitializeDataNodeFromQComboBox((QComboBox *)obj, node);
            }
            else if(obj->inherits("QLineEdit"))
            {
                QLineEdit *co = (QLineEdit *)obj;
                QString text(co->text());

                // Change values by deleting and inserting a node.
                node->RemoveNode(obj->name());
                node->AddNode(new DataNode(obj->name(), std::string(text.latin1())));
            }
            else if(obj->inherits("QSpinBox"))
            {
                QSpinBox *co = (QSpinBox *)obj;

                // Change values by deleting and inserting a node.
                node->RemoveNode(obj->name());
                node->AddNode(new DataNode(obj->name(), co->value()));
            }
            else if(obj->inherits("QTextEdit"))
            {
                QTextEdit *co = (QTextEdit *)obj;
                QString text(co->text());

                // Change values by deleting and inserting a node.
                node->RemoveNode(obj->name());
                node->AddNode(new DataNode(obj->name(), std::string(text.latin1())));
            }

            //
            // Do some of VisIt's custom widgets.
            //
            else if(obj->inherits("QvisColorButton"))
            {
                QvisColorButton *co = (QvisColorButton *)obj;

                // Delete the current data node values.
                node->RemoveNode(obj->name());

                // Add in the new color data node.
                int rgb[3];
                rgb[0] = co->buttonColor().red();
                rgb[1] = co->buttonColor().green();
                rgb[2] = co->buttonColor().blue();
                node->AddNode(new DataNode(obj->name(), rgb, 3));
            }
            else if(obj->inherits("QvisDialogLineEdit"))
            {
                QvisDialogLineEdit *co = (QvisDialogLineEdit *)obj;
                QString text(co->text());

                // Change values by deleting and inserting a node.
                node->RemoveNode(obj->name());
                node->AddNode(new DataNode(obj->name(), std::string(text.latin1())));
            }
            else if(obj->inherits("QvisVariableButton"))
            {
                QvisVariableButton *co = (QvisVariableButton *)obj;
                QString text(co->getVariable());

                // Change values by deleting and inserting a node.
                node->RemoveNode(obj->name());
                node->AddNode(new DataNode(obj->name(), std::string(text.latin1())));
            }
        }
    }
}
