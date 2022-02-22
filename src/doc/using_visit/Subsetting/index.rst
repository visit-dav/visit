.. _Subsetting:

Subsetting
==========

Meshes are frequently composed of a variety of subsets that represent
different portions of the mesh. Common examples are domains, groups
(of domains), AMR patches and levels, part assemblies, boundary conditions,
node sets and zone sets, materials and even material species.

Users often find it useful to *restrict* which subsets are used in any given
operation to focus their analyses on only certain regions of interest. This is
handled through VisIt_'s **Subset Window**. Here, we describe VisIt's
subsetting functionality and **Subset Window** in detail.

What is described here is primarily about *pre-defined, first-class, named
subsets* as created by the data producer and supported within VisIt.
Nonetheless, It is important to keep in mind that there are *other* ways that
the data producer can organize data within VisIt's GUI or that users can employ
VisIt's :ref:`Expressions <Expressions>` and :ref:`Operators <Operators>` to
create and manage subsets. However, using these other approaches for the sole
purpose of subsetting is often cumbersome through VisIt's GUI. To understand
why as well as read about other issues related to subsetting, please see
`these developer notes <https://www.visitusers.org/index.php?title=Generalized_Subsetting>`_.

.. toctree::
    :maxdepth: 2

    What_is_a_subset
    Subset_Inclusion_Lattice
    Using_the_Subset_Window
    Material_Interface_Reconstruction
    Species
