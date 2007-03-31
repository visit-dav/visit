// ****************************************************************************
//  Program: time_annotation
//
//  Purpose:
//      Creates a display that indicates the time in text and in relative
//      proportion.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2002
//
//  Modifications:
//
//    Hank Childs, Thu May 15 15:52:52 PDT 2003
//    Added -precision and -units options.  Made the program run windowless.
//
// ****************************************************************************

#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkGraphicsFactory.h>
#include <vtkImageData.h>
#include <vtkImagingFactory.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkScaledTextActor.h>
#include <vtkTextMapper.h>
#include <vtkTIFFWriter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkWindowToImageFilter.h>

#include <arial_font.h>

static void ParseArguments(int, char **);
static void Help(void);


static unsigned char   bg[3] = { 255, 255, 255 };
static unsigned char   fg[3] = { 0, 0, 0 };

static unsigned char   c1[3] = { 255, 0, 0 };
static unsigned char   c2[3] = { 0, 0, 255 };

static int             size = 150;

static int             precision = -1;
static char           *units = NULL;

static bool            no_time_bounds = false;
static bool            no_current_time = false;

static char           *start_time = NULL;
static char           *stop_time  = NULL;
static char           *current_time = NULL;
static double          proportion = 0.;

static char           *output_file = NULL;

   
int 
main(int argc, char *argv[])
{
    ParseArguments(argc, argv);

#if !defined(_WIN32)
    vtkGraphicsFactory::SetUseMesaClasses(1);
    vtkImagingFactory::SetUseMesaClasses(1);
#endif

    int width = 2*size;
    int height = size;

    //
    // Set up the render window.
    //
    vtkRenderer *ren = vtkRenderer::New();
    vtkRenderWindow *renwin = vtkRenderWindow::New();
    renwin->SetPolygonSmoothing(1); // Anti-aliasing
    renwin->SetLineSmoothing(1); // Anti-aliasing
    renwin->SetSize(width, height);
    renwin->SetOffScreenRendering(1);
    renwin->AddRenderer(ren);
    ren->SetRenderWindow(renwin);
    ren->SetBackground(bg[0]/255., bg[1]/255., bg[2]/255.);
 
    //
    // Add the annotations -- start time, stop time, current time.
    //
    if (!no_time_bounds)
    {
        vtkPolyDataMapper *start_mapper = vtkPolyDataMapper::New();
        start_mapper->SetInput(CreateText(1, &start_time));
        vtkActor *start_actor = vtkActor::New();
        start_actor->SetMapper(start_mapper);
        start_actor->GetProperty()->SetColor(fg[0]/255.,fg[1]/255.,fg[2]/255.);
        start_actor->SetPosition(-6, -1, 0);

        vtkPolyDataMapper *stop_mapper = vtkPolyDataMapper::New();
        stop_mapper->SetInput(CreateText(1, &stop_time));
        vtkActor *stop_actor = vtkActor::New();
        stop_actor->SetMapper(stop_mapper);
        stop_actor->GetProperty()->SetColor(fg[0]/255., fg[1]/255.,fg[2]/255.);
        stop_actor->SetPosition(+6, -1, 0);

        ren->AddActor(start_actor);
        ren->AddActor(stop_actor);
    }

    if (!no_current_time)
    {
        vtkPolyDataMapper *current_mapper = vtkPolyDataMapper::New();
        char str[1024];
        char time_str[1024];
        if (precision > 0)
        {
            double ct = atof(current_time);
            if (ct != 0. || current_time[0] == '0')
            {
                char format[16];
                sprintf(format, "%%%dg", precision);
                sprintf(time_str, format, ct);
            }
            else
            {
                strcpy(time_str, current_time);
            }
        }
        else
        {
            strcpy(time_str, current_time);
        }
        char units_str[1024] = { '\0' };
        if (units != NULL)
        {
            sprintf(units_str, " %s", units);
        }
            
        sprintf(str, "Time: %s%s", time_str, units_str);
        char *stupid_warning = str; // For picky compilers.
        current_mapper->SetInput(CreateText(1, &stupid_warning));
        vtkActor *current_actor = vtkActor::New();
        current_actor->SetMapper(current_mapper);
        current_actor->GetProperty()->SetColor(fg[0]/255., fg[1]/255., 
                                               fg[2]/255.);
        current_actor->SetPosition(-6, -3, 0);

        ren->AddActor(current_actor);
    }

    if (!no_current_time || !no_time_bounds)
    {
        // This dummy actor is only so the rest of the text will be positioned
        // correctly by the VTK camera.  It is purposefully colored the
        // background color.
        vtkPolyDataMapper *dummy_mapper = vtkPolyDataMapper::New();
        char *dummy_str = "_";
        dummy_mapper->SetInput(CreateText(1, &dummy_str));
        vtkActor *dummy_actor = vtkActor::New();
        dummy_actor->SetMapper(dummy_mapper);
        dummy_actor->GetProperty()->SetColor(bg[0]/255.,bg[1]/255.,bg[2]/255.);
        dummy_actor->SetPosition(+6, 3, 0);

        ren->AddActor(dummy_actor);
    }

    renwin->Render();
    ren->GetActiveCamera()->Zoom(2.6);  // 2.6 determined empirically

    //
    // Capture the image of the annotation.  We will put the color bar in
    // manually.
    //
    vtkWindowToImageFilter *w2if = vtkWindowToImageFilter::New();
    w2if->SetInput(renwin);
    vtkImageData *image = w2if->GetOutput();
    renwin->Render();
    image->Update();
    vtkUnsignedCharArray *uchar = (vtkUnsignedCharArray *) 
                                           image->GetPointData()->GetScalars();
    unsigned char *ptr = uchar->GetPointer(0);

    //
    // Now add the bar with colors.
    //
    int top_border    = (int) (height*0.8);
    int bottom_border = (int) (height*0.5);
    int left_border   = (int) (width*0.1);
    int right_border  = (int) (width*0.9);
    int middle = (int)(proportion*(right_border-left_border)) + left_border;
    float middle_x = (left_border + right_border) / 2.;
    float middle_y = (bottom_border + top_border) / 2.;
    int i, j;
    for (i = left_border ; i < middle ; i++)
    {
        for (j = bottom_border ; j < top_border ; j++)
        {
            unsigned char *p = ptr + 3*(j*width + i);
            int horizontal_dist = (i > middle_x ? right_border - i
                                                : i - left_border);
            int vertical_dist = (j > middle_y ? top_border - j
                                              : j - bottom_border);
            float prop = (horizontal_dist / middle_x) *
                         (vertical_dist / middle_y);
            prop = sqrt(prop);
            prop += 0.7;
            if (prop > 1)
               prop = 1.;
            p[0] = (unsigned char) (prop*c1[0] + (1-prop)*fg[0]); 
            p[1] = (unsigned char) (prop*c1[1] + (1-prop)*fg[1]); 
            p[2] = (unsigned char) (prop*c1[2] + (1-prop)*fg[2]); 
        }
    }
    for (i = middle ; i < right_border ; i++)
    {
        for (j = bottom_border ; j < top_border ; j++)
        {
            unsigned char *p = ptr + 3*(j*width + i);
            int horizontal_dist = (i > middle_x ? right_border - i
                                                : i - left_border);
            int vertical_dist = (j > middle_y ? top_border - j
                                              : j - bottom_border);
            float prop = (horizontal_dist / middle_x) *
                         (vertical_dist / middle_y);
            prop = sqrt(prop);
            prop += 0.7;
            if (prop > 1)
               prop = 1.;
            p[0] = (unsigned char) (prop*c2[0] + (1-prop)*fg[0]); 
            p[1] = (unsigned char) (prop*c2[1] + (1-prop)*fg[1]); 
            p[2] = (unsigned char) (prop*c2[2] + (1-prop)*fg[2]); 
        }
    }
    for (i = left_border ; i < right_border ; i++)
    {
        unsigned char *p = ptr + 3*((top_border)*width + i);
        p[0] = fg[0]; 
        p[1] = fg[1]; 
        p[2] = fg[2]; 
        p = ptr + 3*((bottom_border)*width + i);
        p[0] = fg[0]; 
        p[1] = fg[1]; 
        p[2] = fg[2]; 
    }
    for (i = bottom_border ; i < top_border ; i++)
    {
        unsigned char *p = ptr + 3*(i*width + left_border);
        p[0] = fg[0]; 
        p[1] = fg[1]; 
        p[2] = fg[2]; 
        p = ptr + 3*(i*width + middle);
        p[0] = fg[0]; 
        p[1] = fg[1]; 
        p[2] = fg[2]; 
        p = ptr + 3*(i*width + right_border);
        p[0] = fg[0]; 
        p[1] = fg[1]; 
        p[2] = fg[2]; 
    }
    // We ended up missing on pixel, at (right_border, top_border)
    unsigned char *p = ptr + 3*(top_border*width + right_border);
    p[0] = fg[0]; 
    p[1] = fg[1]; 
    p[2] = fg[2]; 
    
    //
    // Write out the TIFF.
    //
    vtkTIFFWriter *wrtr = vtkTIFFWriter::New();
    wrtr->SetInput(image);
    wrtr->SetFileName(output_file);
    wrtr->Write();
}


