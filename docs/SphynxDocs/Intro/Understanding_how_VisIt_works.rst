Understanding how VisIt works
-----------------------------

VisIt visualizes data by creating one or more plots in a visualization window,
also known as a vis window. A plot is a visual representation of the data
being examined. Examples of plots include Mesh plots, Contour plots and
Pseudocolor plots. Plots take as input one or more scalar or vector variables,
which you can modify, by applying operators before passing them to a plot.
Examples of operators include arithmetic operations or taking slices through
the mesh. It is also possible to restrict the visualization of the data to
subsets of the mesh.

VisIt supports up to 16 visualization windows. Each vis window is independent
of the other vis windows. VisIt uses an active window concept; all changes
made in VisIt's **Main Window** or one of its popup windows apply to the
currently active vis window.

VisIt reads its data and performs most of its processing in compute engine
processes. A compute engine is launched on each machine where data to be
visualized is located. The **Host Profiles Window** is used to specify
properties about the compute engines for different machines, such as the
number of processors to use when running the engine. The status of a
compute engine is displayed in the **Compute Engines Window**. The
**Compute Engines Window** can also be used to interrupt pending
operations.

VisIt's architecture can be broken down into four main components, though
there are some other less important components. The first component, and the
one covered by this document, is the GUI. The GUI provides the user
interface and menus that let you easily choose what to visualize. The Viewer
displays all of the visualizations in its vis windows and is responsible for
keeping track of VisIt's state and for talking to the rest of VisIt's
components. Both the GUI and the Viewer are meant to run on the local client
computer so they can take advantage of the client computer's fast graphics
hardware. The next two components can also be run on the client computer but
they are more often run on a remote, parallel computer or cluster where the
data files were generated. The first such component is the database server,
which is responsible for reading the remote file system and passing
information about the files there to the GUI on the local computer. The
database server also opens a file to determine its list of variables and
other metadata that are useful in creating visualizations. Finally, the
compute engine is the component that actually reads the data files,
processes them, and sends back either images or geometry to be drawn by the
viewer using the local computer's fast graphics hardware.
:numref:`Figure %s <VisIts_architecture>` shows connections between VisIt
components.

.. _VisIts_architecture:

.. figure:: images/vis_win.png

   VisIt's architecture
