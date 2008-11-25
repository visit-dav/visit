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

#ifndef XMLEDITMAKEFILE_H
#define XMLEDITMAKEFILE_H

#include <QFrame>

class XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;

// ****************************************************************************
//  Class:  XMLEditMakefile
//
//  Purpose:
//    Makefile editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Brad Whitlock, Fri Feb 23 17:47:21 PST 2007
//    Added viewer widgets.
//
//    Cyrus Harrison, Wed Mar  7 09:17:11 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Cyrus Harrison, Fri Sep 19 13:58:39 PDT 2008
//    Added support for custom libs for gui,engine,mdserver, and viewer libs.
//
// ****************************************************************************
class XMLEditMakefile : public QFrame
{
    Q_OBJECT
  public:
    XMLEditMakefile(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void cxxflagsTextChanged(const QString&);
    void ldflagsTextChanged(const QString&);
    void libsTextChanged(const QString&);
    void gfilesTextChanged(const QString&);
    void glibsTextChanged(const QString&);
    void sfilesTextChanged(const QString&);
    void vfilesTextChanged(const QString&);
    void vlibsTextChanged(const QString&);
    void mfilesTextChanged(const QString&);
    void mlibsTextChanged(const QString&);
    void efilesTextChanged(const QString&);
    void elibsTextChanged(const QString&);
    void wfilesTextChanged(const QString&);
    void vwfilesTextChanged(const QString&);
    void customgfilesChanged();
    void customglibsChanged();
    void customsfilesChanged();
    void customvfilesChanged();
    void customvlibsChanged();
    void custommfilesChanged();
    void custommlibsChanged();
    void customefilesChanged();
    void customelibsChanged();
    void customwfilesChanged();
    void customvwfilesChanged();
    void mdSpecificCodeChanged();
    void engSpecificCodeChanged();
  private:
    XMLDocument     *xmldoc;

    QLineEdit       *CXXFLAGS;
    QLineEdit       *LDFLAGS;
    QLineEdit       *LIBS;
    QCheckBox       *customGFiles;
    QLineEdit       *GFiles;
    QCheckBox       *customSFiles;
    QLineEdit       *SFiles;
    QCheckBox       *customVFiles;
    QLineEdit       *VFiles;
    QCheckBox       *customMFiles;
    QLineEdit       *MFiles;
    QCheckBox       *customEFiles;
    QLineEdit       *EFiles;
    QCheckBox       *customWFiles;
    QLineEdit       *WFiles;
    QCheckBox       *customVWFiles;
    QLineEdit       *VWFiles;
    QCheckBox       *customGLibs;
    QLineEdit       *GLibs;
    QCheckBox       *customELibs;
    QLineEdit       *ELibs;
    QCheckBox       *customMLibs;
    QLineEdit       *MLibs;
    QCheckBox       *customVLibs;
    QLineEdit       *VLibs;
    QCheckBox       *mdSpecificCode;
    QCheckBox       *engSpecificCode;
};

#endif
