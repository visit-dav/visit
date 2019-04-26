.. _What is a subset:

What is a subset?
-----------------

VisIt has first-class support for four different kinds of subsets; *Domains*,
*Groups* (also called *Blocks*), *Materials* and material *Species*.
In particular, as currently designed, any given mesh in VisIt can have only
**one** decomposition into each of these kinds of subsets. That is, a mesh can
have only one *Domain* decomposition, one *Group* decomposition, one
*Material* decomposition and one material *Species* decomposition.
A fifth kind of subset, *Enumerated*, is also supported and provides some
additional generality but cannot be used in combination with the other four
or even with other *Enumerated* subsets.

Data producers as well as the database plugins that read data into VisIt
often have flexibility in deciding how to utilize these various kinds
of subsets in representing their data. We describe each of these kinds of
subsets and constraints in their use below.

Domain Subsets
~~~~~~~~~~~~~~
VisIt's concept of a *Domain* subset is fundamental to its
parallel programming and execution model. A domain in VisIt represents a
*chunk* of mesh plus its variables that is **both** stored (in files and in
memory) **and** processed *coherently* as a single, self-contained unit.
Large meshes in VisIt are typically decomposed into *Domain* subsets for
parallel processing. In fact, except in rare cases, the maximum number of
MPI tasks VisIt may use is determined by the number of *Domain* subsets
created by the data producer. VisIt's approach to processing a mesh in
parallel is often described as *piggy-backing* off of the parallel decomposition
created by the data producer.

*Domain* subsets also represent the *unit of work* VisIt allocates in its load
balancing algorithms. If VisIt is running on *M* processors and reading a
mesh of *N* domains, then if N<M, N-M processors will idle for operations
involving that mesh. On the other hand, if N>kM (k an integer), some processors
will be assigned *k* domains and some *k+1* domains.

In almost all cases, if a mesh is to be processed in parallel by VisIt, it must
have been decomposed into *Domain* subsets by the data producer prior to reading
the data into VisIt. In general, VisIt does not perform any on-the-fly domain
decomposition of data it is reading. However, there is one, special case 
where VisIt can perform on-the-fly domain decomposition of a large, monolithic
mesh; a structured mesh stored in a file format that supports hyper-slabbed I/O.
In this simple case, VisIt will try to evenly decompose the 2 or 3D mesh into
roughly equal sized hyper-slabs whose number is determined by the number of
parallel tasks. VisIt will also then utilize the file format's hyper-slab I/O
routines to read into each parallel task only the part(s) of the mesh assigned
to that task.

A mesh is **required** to have domains if it is ever to be processed in parallel
by VisIt.

Group or Block Subsets
~~~~~~~~~~~~~~~~~~~~~~
*Groups* (or *Blocks*) are just unions of *Domains*. They are optional. A mesh
is not **required** to have groups. On the other hand, if a mesh has *Groups*,
then **every** domain in the mesh must be assigned to one and only one *Group*
subset. *Groups* may be used to represent, for example, the files in which
multiple domains are stored or sets of neighboring domains that share a common
logical/structured indexing arrangement in an otherwise globally unstructured
mesh.

The key constraint about group subsets is that they can represent only unions
of the *domain* subsets. Internally in VisIt, a group subset is implemented as
a list of domain subset ids.

Material Subsets
~~~~~~~~~~~~~~~~
Material subsets are used to represent the decomposition of a mesh into various
materials. For example, a mesh may be composed of steel, brass, and aluminum
materials. If these materials are given integer ids 83 (``int('S')``), 66
(``int('B')``) and 65 (``int('A')``), then each zone (or cell) in the mesh can
be assigned a value of 83, 66 or 65 to indicate the zone is composed of steel,
brass or aluminum. This would be equivalent to an integer valued (with 3 unique
values), zone-centered variable on the mesh.

For material subsets, however, VisIt also supports a notion of
*mixing* where a single zone (or cell) can be composed of multiple materials
each occupying some fractional volume of a whole zone (or cell). From a
sub-setting perspective, a more formal way of thinking about *mixing*
is that it is way of supporting *partial inclusion* of a mesh zone (or cell)
within a given material subset.

Material subsets are optional. Furthermore, if material subsets are defined
additionally supporting *mixing* is also optional. Only some data producers that
involve *Material* subsets also involve *mixing*.

When *mixing materials* are involved, VisIt can employ a variety of
sophisticated
:ref:`Material Interface Reconstruction (MIR)<Material Interface Reconstruction>`
algorithms to draw the interfaces between materials based on the volume
fractions of the *mixing*. The main point about *MIR* is that it represents
an additional computational burden when manipulating *Material* subsets.
Manipulating *Group* or *Domain* subsets has no such equivalent computational
cost.

