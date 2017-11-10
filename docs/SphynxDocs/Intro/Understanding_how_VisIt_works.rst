.. _Understanding how VisIt works:

Understanding how VisIt_ works
------------------------------

VisIt's Core Abstractions
~~~~~~~~~~~~~~~~~~~~~~~~~

VisIt's interface is built around five core abstractions. These include:

VisIt_ visualizes data by creating one or more plots in a visualization window,
also known as a **Viewer** window. A plot is a visual representation of the data
being examined. Examples of plots include Mesh plots, Contour plots and
Pseudocolor plots. Plots take as input one or more scalar or vector variables,
which you can modify, by applying operators before passing them to a plot.
Examples of operators include arithmetic operations or taking slices through
the mesh. It is also possible to restrict the visualization of the data to
subsets of the mesh.

Databases read data from files and presents the data in the user interface
as variables. VisIt supports many different types of variables including:

* Meshes
* Scalars
* Vectors
* Tensors
* Materials
* Species

VisIt_'s architecture uses four main communicating components:

Scalars are single valued fields and examples include density, pressure
and temperature. Vectors are multi valued fields that have a direction
and magnitude. Examples include velocity and magnetic fields. Tensors are
multi valued fields that are typically thought of as 2 x 2 matrices in
the case of 2D data and 3 x 3 matrices in the case of 3D data. The typical
tensor variable is the stress tensor. Materials are a special type of
variable that associates one or more materials with a cell. The location
of the material is not specified within the cell and in the case of multi
material cells, algorithms must be used to determine where the material is
located in the cell, typically by looking at the materials in neighboring
cells. Species are variables that are associated with each material. For a
given material, species are a further breakdown of a material. The
distinctive property of a species is that it is uniformly distributed
throughout the material. For example, air consists of many different gases
such as oxygen, nitrogen, carbon monoxide, carbon dioxide, etc.

The **GUI**. The GUI is a VisIt_ client that provides
the user interface and menus that let you choose what to visualize. The 
**Viewer** displays all of the visualizations and is responsible for
keeping track of VisIt_'s state and coordinating this state with the other
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
:numref:`Figure %s <visit_architecture_image>` shows connections between VisIt_
components.

Operators take variables and modify them in some way. Operators perform
their operations before they are plotted. Multiple operators may be
applied to a variable forming a pipeline. For example, a mesh may be
subsetted so that all the values fall within a given range, furthermore,
the mesh may be subsetted to a portion of the mesh within a user specified
box.

Expressions perform calculations on variables to generate new variables.
Some common expressions consist of the standard mathematical operations
such as addition, subtraction, multiplication and division. It also includes
more complex operations such as gradient and divergence.

   VisIt_'s architecture
   
The **Viewer** supports up to 16 visualization windows. Each window is independent
of the others. VisIt_ uses an active window concept; all changes
made in **GUI Main Window** or one of its popup windows apply to the
currently active visualization window.

VisIt_ reads data and performs most of its processing in compute engine
processes. A compute engine is launched on each machine where data to be
visualized is located. The **Host Profiles Window** is used to specify
properties about the compute engines for different machines, such as the
number of processors to use when running the engine. The status of a
compute engine is displayed in the **Compute Engines Window**. The
**Compute Engines Window** can also be used to interrupt pending
operations.
