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
// ****************************************************************************

avtVTKWriter::avtVTKWriter(DBOptionsAttributes *atts)
{
    doBinary = atts->GetBool("Binary format");
    doMultiBlock = true;
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
// ****************************************************************************

void
avtVTKWriter::OpenFile(const string &stemname)
{
    stem = stemname;
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
// ****************************************************************************

void
avtVTKWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           vector<string> &scalars, vector<string> &vectors,
                           vector<string> &materials)
{
    int nblocks = 1;
    if (md->GetNumMeshes() > 0)
        nblocks = md->GetMesh(0)->numBlocks;
    // else probably a curve.
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


