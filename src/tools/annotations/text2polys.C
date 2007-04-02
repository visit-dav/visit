// ************************************************************************* //
//                               text2polys.C                                //
// ************************************************************************* //

// ****************************************************************************
//  Program: text2polys
//
//  Purpose:
//      Takes in an input string and creates a .vtk file that contains
//      a polygon representation of the input string.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2002
//
// ****************************************************************************

#include <stdlib.h>

#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>

#include <arial_font.h>


// ****************************************************************************
//  Function: Usage
//
//  Purpose:
//      Explains the appropriate usage of text2polys and exits.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2002
//
// ****************************************************************************

void Usage(void)
{
    cerr << "Usage: text2polys <output-file-name> [-2D] TEXT" << endl;
    exit(EXIT_FAILURE);
}


// ****************************************************************************
//  Function: main
//
//  Purpose:
//      Parses the command line arguments and converts the desired string to 
//      polygonal data.  Writes that data out to a file.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2002
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    //
    // Parse the command line arguments.
    //
    if (argc < 3)
    {
        Usage();
    }
    char *output_file_name = argv[1];
    bool make3D = true;
    int first_arg = 2;
    if (strcmp(argv[2], "-2D") == 0)
    {
        if (argc == 3)
        {
            Usage();
        }
        first_arg = 3;
        make3D = false;
    }
    
    vtkPolyData *output = CreateText(argc-first_arg, argv+first_arg);
    if (make3D)
    {
        vtkPoints *pts = output->GetPoints();
        if (pts->GetNumberOfPoints() > 0)
        {
            double pt[3];
            pts->GetPoint(0, pt);
            pt[2] = 0.0001;
            pts->SetPoint(0, pt);
        }
    }

    //
    // Now write the output to a file.
    //
    vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
    writer->SetInput(output);
    writer->SetFileName(output_file_name);
    writer->Write();

    writer->Delete();
    output->Delete();
}


