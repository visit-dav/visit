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

// ----------------------------------------------------------------------------
// File:  DataSet.C
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#include "DataSet.h"
#include "Vector.h"
#include "Shape.h"

#include <visitstream.h>

DataSet::DataSet(ShapeType st, int sc)
{
    shapes.push_back(Shape(st, sc, this));

    xmin=-1;
    ymin=-1;
    zmin=-1;
    xmax= 1;
    ymax= 1;
    zmax= 1;
    xsize=2;
    ysize=2;
    zsize=2;

    selectedShape = 0;
    copyOfDataset = NULL;
}

void
DataSet::DrawPolyData(Vector &up, Vector &right)
{
    if (selectedShape > shapes.size())
        selectedShape = 0;

    if (selectedShape > 0)
    {
        shapes[0].DrawPolyData(up, right);
        shapes[selectedShape].DrawPolyData(up, right);
    }
    else
    {
        for (int i=0; i<shapes.size(); i++)
        {
            shapes[i].DrawPolyData(up, right);
        }
    }
}

void
DataSet::ReInit()
{
    if (copyOfDataset)
    {
        shapes.resize(copyOfDataset->shapes.size());
        for (int i=1; i<copyOfDataset->shapes.size(); i++)
        {
            shapes[i] = Shape(&copyOfDataset->shapes[i],
                              transformNumber,
                              &shapes[0],
                              this);
        }
    }

    for (int i=1; i<shapes.size(); i++)
    {
        shapes[i].parentShape = &shapes[0];
        shapes[i].Init();
    }

    selectedShape = 0;
}
