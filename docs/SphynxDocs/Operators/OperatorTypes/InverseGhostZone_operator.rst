InverseGhostZone operator
~~~~~~~~~~~~~~~~~~~~~~~~~

The InverseGhostZone operator makes ghost cells visible and removes real cells from the dataset so plots to which the InverseGhostZone operator have been applied show only the mesh's ghost cells. Ghost cells are a layer of cells around the mesh that usually correspond to real cells in an adjacent mesh when the whole mesh has been decomposed into smaller domains. Ghost cells are frequently used to ensure continuity between domains for operations like contouring. The InverseGhostZone operator is useful for debugging ghost cell placement in simulation data and for database reader plugins under development.

Making all cells visible
""""""""""""""""""""""""

The InverseGhostZone operator's only purpose is to make ghost cells visible and real cells are usually stripped out. If you want to ensure that real cells are not removed while still making the ghost cells be enabled, click the
**Both ghost zones and real zones **
radio button in the
**InverseGhostZone operator attributes window**
(see
).