// ****************************************************************************
//  Function: ParseArguments
//
//  Purpose:
//      Parses the arguments passed in on the command line.  Sets global
//      variables when flags are passed.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2002
//
//  Modifications:
//
//    Hank Childs, Thu May 15 15:43:21 PDT 2003
//    Added arguments -units and -precision.
//
// ****************************************************************************

static void
ParseArguments(int argc, char *argv[])
{
    if (argc < 5)
    {
        Help();
    }

    for (int i = 1 ; i < argc-4 ; i++)
    {
        if (strcmp(argv[i], "-bg") == 0)
        {
            bg[0] = atoi(argv[i+1]);
            bg[1] = atoi(argv[i+2]);
            bg[2] = atoi(argv[i+3]);
            i += 3;
        }
        else if (strcmp(argv[i], "-fg") == 0)
        {
            fg[0] = atoi(argv[i+1]);
            fg[1] = atoi(argv[i+2]);
            fg[2] = atoi(argv[i+3]);
            i += 3;
        }
        else if (strcmp(argv[i], "-c1") == 0)
        {
            c1[0] = atoi(argv[i+1]);
            c1[1] = atoi(argv[i+2]);
            c1[2] = atoi(argv[i+3]);
            i += 3;
        }
        else if (strcmp(argv[i], "-c2") == 0)
        {
            c2[0] = atoi(argv[i+1]);
            c2[1] = atoi(argv[i+2]);
            c2[2] = atoi(argv[i+3]);
            i += 3;
        }
        else if (strcmp(argv[i], "-size") == 0)
        {
            size = atoi(argv[i+1]);
            i += 1;
        }
        else if (strcmp(argv[i], "-no_time_bounds") == 0)
        {
            no_time_bounds = true;
        }
        else if (strcmp(argv[i], "-no_current_time") == 0)
        {
            no_current_time = true;
        }
        else if (strcmp(argv[i], "-precision") == 0)
        {
            precision = atoi(argv[i+1]);
            i += 1;
        }
        else if (strcmp(argv[i], "-units") == 0)
        {
            units = new char[strlen(argv[i+1])+1];
            strcpy(units, argv[i+1]);
            i += 1;
        }
        else
        {
            cerr << "Unrecognized flag: \"" << argv[i] << "\"." << endl;
            Help();
        }
    }

    double start_time_num   = atof(argv[argc-4]);
    double stop_time_num    = atof(argv[argc-3]);
    double current_time_num = atof(argv[argc-2]);

    if (stop_time_num <= start_time_num)
    {
        cerr << "Stop time cannot be greater than start time." << endl;
        Help();
    }
    if (current_time_num < start_time_num)
    {
        cerr << "Current time cannot be less than start time." << endl;
        Help();
    }
    if (current_time_num > stop_time_num)
    {
        cerr << "Current time cannot be more than stop time." << endl;
        Help();
    }

    proportion = (current_time_num - start_time_num) / 
                 (stop_time_num - start_time_num);
    start_time = new char[strlen(argv[argc-4])+1];
    strcpy(start_time, argv[argc-4]);

    stop_time = new char[strlen(argv[argc-3])+1];
    strcpy(stop_time, argv[argc-3]);

    current_time = new char[strlen(argv[argc-2])+1];
    strcpy(current_time, argv[argc-2]);

    output_file = new char[strlen(argv[argc-1])+1];
    strcpy(output_file, argv[argc-1]);
}


