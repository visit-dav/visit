/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

// ************************************************************************* //
//                               avtVTKWriter.C                              //
// ************************************************************************* //

#include <avtVTKWriter.h>

#include <vector>

#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtVTKWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2005
//
//  Modifications:
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Initialize doMultiBlock.
//
//    Jeremy Meredith, Tue Mar 27 15:10:21 EDT 2007
//    Added nblocks so we don't have to trust the meta data.
//
// ****************************************************************************

avtVTKWriter::avtVTKWriter(DBOptionsAttributes *atts)
{
    doBinary = atts->GetBool("Binary format");
    doMultiBlock = true;
    nblocks = 0;
}


// ****************************************************************************
//  Method: avtVTKWriter::OpenFile
//
//  Purpose:
//      Does no actual work.  Just records the stem name for the files.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 27 15:10:21 EDT 2007
//    Added nblocks to this function and save it so we don't have to 
//    trust the meta data.
// ****************************************************************************

void
avtVTKWriter::OpenFile(const string &stemname, int nb)
{
    stem = stemname;
    nblocks = nb;
}


// ****************************************************************************
//  Method: avtVTKWriter::WriteHeaders
//
//  Purpose:
//      Writes out a VisIt file to tie the VTK files together.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Add support for curves.
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Initialize doMultiBlock.
//
//    Jeremy Meredith, Tue Mar 27 11:36:57 EDT 2007
//    Use the saved nblocks because we can't trust the meta data.
//
// ****************************************************************************

void
avtVTKWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           vector<string> &scalars, vector<string> &vectors,
                           vector<string> &materials)
{
    doMultiBlock = (nblocks > 1);
    if (nblocks > 1)
    {
        char filename[1024];
        sprintf(filename, "%s.visit", stem.c_str());
        ofstream ofile(filename);
        ofile << "!NBLOCKS " << nblocks << endl;
        for (int i = 0 ; i < nblocks ; i++)
        {
            char chunkname[1024];
            sprintf(chunkname, "%s.%d.vtk", stem.c_str(), i);
            ofile << chunkname << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtVTKWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//
//    Hank Childs, Thu May 26 17:23:39 PDT 2005
//    Add support for binary writes through DB options.
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Change name based on whether or not we are doing multi-block.
//
// ****************************************************************************

void
avtVTKWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    char chunkname[1024];
    if (doMultiBlock)
        sprintf(chunkname, "%s.%d.vtk", stem.c_str(), chunk);
    else
        sprintf(chunkname, "%s.vtk", stem.c_str());
    vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
    if (doBinary)
        wrtr->SetFileTypeToBinary();
    wrtr->SetInput(ds);
    wrtr->SetFileName(chunkname);
    wrtr->Write();

    wrtr->Delete();
}


// ****************************************************************************
//  Method: avtVTKWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
// ****************************************************************************

void
avtVTKWriter::CloseFile(void)
{
    // Just needed to meet interface.
}


