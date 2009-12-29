/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef XMLEDITPLUGIN_H
#define XMLEDITPLUGIN_H

#include <qframe.h>

class XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QGroupBox;

// ****************************************************************************
//  Class:  XMLEditPlugin
//
//  Purpose:
//    Plugin editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 13 11:13:05 PDT 2003
//    I added support for plugin icons.
//
//    Hank Childs, Fri Aug  1 11:27:57 PDT 2003
//    Added support for curves.
//
//    Jeremy Meredith, Tue Sep 23 17:05:53 PDT 2003
//    Added support for tensor and symmetric tensor variable types.
//    Added support for the "haswriter" database plugin field.
//
//    Jeremy Meredith, Wed Nov  5 13:49:49 PST 2003
//    Added support for enabling or disabling plugins by default.
//
//    Brad Whitlock, Fri Apr 1 16:14:33 PST 2005
//    Added label var type.
//
//    Hank Childs, Tue May 24 09:26:14 PDT 2005
//    Added hasOptions.
//
//    Hank Childs, Tue Jul 19 14:08:19 PDT 2005
//    Added array var type.
//
//    Hank Childs, Thu Jan 10 13:56:32 PST 2008
//    Added the ability to have a plugin only open explicit filenames.
//
//    Brad Whitlock, Fri Mar 7 10:55:40 PDT 2008
//    Added some group boxes.
//
//    Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//    Ported to Qt 4.4
//
//    Jeremy Meredith, Tue Dec 29 11:21:30 EST 2009
//    Replaced "Extensions" and "Filenames" with "FilePatterns".  Removed
//    specifiedFilenames.  Added filePatternsStrict and opensWholeDirectory.
//
// ****************************************************************************

class XMLEditPlugin : public QFrame
{
    Q_OBJECT
  public:
    XMLEditPlugin(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void attpluginGroupChanged(int);
    void nameTextChanged(const QString&);
    void labelTextChanged(const QString&);
    void versionTextChanged(const QString&);
    void iconFileTextChanged(const QString&);
    void hasIconChanged(bool);
    void hasWriterChanged(bool);
    void hasOptionsChanged(bool);
    void enabledByDefaultChanged(bool);
    void pluginTypeChanged(int);
    void varTypesChanged();
    void dbTypeChanged(int);
    void filePatternsTextChanged(const QString&);
    void filePatternsStrictChanged(bool);
    void opensWholeDirectoryChanged(bool);
  private:
    XMLDocument     *xmldoc;

    QButtonGroup    *attpluginGroup;
    QGroupBox       *pluginGroup;
    QComboBox       *pluginType;
    QLineEdit       *name;
    QLineEdit       *label;
    QLineEdit       *version;
    QCheckBox       *hasIcon;
    QCheckBox       *hasWriter;
    QCheckBox       *hasOptions;
    QCheckBox       *enabledByDefault;
    QLineEdit       *iconFile;
    QGroupBox       *plotPluginGroup;
    QCheckBox       *varTypeMesh;
    QCheckBox       *varTypeScalar;
    QCheckBox       *varTypeVector;
    QCheckBox       *varTypeMaterial;
    QCheckBox       *varTypeSubset;
    QCheckBox       *varTypeSpecies;
    QCheckBox       *varTypeCurve;
    QCheckBox       *varTypeTensor;
    QCheckBox       *varTypeSymmetricTensor;
    QCheckBox       *varTypeLabel;
    QCheckBox       *varTypeArray;
    QGroupBox       *dbPluginGroup;
    QComboBox       *dbType;
    QLineEdit       *filePatterns;
    QCheckBox       *filePatternsStrict;
    QCheckBox       *opensWholeDirectory;
};

#endif
