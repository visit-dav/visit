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

#include <QvisAnnotationObjectInterfaceFactory.h>
#include <QvisImageAnnotationInterface.h>
#include <QvisLine2DInterface.h>
#include <QvisText2DInterface.h>
#include <QvisTimeSliderInterface.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: QvisAnnotationObjectInterfaceFactory::QvisAnnotationObjectInterfaceFactory
//
// Purpose: 
//   Constructor for the QvisAnnotationObjectInterfaceFactory class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 09:31:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisAnnotationObjectInterfaceFactory::QvisAnnotationObjectInterfaceFactory()
{
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterfaceFactory::~QvisAnnotationObjectInterfaceFactory
//
// Purpose: 
//   Destructor for the QvisAnnotationObjectInterfaceFactory class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 09:31:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisAnnotationObjectInterfaceFactory::~QvisAnnotationObjectInterfaceFactory()
{
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterfaceFactory::GetMaxInterfaces
//
// Purpose: 
//   Returns the maximum number of interfaces that the factory creates.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 09:31:50 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 13:32:58 PST 2005
//   Made it return 8.
//
// ****************************************************************************

int
QvisAnnotationObjectInterfaceFactory::GetMaxInterfaces() const
{
    return 8;
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterfaceFactory::CreateInterface
//
// Purpose: 
//   Creates an interface and returns it.
//
// Arguments:
//   i      : The index of the interface to create.
//   parent : The parent widget for the interface.
//
// Returns:    A pointer to the new interface or 0 if no interface was created.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 09:32:56 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 12:09:55 PDT 2005
//   Added John Anderson's image and line2d annotation interfaces.
//
// ****************************************************************************

QvisAnnotationObjectInterface *
QvisAnnotationObjectInterfaceFactory::CreateInterface(int i, QWidget *parent) const
{
    QvisAnnotationObjectInterface *retval = 0;

    switch(i)
    {
    case 0: // Text2D
        retval = new QvisText2DInterface(parent, "text2DInterface");
        break;
    case 2: // TimeSlider;
        retval = new QvisTimeSliderInterface(parent, "timeSliderInterface");
        break;
    case 3: // Line2D
        retval = new QvisLine2DInterface(parent, "line2DInterface");
        break;
    // Arrow2D
    // Arrow3D
    // Box
    case 7: // Image
        retval = new QvisImageAnnotationInterface(parent, "imageAnnotationInterface");
        break;
    default:
        debug1 << "QvisAnnotationObjectInterfaceFactory::CreateInterface:"
               << " invalid index=" << i << endl;
        break;
    }

    return retval;
}
