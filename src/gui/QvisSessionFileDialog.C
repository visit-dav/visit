/*****************************************************************************
*
* Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <QvisSessionFileDialog.h>
#include <QComboBox>


// ****************************************************************************
// Method: QvisSessionFileDialog::QvisSessionvisFileDialog
//
// Purpose:
//   Constructor for the QvisSessionFileDialog class.
//   This dialog is used for both Save and Restore of Session files.
//
// Arguments:
//   caption : The name of the window.
//
// Programmer: David Camp
// Creation:   Tue Aug  4 11:04:14 PDT 2015
//
// Modifications:
//
// ****************************************************************************

QvisSessionFileDialog::QvisSessionFileDialog(const QString &caption) : 
    QvisFileOpenDialog(caption)
{
}

// ****************************************************************************
// Method: QvisSessionFileDialog::~QvisSessionFileDialog
//
// Purpose: 
//   Destructor.
//
// Programmer: David Camp
// Creation:   Tue Aug  4 11:04:14 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

QvisSessionFileDialog::~QvisSessionFileDialog()
{
}

// ****************************************************************************
// Method: QvisFileDialog::getFileName
//
// Purpose:
//   Function for getting the name of an existing file using VisIt's
//   File open window or creating a filename if in save mode.
//
// Arguments:
//   type        : Type of dlg Save or Load.
//   initialFile : The host, path, and filename of the initial file.
//   caption     : The name of the window.
//
// Returns:    The name of the file, selected or created, or a null QString if
//             the user did not select a file.
//
// Note:
//
// Programmer: David Camp
// Creation:   Thu Aug  6 07:32:42 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisSessionFileDialog::getFileName(DLG_TYPE type, 
                                   const QString &initialFile,
                                   QualifiedFilename &filename)
{
    QString qfilename;
#if defined(_WIN32)
    QString fltr("*.session *.vses");
#else
    QString fltr("*.session");
#endif

    SetUsageMode(QvisFileOpenDialog::SelectFilename);
    SetHideFileFormat(true);
    SetHideOptions(true);
    if(type == SAVE_DLG)
        SetShowFilename(true);
    qfilename = getOpenFileNameEx(initialFile, fltr);

    if(!qfilename.isEmpty())
    {
        filename.SetFromString(qfilename.toStdString());
    }
}

