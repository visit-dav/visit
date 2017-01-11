.. _Understanding how VisIt works:

Understanding how VisIt works
-----------------------------

.. danger::

   Change to 5-building blocks verbiage? (Database, Plots, Operators, Expressions and Queries )

VisIt visualizes data by creating one or more plots in a visualization window,
also known as a vis window. A plot is a visual representation of the data
being examined. Examples of plots include Mesh plots, Contour plots and
Pseudocolor plots. Plots take as input one or more scalar or vector variables,
which you can modify, by applying operators before passing them to a plot.
Examples of operators include arithmetic operations or taking slices through
the mesh. It is also possible to restrict the visualization of the data to
subsets of the mesh.

.. danger::

   Some of this is focused on talking about the main window, some about the arch, we should clarify

VisIt's architecture uses four main communicating components:

* GUI (gui)
* Viewer (viewer)
* Compute Engine (engine_ser or engine_par)
* MetaData Server (mdserver)

The **GUI**. The GUI is a VisIt client that provides
the user interface and menus that let you choose what to visualize. The 
**Viewer** displays all of the visualizations and is responsible for
keeping track of VisIt's state and coordinating this state with the other
components. Both the GUI and the Viewer are meant to run locally 
to advantage of the local computer's graphics hardware. 
The next two components can also be run on a client computer but
they are more often run on a remote, parallel computer or cluster where the
data files are generated. 
The the **MetaData Server**
is responsible for reading the remote file system and passing
information about remote files to the GUI on the client computer. The
**MetaData Server** also opens files to determine the variables and
other metadata that are useful in creating visualizations. Finally, the
**Compute Engine** is the component that actually reads the data files,
executes processing pipelines, and sends back either images or geometry to be drawn by the
**Viewer** using the clients local graphics hardware.
:numref:`Figure %s <VisIts_architecture>` shows connections between VisIt
components.

.. _VisIts_architecture:

.. figure:: images/vis_win.png

   VisIt's architecture
   
The **Viewer** supports up to 16 visualization windows. Each window is independent
of the others. VisIt uses an active window concept; all changes
made in **GUI Main Window** or one of its popup windows apply to the
currently active visualization window.

VisIt reads data and performs most of its processing in compute engine
processes. A compute engine is launched on each machine where data to be
visualized is located. The **Host Profiles Window** is used to specify
properties about the compute engines for different machines, such as the
number of processors to use when running the engine. The status of a
compute engine is displayed in the **Compute Engines Window**. The
**Compute Engines Window** can also be used to interrupt pending
operations.
