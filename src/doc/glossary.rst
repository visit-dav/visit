.. _Glossary:

Glossary
========

.. glossary::

   AAN
   Always, Auto, Never
       Various features in VisIt_ support an **Always, Auto, Never** choice.
       A setting of **Never** means to never enable the the feature and a
       setting of **Always** means to always enable the feature. A setting
       of **Auto**, which is typically the default, means the allow VisIt_ 
       to decide when it thinks it is best to enable or disable the feature.
    
   Integral Curve
       An integral curve is a curve that begins at a seed location and is
       tangent at every point in a vector field. It is computed by numerical
       integration of the seed location through the vector field.

   Node
   Point
   Vertex
       These terms refer to the *corners* or *ends* of mesh elements.

   Pathlines
       A path rendered by an integrator that uses the vector field that is
       *in-step* with the integrator, so that as the integrator steps through
       time, it uses data from the vector field at each new time step.

   Node-centered
   Point-centered
       These terms refer to a piecewise-linear (one degree of freedom at each
       of mesh element *corner*) interpolation scheme used to define a variable
       on a mesh. VTK tends to use the *point* terminology whereas VisIt_ tends
       to use the *node* terminology.

   Parallel task
       Although developers are working to enhance VisIt_ to support a variety
       of fine-grained parallelism methods (e.g. MC or GPU) and although some
       portions of VisIt_ have supported multi-threaded processing for several
       years, in the currently available implementations, a parallel task is an
       :abbr:`MPI (Message Passing Interface)` rank.

   Streamlines
       A path rendered by an integrator that uses the *same* vector field for
       the entire integration.

   SIL
   Subset Inclusion Lattice
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
       SR is an abbreviation for **Scalable Rendering**. This is a mode of
       operation where the VisIt_ **engine** performs scalable, parallel
       rendering and ships the final rendered image (e.g. pixels) to the
       **viewer**. This is in contrast to *standard* mode where the
       **engine** ships polygons to the **viewer** to be rendered there.

   Zone
   Cell
       These terms refer to the the individual computational elements comprising
       a mesh.

   Zone-centered
   Cell-centered
       These terms refer to a piecewise-constant (single degree of freedom for
       an entire zone) interpolation scheme used to define a field variable on
       a mesh. VTK tends to use the *cell* terminology whereas VisIt_ tends to
       use the *zone* terminology.

