/****************************************************************************
* Copyright (c) 2022, CEA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include <Operator.h>
#include <Octree_Double.h>

#define verb_level 4

// Description: Find duplicate coordinates in the "coord" array.
//  nodes_renumber will have dimension src_coord.dimension(0)
//  nodes_renumber[i] = i if the node imust be conserved,
//  nodes_renumber[i] = j if the node i is identical to node j. We always have j<i
//  eps = tolerance in each direction to consider that two nodes are identical
//  nb_nodes_untouched : do not search duplicate nodes in the "nb_nodes_untouched"
//   first nodes. The remaining nodes are still compared to all nodes.
void Reconnect::search_duplicate_nodes(const BigFloatTab& src_coord, BigArrOfTID& nodes_renumber, double eps, trustIdType nb_nodes_untouched)
{
  // Create a temporary DoubleTab (coords are normally float)
  const trustIdType nb_nodes = src_coord.dimension(0);
  const int dim = (int)src_coord.dimension(1);
  // Build an octree with all coordinates
  Journal(verb_level + 1) << " Building octree" << endl;
  BigDoubleTab coords;
  coords.resize(nb_nodes, dim);
  for (trustIdType i = 0; i < nb_nodes; i++)
    for (int j = 0; j < dim; j++)
      coords(i, j) = src_coord(i, j);
  Octree_Double_64 octree;
  octree.build_nodes(coords, 0 /* no virtual nodes */);

  Journal(verb_level + 1) << " Searching duplicate nodes" << endl;
  nodes_renumber.resize_array(nb_nodes);
  for (trustIdType i = 0; i < nb_nodes; i++)
    nodes_renumber[i] = i;
  // For each node, are there several nodes within epsilon ?
  BigArrOfTID node_list;
  trustIdType count = 0; // Number of nodes renumbered
  for (trustIdType i = 0; i < nb_nodes; i++)
    {
      if (nodes_renumber[i] != i)
        continue; // node already suppressed

      const double x = coords(i, 0);
      const double y = (dim > 1) ? coords(i, 1) : 0.;
      const double z = (dim > 2) ? coords(i, 2) : 0.;
      octree.search_elements_box(x - eps, y - eps, z - eps, x + eps, y + eps, z + eps, node_list);
      Octree_Double_64::search_nodes_close_to(x, y, z, coords, node_list, eps);
      const trustIdType n = node_list.size_array();
      if (n > 1)
        {
          for (trustIdType j = 0; j < n; j++)
            {
              // Change only nodes with rank > i
              const trustIdType node = node_list[j];
              if (node > j)
                {
                  nodes_renumber[node] = i;
                  count++;
                }
            }
        }
    }
  Journal(verb_level + 1) << " " << count << " duplicate nodes will be removed" << endl;
}

void Reconnect::apply_renumbering(const BigArrOfTID& nodes_renumber, BigArrOfTID& data)
{
  trustIdType ntot = data.size_array();
  for (trustIdType i = 0; i < ntot; i++)
    {
      const trustIdType node = data[i];
      const trustIdType n = nodes_renumber[node];
      if (n != node)
        data[i] = n;
    }
}

// Description: updates the elements_ and faces_ arrays of the domain so that
//  all nodes having the same coordinates are replaced by one unique node
//  in these arrays. See search_duplicate_nodes for nb_nodes_untouched description.
void Reconnect::reconnect_geometry(DomainUnstructured& geom, double tolerance, trustIdType nb_nodes_untouched)
{
  Journal(verb_level) << "Reconnect domain " << geom.id_.name_ << endl;

  BigArrOfTID nodes_renumber;
  search_duplicate_nodes(geom.nodes_, nodes_renumber, tolerance, nb_nodes_untouched);

  apply_renumbering(nodes_renumber, geom.elements_);

  if (geom.faces_ok())
    apply_renumbering(nodes_renumber, geom.faces_);
}

#undef verb_level