Mesh Variables with Material Specific Properties
""""""""""""""""""""""""""""""""""""""""""""""""
For some mesh variables, data producers may have different values of the
variable for each of the materials within various zones (or cells) of the mesh
where *mixing* is occurring. When such a variable is being plotted, for example
with the :ref:`Pseudocolor Plot <pseudocolor_plot_head>`, what value/color
should VisIt show for such zones? The fact is, depending on the user's needs,
VisIt is capable of showing either an *overall* value for the zone or showing
the material-specific values in the zone. This can be handled through
appropriate use of VisIt's :ref:`(MIR)<Material Interface Reconstruction>`
algorithms and **Subset Window** controls.

Species Subsets
~~~~~~~~~~~~~~~
In addition to *mixing*, another feature *Materials* subsets support is a
notion of *Species*. For example, there are many different varieties of
brass and steel depending on the alloys used. Neither brass nor steel are
themselves pure elements on the periodic table. They are instead *alloys* of
other pure metals. Common Yellow Brass is, nominally, a mixture
of Copper (Cu) and Zinc (Zn) while Tool Steel is composed primarily of Iron (Fe)
but mixed with some Carbon (C) and a variety of other elements.

Lets suppose we are dealing with the following alloys and species
compositions...

+-----------+-----------------------------------------------------+
| Material  | Species composition                                 |
+===========+=====================================================+
| Brass     | Cu:65%, Zn:35%                                      |
+-----------+-----------------------------------------------------+
| T-1 Steel | Fe:76.3%, W:18%,  Cr:4.0%, C:0.7%, V:1%             |
+-----------+-----------------------------------------------------+
| O-1 Steel | Fe:96.2%, W:0.5%, Cr:0.5%, C:0.9%, Mn:1.4%, Ni:0.5% |
+-----------+-----------------------------------------------------+

The *Materials* decomposition would consist of 3 subsets for Brass, T-1 Steel
and O-1 Steel. For the *Species* decomposition, Brass would be further
decomposed into 2 *Species* subsets, T-1 Steel into 5 *Species* subsets and
O-1 Steel, 6 *Species* subsets.

Alternatively, one could opt to characterize both T-1 Steel
and O-1 Steel has a single, non-specific *Steel* having
7 *Species* subsets, Fe, W, Cr, C, V, Mn, Ni where for T-1 Steel, the Mn and Ni
*Species* subsets are always empty and for O-1 Steel the V *Species* subset
is always empty. In that case, there would only be 2 *Materials* subsets
for Brass and non-specific *Steel*.

*Species* subsets are optional. A mesh does not need to have them defined.
However, as currently designed, a data producer cannot define *Species*
subsets without also defining *Materials* subsets (even if there is only one
material subset for the whole mesh).

A final thing to note about *Species* subsets is that they do not represent
spatially distinct parts of the mesh like *Domains*, *Groups*, or *Materials*.
Instead, *Species*, if they are defined are ever present, everywhere in the
mesh. Only their relative concentrations vary at any given point in the mesh.
But, *Species* do permit subsetting a particular physical quantity's *value*
in that, for example the *total pressure* in a zone can be decomposed into
partial pressures on each of the species comprising the materials in the zone.
Furthermore, using the **Subset Window**, VisIt can then control which partial
value(s) are used in a particular plot.

Domains, Groups, Materials and Species In Combination
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
A given mesh may involve any combination of *Domain*, *Group* and *Material*
subsets. Furthermore, VisIt's **Subset Window**  makes it possible to manipulate
these four kinds of subset *in combination*. That is, a user can simultaneously
control which domains, which materials and which groups VisIt should process in
any given operation. However, manipulating subsets in combination works only
for these kinds of subsets. Other kinds of sub-setting, such as Enumerated
subsets which are discussed next, are not as well integrated.

Enumerated Subsets
~~~~~~~~~~~~~~~~~~
A key constraint of the other kinds of subsets is that any given mesh can have
only **one** decomposition into domains and **one** decomposition into groups
and **one** decomposition into materials. However, a mesh can be composed of
any number of *Enumerated* subsets. Enumerated subsets are defined by first
defining the enumeration *class* and then creating a *bitmap* like variable
over the mesh to indicate which mesh entities (nodes, edges, faces or volumes)
belong to which subsets of the enumeration class.

Within an enumeration class, the sets can be arranged hierarchically so that
some sets contain other sets as in a part assembly.

Enumerated subsets do not work in
combination with domains, groups or materials or in combination with other
classes of *Enumerated* subsets. On the other hand, for any given mesh, there
can be any number of enumeration classes, each defining a collection of related
subsets. For example, if a mesh has defined two enumeration classes, one for
*node sets* and one for *face sets*, then different subsets of nodes can be
manipulated simultaneously or different subsets of faces can be manipulated
simultaneously but different sets of nodes cannot simultaneously be manipulated
in combination with different sets of faces. Finally, manipulating enumerated
subsets can also incur small a computational burden due to the work involved in
finding the mesh entities within a given subset.
