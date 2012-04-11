/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


/*
 *  dir_graph.h:
 *
 *  Written by:
 *   Guoning Chen
 *   SCI Institute
 *   University of Utah
 *   April 2012
 *
 */

#if !defined(DIR_GRAPH_H)
#define DIR_GRAPH_H

#include "skelet.h"

/*
    Define a directed graph structure
*/
class DIR_Graph
{
public:
  // a node in the directed graph (corresponding to the skeleton vertex)
  typedef struct GraphNode
  {
    int node_id;
    int pre_node_id;
    float x, y;
    std::vector<int> edges;
  } GraphNode;
  
  // an edge in the directed graph (corresponding to the skeleton line)
  typedef struct GraphEdge
  {
    int node1, node2;    // edge direction: node1-->node2
    int edge_id;
    float weight;
  } GraphEdge;
  
public:
  DIR_Graph(int node_size, int edge_size);

  ~DIR_Graph();

  void add_one_node(GraphNode &node);
  
  void add_one_edge(GraphEdge &e);

  void init_graph();

  void convert_SKL_to_graph (Skeleton::Skeleton s);

  void trim_leafs();

  int Min_Distance(const std::vector<double>& D, std::vector<bool>& F);

  void Dijkstra_path_between(int n1, int n2, std::vector<int> &path);

  void search_shortest_paths_between_pairs_of_leafs();

  void get_the_longest_shortest_path();

  Skeleton::Point get_centroid();

protected:
  std::vector<GraphNode> nlist;
  std::vector<GraphEdge> elist;

  std::vector< std::vector< int > > all_paths_between_leafs;
  std::vector<int> longest_path;
  Skeleton::Point Centroid;
  float longest_length; 
};
#endif
