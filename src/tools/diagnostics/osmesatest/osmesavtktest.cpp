// This junk is needed to make VTK work on startup.
#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkCamera.h>
#include <vtkCylinderSource.h>
#include <vtkDepthSortPolyData.h>
#include <vtkImageMapper.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTexture.h>
#include <vtkTexturedSphereSource.h>
#include <vtkWindowToImageFilter.h>

#include <vtkVisItOSMesaRenderingFactory.h>
#include <Utility.h>

#include <stdio.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

int
main(int argc, char *argv[])
{
    const char *default_filename = "image.png";
    const char *barrier = NULL;
    char filename[1000];
    bool interactive = false;

    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-interactive") == 0)
            interactive = true;
        else if(strcmp(argv[i], "-output") == 0 && (i+1)<argc)
        {
            default_filename = argv[i+1];
            ++i;
        }
        else if(strcmp(argv[i], "-barrier") == 0 && (i+1)<argc)
        {
            barrier = argv[i+1];
            ++i;
        }
    }
#ifdef PARALLEL
    /* Init MPI */
    MPI_Init(&argc, &argv);
    int par_rank = 0, par_size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &par_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &par_size);
    sprintf(filename, "%s.%d.png", default_filename, par_rank);
#else
    strcpy(filename, default_filename);
#endif

    if(barrier != NULL)
        WaitUntilFile(barrier);

#if 1
    // Just including this in the shared library build version of the program causes it to bail!
    if(!interactive)
    {
        // Force Mesa rendering classes.
        printf("Forcing Mesa classes.\n");
        vtkVisItOSMesaRenderingFactory::ForceMesa();
    }
#endif

    // This creates a polygonal cylinder model with eight circumferential facets.
    //
    vtkCylinderSource *cylinder = vtkCylinderSource::New();
    cylinder->SetResolution(8);

    // The mapper is responsible for pushing the geometry into the graphics
    // library. It may also do color mapping, if scalars or other attributes
    // are defined.
    //
    vtkPolyDataMapper *cylinderMapper = vtkPolyDataMapper::New();
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

    // The actor is a grouping mechanism: besides the geometry (mapper), it
    // also has a property, transformation matrix, and/or texture map.
    // Here we set its color and rotate it -22.5 degrees.
    vtkActor *cylinderActor = vtkActor::New();
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->GetProperty()->SetColor(1.0000, 0.3882, 0.2784);
    cylinderActor->RotateX(30.0);
    cylinderActor->RotateY(-45.0);

    // Create the graphics structure. The renderer renders into the
    // render window. The render window interactor captures mouse events
    // and will perform appropriate camera or actor manipulation
    // depending on the nature of the events.
    //
    vtkRenderer *ren1 = vtkRenderer::New();
    vtkRenderWindow *renWin = vtkRenderWindow::New();
    renWin->AddRenderer(ren1);
    vtkRenderWindowInteractor *iren = NULL;
    if(interactive)
    {
        iren = vtkRenderWindowInteractor::New();
        iren->SetRenderWindow(renWin);
    }

    // Add the actors to the renderer, set the background and size
    //
    ren1->AddActor(cylinderActor);
    ren1->SetBackground(0.1, 0.2, 0.4);
    renWin->SetSize(800, 800);

    // We'll zoom in a little by accessing the camera and invoking a "Zoom"
    // method on it.
    ren1->ResetCamera();
    ren1->GetActiveCamera()->Zoom(1.5);
    printf("Rendering\n");
    renWin->Render();

    // This starts the event loop and as a side effect causes an initial render.
    if(interactive)
    {
        printf("Starting interactive.\n");
        iren->Start();
    }
    else
    {
        vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = 
            vtkSmartPointer<vtkWindowToImageFilter>::New();
        windowToImageFilter->SetInput(renWin);
        windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
        windowToImageFilter->ReadFrontBufferOff(); // read from the back buffer
        windowToImageFilter->Update();
 
        printf("Made it past windowToImageFilter.\n");

        vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
        writer->SetFileName(filename);
        writer->SetInputConnection(windowToImageFilter->GetOutputPort());
        writer->Write();

        printf("Made it past vtkPNGWriter. filename=%s\n", filename);
    }

    // Exiting from here, we have to delete all the instances that
    // have been created.
    cylinder->Delete();
    cylinderMapper->Delete();
    cylinderActor->Delete();
    ren1->Delete();
    renWin->Delete();
    if(iren != NULL)
        iren->Delete();

#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}