// ****************************************************************************
//  Function: Help
//
//  Purpose:
//      Prints the acceptable arguments and exits.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2002
//
//  Modifications:
//
//    Hank Childs, Thu May 15 15:43:21 PDT 2003
//    Remove unused flag '-h'.  Added precision and units.
//
// ****************************************************************************

static void
Help(void)
{
    cerr << "\n\nUsage: <time_annotation> [-bg color] [-fg color] [-c1 color] "
         << "[-c2 color]\n\t\t[-size int] [-no_time_bounds] "
         << "[-no_current_time]\n"
         << "\t\t[-precision int] [-units string]\n"
         << "\t\t<start_time> <stop_time> <currrent_time> "
         << "<output_file_name>" << endl;
    cerr << "Flags:\n";
    cerr << "\t-bg\t\t\tBackground color         [default = white]\n";
    cerr << "\t-fg\t\t\tForeground color         [default = black]\n";
    cerr << "\t-c1\t\t\tFill color 1 (on left)   [default = red]\n";
    cerr << "\t-c2\t\t\tFill color 2 (on right)  [default = blue]\n";
    cerr << "\t-size\t\t\tSize -- [2s x s]        [default = 300]\n";
    cerr << "\t-no_time_bounds\t\tDo not display the bounds of the time.\n";
    cerr << "\t-no_current_time\tDo not display the current time.\n";
    cerr << "\t-precision\t\tThe precision for the current time.\n";
    cerr << "\t-units\t\t\tDisplay units of time (example: seconds).\n";
    cerr << "\t<start_time>\t\tThe start time of the simulation.\n";
    cerr << "\t<stop_time>\t\tThe stop time of the simulation.\n";
    cerr << "\t<current_time>\t\tThe current time of the simulation.\n";
    cerr << "\t<output_file_name>\tThe name to give to the output TIFF.\n";
    cerr << "\nAll colors are specified as 3-tuples of unsigned characters.\n";
    cerr << endl;
    exit(EXIT_FAILURE);
}


