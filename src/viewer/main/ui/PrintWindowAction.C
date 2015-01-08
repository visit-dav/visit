/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#include <PrintWindowAction.h>

#include <ViewerMessaging.h>
#include <ViewerState.h>
#include <ViewerText.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <DebugStream.h>
#include <PrinterAttributes.h>
#include <SaveWindowAttributes.h>

#include <avtFileWriter.h>
#include <avtImage.h>

#include <vtkQtImagePrinter.h>

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PrintWindowAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::OpenMDServerRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
PrintWindowAction::Execute()
{
    int windowIndex = GetWindow()->GetWindowId();

    //
    // Make sure that we have a printer name if we're printing to a
    // printer and make sure that we have a filename if we're printing
    // to a file.
    //
    if(GetViewerState()->GetPrinterAttributes()->GetOutputToFile())
    {
        if(GetViewerState()->GetPrinterAttributes()->GetOutputToFileName().empty())
        {
            GetViewerMessaging()->Error(
               TR("You cannot print the window to a file because you "
                  "did not specify an output filename."));
            return;
        }
    }
    else
    {
        if(GetViewerState()->GetPrinterAttributes()->GetPrinterName().empty())
        {
            GetViewerMessaging()->Error(
                TR("You cannot print the window because you have not "
                   "specified a printer name."));
            return;
        }
    }

    //
    // Send a status message about starting to save the image and make the
    // status message display for 10 minutes.
    //
    int index = windowIndex + 1;
    GetViewerMessaging()->Status(TR("Printing window %1...").arg(index), 6000000);

    //
    // Create a vtkQtImagePrinter object and set its printer attributes.
    //
    vtkQtImagePrinter *imagePrinter = vtkQtImagePrinter::New();
    QPrinter &printer = imagePrinter->printer();
    debug1 << "Setting printer attributes: " << endl;
    printer.setPrinterName(GetViewerState()->GetPrinterAttributes()->GetPrinterName().c_str());
    debug1 << "\tprinterName=" << GetViewerState()->GetPrinterAttributes()->GetPrinterName() << endl;

    if(!GetViewerState()->GetPrinterAttributes()->GetPrintProgram().empty())
    {
        printer.setPrintProgram(GetViewerState()->GetPrinterAttributes()->GetPrintProgram().c_str());
        debug1 << "\tprintProgram=" << GetViewerState()->GetPrinterAttributes()->GetPrintProgram() << endl;
    }

    printer.setCreator(GetViewerState()->GetPrinterAttributes()->GetCreator().c_str());
    debug1 << "\tcreator=" << GetViewerState()->GetPrinterAttributes()->GetCreator() << endl;

    printer.setDocName(GetViewerState()->GetPrinterAttributes()->GetDocumentName().c_str());
    debug1 << "\tdocName=" << GetViewerState()->GetPrinterAttributes()->GetDocumentName() << endl;

    printer.setNumCopies(GetViewerState()->GetPrinterAttributes()->GetNumCopies());
    debug1 << "\tnumCopies=" << GetViewerState()->GetPrinterAttributes()->GetNumCopies() << endl;

    printer.setOrientation(GetViewerState()->GetPrinterAttributes()->GetPortrait() ? QPrinter::Portrait :
        QPrinter::Landscape);
    debug1 << "\torientation="
           << (GetViewerState()->GetPrinterAttributes()->GetPortrait()?"portrait":"landscape") << endl;

    printer.setFromTo(1,1);
    debug1 << "\tfromTo=1,1" << endl;

    printer.setColorMode(GetViewerState()->GetPrinterAttributes()->GetPrintColor() ? QPrinter::Color :
        QPrinter::GrayScale);
    debug1 << "\tprintColor="
           << (GetViewerState()->GetPrinterAttributes()->GetPrintColor()?"color":"grayscale") << endl;

    if(GetViewerState()->GetPrinterAttributes()->GetOutputToFile())
    {
        printer.setOutputFormat(QPrinter::PdfFormat);
        debug1 << "\toutputFormat=PDF" << endl;
        printer.setOutputFileName(GetViewerState()->GetPrinterAttributes()->GetOutputToFileName().c_str());
        debug1 << "\toutputFilename=" << GetViewerState()->GetPrinterAttributes()->GetOutputToFileName() << endl;
    }
    else
    {
        printer.setOutputFileName(QString());
        debug1 << "\toutputFilename=(empty)" << endl;
    }
    printer.setPaperSize((QPrinter::PageSize)GetViewerState()->GetPrinterAttributes()->GetPageSize());
    debug1 << "\tpaperSize=" << GetViewerState()->GetPrinterAttributes()->GetPageSize() << endl;

    //
    // Create an image that will fit on the printer, else scale the
    // image to fit on the printer surface.
    //
    avtImage_p image = windowMgr->CreateSingleImage(windowIndex,
        GetViewerState()->GetSaveWindowAttributes()->GetWidth(), GetViewerState()->GetSaveWindowAttributes()->GetHeight(),
        GetViewerState()->GetSaveWindowAttributes()->GetScreenCapture(), true);

    //
    // Handle failure to obtain an image
    //
    if (*image == 0)
    {
        GetViewerMessaging()->Error(TR("Unable to obtain an image to print."));
        GetViewerMessaging()->Status(TR("Print from VisIt failed...."), 6000000);
        return;
    }

    //
    // Tell the imageWriter to use our writer to write the image. In this
    // case, the writer is an image printer.
    //
    avtDataObject_p dob;
    CopyTo(dob, image);
    avtFileWriter fileWriter;
    if(GetViewerState()->GetPrinterAttributes()->GetOutputToFile())
    {
        fileWriter.WriteImageDirectly(imagePrinter,
            GetViewerState()->GetPrinterAttributes()->GetOutputToFileName().c_str(), dob);
    }
    else
    {
        fileWriter.WriteImageDirectly(imagePrinter,
            GetViewerState()->GetPrinterAttributes()->GetDocumentName().c_str(), dob);
    }

    //
    // Delete the image printer.
    //
    imagePrinter->Delete();

    //
    // Indicate that the image has been printed.
    //
    if(GetViewerState()->GetPrinterAttributes()->GetOutputToFile())
    {
        GetViewerMessaging()->Status(
            TR("Window %1 saved to %2.").
            arg(index).
            arg(GetViewerState()->GetPrinterAttributes()->GetOutputToFileName()));
    }
    else
    {
        GetViewerMessaging()->Status(
            TR("Window %1 sent to printer.").arg(index));
    }
}
