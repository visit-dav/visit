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

#ifndef QVIS_DELAYED_WINDOW_H
#define QVIS_DELAYED_WINDOW_H
#include <gui_exports.h>
#include <QvisWindowBase.h>

class QWidget;
class QVBoxLayout;

// ****************************************************************************
// Class: QvisDelayedWindow
//
// Purpose:
//   This class defines the interface for a top level window that
//   is not created until it is needed.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 11:27:10 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 13:41:37 PST 2000
//   Made it inherit from QvisWindowBase.
//
//   Brad Whitlock, Fri Feb 15 11:31:07 PDT 2002
//   Changed protection on some methods.
//
//   Brad Whitlock, Mon Sep 30 07:50:55 PDT 2002
//   I added window flags.
//
//   Brad Whitlock, Wed Apr  9 10:31:28 PDT 2008
//   Changed ctor args.
//
// ****************************************************************************

class GUI_API QvisDelayedWindow : public QvisWindowBase
{
    Q_OBJECT
public:
    QvisDelayedWindow(const QString &captionString, WFlags f = 0);
    virtual ~QvisDelayedWindow();
    QWidget *GetCentralWidget();
    virtual void CreateEntireWindow();
public slots:
    virtual void raise();
    virtual void show();
    virtual void hide();
protected:
    virtual void CreateWindowContents() = 0;
    virtual void UpdateWindow(bool doAll);
protected:
    bool        isCreated;
    QWidget     *central;
    QVBoxLayout *topLayout;
};

#endif
