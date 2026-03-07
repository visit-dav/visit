.. _Glossary:

Glossary
========

.. glossary::

   AAN
   Always, Auto, Never
       Spanish: **siempre, automático, nunca**
       Various features in VisIt_ support an **Always, Auto, Never** choice.
       A setting of **Never** means to never enable the the feature and a
       setting of **Always** means to always enable the feature. A setting
       of **Auto**, which is typically the default, means the allow VisIt_ 
       to decide when it thinks it is best to enable or disable the feature.
    
   Integral Curve
       Spanish: **curva integral**
       An integral curve is a curve that begins at a seed location and is
       tangent at every point in a vector field. It is computed by numerical
       integration of the seed location through the vector field.

   Node
   Point
   Vertex
       Spanish: **nodo, punto, vértice**
       These terms refer to the *corners* or *ends* of mesh elements.

   Pathlines
       Spanish: **líneas de trayectoria**
       A path rendered by an integrator that uses the vector field that is
       *in-step* with the integrator, so that as the integrator steps through
       time, it uses data from the vector field at each new time step.

   Node-centered
   Point-centered
       Spanish: **centrado en nodos / centrado en puntos**
       These terms refer to a piecewise-linear (one degree of freedom at each
       of mesh element *corner*) interpolation scheme used to define a variable
       on a mesh. VTK tends to use the *point* terminology whereas VisIt_ tends
       to use the *node* terminology.

   Parallel task
       Spanish: **tarea paralela**
       Although developers are working to enhance VisIt_ to support a variety
       of fine-grained parallelism methods (e.g. MC or GPU) and although some
       portions of VisIt_ have supported multi-threaded processing for several
       years, in the currently available implementations, a parallel task is an
       :abbr:`MPI (Message Passing Interface)` rank.

   Streamlines
       Spanish: **líneas de corriente**
       A path rendered by an integrator that uses the *same* vector field for
       the entire integration.

   SIL
   Subset Inclusion Lattice
       Spanish: **retículo de inclusión de subconjuntos**
       A **Subset Inclusion Lattice** or **SIL** is a term used to describe
       the often complex, graph like relationships among a variety of subsets
       defined for a mesh. A **SIL** describes which subsets and categories
       of subsets are contained within other subsets and subset categories.
       The **Subset Window** is the part of VisIt_ GUI that displays the
       contents of a **SIL** and allows the user to browse subsets and
       subset categories and turn subsets (and trees of subsets) on and off
       in visualizations.

   SR
   SR mode
       Spanish: **SR / modo SR**
       SR is an abbreviation for **Scalable Rendering**. This is a mode of
       operation where the VisIt_ **engine** performs scalable, parallel
       rendering and ships the final rendered image (e.g. pixels) to the
       **viewer**. This is in contrast to *standard* mode where the
       **engine** ships polygons to the **viewer** to be rendered there.

   Zone
   Cell
       Spanish: **zona, celda**
       These terms refer to the the individual computational elements comprising
       a mesh.

   Zone-centered
   Cell-centered
       Spanish: **centrado en zonas / centrado en celdas**
       These terms refer to a piecewise-constant (single degree of freedom for
       an entire zone) interpolation scheme used to define a field variable on
       a mesh. VTK tends to use the *cell* terminology whereas VisIt_ tends to
       use the *zone* terminology.

   Annotation objects
       Spanish: **objetos de anotación**
       Text, axes, legends, and other markup added to visualizations.

   Attributes
       Spanish: **atributos**
       Generated state/setting objects used by VisIt_ (e.g. for plots/operators),
       often with C++/Python/Java bindings.

   Backward compatibility
       Spanish: **compatibilidad hacia atrás**
       Keeping older session files and scripts working across newer VisIt_ versions.

   CLI
   Command Line Interface
       Spanish: **interfaz de línea de comandos**
       A text-based interface to control VisIt_, commonly via Python scripting.

   Client
       Spanish: **cliente**
       A VisIt_ front-end process (e.g. GUI, CLI, Java) that controls visualization state.

   Client-server mode
       Spanish: **modo cliente-servidor**
       Running the user interface locally while launching VisIt_ components (mdserver/engine)
       on a remote machine where the data reside.

   Color table
       Spanish: **tabla de colores**
       A named colormap used by plots (e.g. mapping scalar values to colors).

   Compute engine
       Spanish: **motor de cómputo**
       The VisIt_ component that reads problem-sized data and executes the visualization pipeline.

   Database
       Spanish: **base de datos**
       A data source opened by VisIt_ (often a file or set of files) that exposes variables.

   Database correlation
       Spanish: **correlación de bases de datos**
       A mapping that relates multiple databases so they can be compared or co-visualized.

   Expression
       Spanish: **expresión**
       A formula that computes a derived variable from existing variables.

   GUI
   Graphical User Interface
       Spanish: **interfaz gráfica de usuario**
       The main interactive graphical client used to build plots, apply operators, and manage state.

   Host profile
       Spanish: **perfil de host**
       A saved configuration describing how VisIt_ launches components on a given machine.

   Interactor
       Spanish: **controlador de interacción**
       The interaction mode/settings that determine how mouse/keyboard actions manipulate the view.

   Lineout
       Spanish: **perfil lineal**
       Extracting a curve by sampling values along a line through the data.

   LineSampler
       Spanish: **muestreador de línea**
       Sampling data along a line or related geometry (commonly used for lineouts and profiles).

   Material
       Spanish: **material**
       A variable describing which material(s) occupy a mesh zone; may be multi-material.

   mdserver
   Metadata server
       Spanish: **servidor de metadatos**
       The VisIt_ component that opens/browses databases to obtain metadata (meshes, variables, etc.).

   Mesh
       Spanish: **malla**
       The discretization of space into elements (zones/cells) on which variables are defined.

   MPI rank
       Spanish: **rango MPI**
       One process in an MPI-parallel run; often what VisIt_ documentation calls a parallel task.

   Operator
       Spanish: **operador**
       A data transformation applied before plotting (operators can be chained).

   Parallel compute engine
       Spanish: **motor de cómputo paralelo**
       A compute engine running across multiple MPI ranks to process large datasets.

   Pick
       Spanish: **sondeo (pick)**
       An interactive query/probe used to retrieve information about data at a selected location.

   Pipeline
       Spanish: **tubería (pipeline)**
       The sequence of operators and plot processing steps applied to data.

   Plot
       Spanish: **gráfico (plot)**
       A visualization mapping from variables to rendered geometry/images (e.g. Pseudocolor plot).

   Plot plugin
       Spanish: **plugin de gráfico**
       A plugin that adds a new plot type to VisIt_.

   Plugin
       Spanish: **plugin (complemento)**
       An extension module that adds capabilities such as new file readers, plots, or operators.

   Preferences
       Spanish: **preferencias**
       Persistent user settings controlling defaults and behavior.

   Query
       Spanish: **consulta**
       A computation that summarizes data (single values, small tables, or generated curves).

   Query over time
       Spanish: **consulta a lo largo del tiempo**
       A query evaluated across timesteps to produce a time series curve.

   Scalable Rendering
       Spanish: **renderizado escalable**
       Rendering done in parallel by the engine, shipping final pixels to the viewer.

   Session file
       Spanish: **archivo de sesión**
       A saved VisIt_ state (databases, plots, operators, windows, and settings) for later restoration.

   Species
       Spanish: **especies**
       Components of a material; species are treated as uniformly distributed within a material.

   Standard rendering mode
       Spanish: **modo de renderizado estándar**
       Rendering where the engine ships geometry to the viewer for rendering on the client side.

   Subset
       Spanish: **subconjunto**
       A selectable portion/category of a dataset (e.g. materials, domains, groups) used for subsetting.

   SIL restriction
       Spanish: **restricción del SIL**
       The current selection of subsets/categories (from the SIL) used to include/exclude data.

   Tensor
       Spanish: **tensor**
       A multi-component field often treated as a matrix (e.g. 2×2 in 2D, 3×3 in 3D).

   Time slider
       Spanish: **control deslizante de tiempo**
       The GUI control used to choose the active time state/timestep.

   Time state
       Spanish: **estado temporal**
       The current timestep/index in a time-varying database.

   Variable
       Spanish: **variable**
       A named dataset (mesh, scalar, vector, tensor, material, etc.) available in a database.

   Vector
       Spanish: **vector**
       A multi-component field with direction and magnitude (e.g. velocity).

   Viewer
       Spanish: **visor**
       The VisIt_ component that displays rendered results and coordinates state among clients.

   Visualization window
       Spanish: **ventana de visualización**
       One of the viewer's render windows; each window can show an independent visualization.

   Virtual database
       Spanish: **base de datos virtual**
       A logical database composed of multiple files (commonly a time series).

   VCL
   VisIt Component Launcher
       Spanish: **lanzador de componentes de VisIt**
       A helper process (often remote) responsible for launching mdserver and compute engines.

   VTK objects
       Spanish: **objetos VTK**
       Data objects used internally when translating database data into renderable forms.

   XML tools
       Spanish: **herramientas XML**
       Tools used to generate/update plugin and attributes code (e.g. ``xml2plugin``, ``xml2cmake``, ``xml2info``).

   Zone/cell interpolation
       Spanish: **interpolación por zona/celda**
       How field values are defined over a mesh (e.g. zone-centered vs node-centered).

   Mesh plot
       Spanish: **gráfico de malla**
       A plot that displays mesh lines/edges.

   Pseudocolor plot
       Spanish: **gráfico de pseudocolor**
       A plot that maps scalar values to colors.

   Contour plot
       Spanish: **gráfico de contornos**
       A plot that renders isocontours/isosurfaces of a scalar field.

   Vector plot
       Spanish: **gráfico de vectores**
       A plot that displays vector glyphs (arrows) showing direction and magnitude.

   Volume plot
       Spanish: **gráfico de volumen**
       A plot that renders volumetric data using transfer functions (opacity/color).

   Clip operator
       Spanish: **operador de recorte**
       An operator that removes data outside/inside a specified region.

   Slice operator
       Spanish: **operador de corte**
       An operator that extracts a planar cross-section of the data.

   Threshold operator
       Spanish: **operador de umbral**
       An operator that selects data based on value ranges.

   Isosurface operator
       Spanish: **operador de isosuperficie**
       An operator that extracts surfaces of constant scalar value (3D).

   Isovolume operator
       Spanish: **operador de isovolumen**
       An operator that extracts volumes within a scalar value range.
