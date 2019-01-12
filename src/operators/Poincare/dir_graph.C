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
 *  dir_graph.C:
 *
 *  Written by:
 *   Guoning Chen
 *   SCI Institute
 *   University of Utah
 *   April 2012
 *
 */

/*
    Define a directed graph structure
*/
#include "dir_graph.h"

DIR_Graph::DIR_Graph(int node_size, int edge_size)
{
  nlist.resize(node_size);
  elist.resize(edge_size);
}

DIR_Graph::~DIR_Graph()
{
  nlist.clear();
  elist.clear();
}

void DIR_Graph::add_one_node(GraphNode &node)
{
  node.node_id = (int)nlist.size();
  node.edges.clear();
  nlist.push_back(node);
}
  
void DIR_Graph::add_one_edge(GraphEdge &e)
{
  e.edge_id=(int)elist.size();
  elist.push_back(e);
                
  // remember to udpate the edge lists of the two nodes connected by e
  nlist[e.node1].edges.push_back(e.edge_id);
  nlist[e.node2].edges.push_back(e.edge_id);
}

void DIR_Graph::init_graph()
{
  nlist.clear();
  elist.clear();
}


void DIR_Graph::convert_SKL_to_graph (Skeleton::Skeleton s)
{
  init_graph();

  int num_edges = (int)s.size();
  nlist.resize(num_edges + 1);
  elist.resize(num_edges);

  // Add nodes
  for (std::list<Skeleton::SkeletonLine>::iterator SL = s.begin();
       SL != s.end();
       ++SL)
  {
    int cur_nid = (*SL).lower.vertex->ID;
    nlist[cur_nid].node_id = nlist[cur_nid].pre_node_id = cur_nid;
    nlist[cur_nid].x = (*SL).lower.vertex->point.x;
    nlist[cur_nid].y = (*SL).lower.vertex->point.y;

    cur_nid = (*SL).higher.vertex->ID;
    nlist[cur_nid].node_id = nlist[cur_nid].pre_node_id = cur_nid;
    nlist[cur_nid].x = (*SL).higher.vertex->point.x;
    nlist[cur_nid].y = (*SL).higher.vertex->point.y;
  }

  // Add edges
  for (std::list<Skeleton::SkeletonLine>::iterator SL = s.begin();
       SL != s.end();
       ++SL)
  {
    elist[(*SL).ID].edge_id = (*SL).ID;
    elist[(*SL).ID].node1 = (*SL).lower.vertex->ID;
    elist[(*SL).ID].node2 = (*SL).higher.vertex->ID;
    float dx = (*SL).higher.vertex->point.x - (*SL).lower.vertex->point.x;
    float dy = (*SL).higher.vertex->point.y - (*SL).lower.vertex->point.y;
    elist[(*SL).ID].weight = (dx*dx + dy*dy);

    nlist[(*SL).lower.vertex->ID].edges.push_back((*SL).ID);
    nlist[(*SL).higher.vertex->ID].edges.push_back((*SL).ID);
  }
}

void DIR_Graph::trim_leafs()
{
  std::vector<GraphEdge> tmp_elist;
  std::vector<GraphNode> tmp_nlist;

  size_t i;
  for (i=0; i<elist.size(); i++)
  {
    GraphEdge &e=elist[i];
    if (nlist[e.node1].edges.size()<2 || nlist[e.node2].edges.size()<2)
          continue;
    tmp_elist.push_back(e);
  }

  // remove the leaf nodes
  for (i=0; i<tmp_elist.size(); i++)
  {
    GraphEdge &e=tmp_elist[i];
    bool add=true;
    for (size_t j=0; j<tmp_nlist.size(); j++)
    {
      if (tmp_nlist[j].pre_node_id == e.node1)
      {
        add = false;
        break;
      }
    }
    
    if (add)
    {
      tmp_nlist.push_back(nlist[e.node1]);
      tmp_nlist[tmp_nlist.size()-1].node_id = (int)tmp_nlist.size()-1;
      tmp_nlist[tmp_nlist.size()-1].edges.clear();
    }
    
    add=true;
    for (size_t j=0; j<tmp_nlist.size(); j++)
    {
      if (tmp_nlist[j].pre_node_id == e.node2)
      {
        add = false;
        break;
      }
    }

    if (add)
    {
      tmp_nlist.push_back(nlist[e.node2]);
      tmp_nlist[tmp_nlist.size()-1].node_id = (int)tmp_nlist.size()-1;
      tmp_nlist[tmp_nlist.size()-1].edges.clear();
    }
  }

  // update the temporary edge list
  for (i=0; i<tmp_elist.size(); i++)
  {
    GraphEdge &e=tmp_elist[i];
    
    for (size_t j=0; j<tmp_nlist.size(); j++)
    {
      if (tmp_nlist[j].pre_node_id == e.node1)
      {
        e.node1 = tmp_nlist[j].node_id;
        tmp_nlist[j].edges.push_back((int)i);
        break;
      }
    }
    
    for (size_t j=0; j<tmp_nlist.size(); j++)
    {
      if (tmp_nlist[j].pre_node_id == e.node2)
      {
        e.node2 = tmp_nlist[j].node_id;
        tmp_nlist[j].edges.push_back((int)i);
        break;
      }
    }
  }

  // copy back to the original graph
  nlist.clear();
  elist.clear();
    
  for (i=0; i<tmp_nlist.size(); i++)
  {
    nlist.push_back(tmp_nlist[i]);
    nlist[i].pre_node_id = nlist[i].node_id;
  }

  for (i=0; i<tmp_elist.size(); i++)
    elist.push_back(tmp_elist[i]);
}

int DIR_Graph::Min_Distance(const std::vector<double>& D, std::vector<bool>& F)
{                
  size_t j = 0;
  double dmin = 1.e30;                                                   
            
  for(size_t i = 0; i < nlist.size(); i++)
  {                                                      
    if(F[i])
    {
      if(D[i] < dmin)
      {
        dmin = D[i];
        
        j = i;
      }
    }
  }
            
  F[j] = false;

  return (int)j;    
}  

void DIR_Graph::Dijkstra_path_between(int n1, int n2, std::vector<int> &path)
{
  size_t n = nlist.size();
  size_t count = 0;
  std::vector<int> P(n, 0);
    
  std::vector<bool> F(n, true);
    
  std::vector<double> D(n, 1.e30);     
  D[n1]=0.0;

  while(count < n)
  {
    int u = Min_Distance(D, F);
    
    if (u==n2)
      break;
    
    for (size_t i=0; i < nlist[u].edges.size(); i++)
    {
      GraphEdge &cur_e = elist[nlist[u].edges[i]];
      
      int v = cur_e.node1;
      
      if (v==u)
        v=cur_e.node2;
      
      double alt = D[u] + cur_e.weight;
      
      if (alt < D[v])
      {
        D[v] = alt;
        P[v] = u;
      }
    }

    count++;
  }
  
  //Vertex *u = v2;
  int u = n2;
  path.clear();

  while(P[u])
  {
    path.push_back(u);
    u = P[u];
  }

  path.push_back(n1);

  P.clear();
  F.clear();
  D.clear();
}

void DIR_Graph::search_shortest_paths_between_pairs_of_leafs()
{
  std::vector<int> leaf_nodes;
  size_t i, j;
  
  for (i=0; i<nlist.size(); i++)
  {
    if (nlist[i].edges.size()<2)
      leaf_nodes.push_back((int)i);
  }
  
  for (i=0; i<leaf_nodes.size()-1; i++)
  {
    for (j=i+1; j<leaf_nodes.size(); j++)
    {
      std::vector<int> path;
      Dijkstra_path_between(leaf_nodes[i], leaf_nodes[j], path);
      all_paths_between_leafs.push_back(path);
    }
  }
}

void DIR_Graph::get_the_longest_shortest_path()
{
  std::vector<float> path_lens;
  size_t i, j;

  for (i=0; i<all_paths_between_leafs.size(); i++)
  {
    float sum = 0;
    for (j=0; j<all_paths_between_leafs[i].size()-1; j++)
    {
      //sum += elist[all_paths_between_leafs[i][j]].weight;
      float dx = nlist[all_paths_between_leafs[i][j+1]].x -
        nlist[all_paths_between_leafs[i][j]].x;
      float dy = nlist[all_paths_between_leafs[i][j+1]].y -
        nlist[all_paths_between_leafs[i][j]].y;
      
      sum += sqrt(dx*dx + dy*dy);
    }
    
    path_lens.push_back(sum);
  }
  
  int path_id = 0;
  float cur_longest_len = path_lens[0];
  
  for (i=1; i<path_lens.size(); i++)
  {
    if (path_lens[i] > cur_longest_len)
    {
      cur_longest_len = path_lens[i];
      path_id = (int)i;
    }
  }

  for (i=0; i<all_paths_between_leafs[path_id].size(); i++)
  {
    longest_path.push_back(all_paths_between_leafs[path_id][i]);
  }
  
  longest_length = cur_longest_len;
}

Skeleton::Point DIR_Graph::get_centroid()
{
  trim_leafs();
  search_shortest_paths_between_pairs_of_leafs();
  get_the_longest_shortest_path();

  size_t i;
  float half_len = longest_length/2.;

  float cur_len = 0;
  for (i=0; i<longest_path.size()-1; i++)
  {
    float dx = nlist[longest_path[i+1]].x - nlist[longest_path[i]].x;
    float dy = nlist[longest_path[i+1]].y - nlist[longest_path[i]].y;
    
    float tmp_len = sqrt(dx*dx+dy*dy);
    cur_len += tmp_len;
    
    if (cur_len > half_len)
    {
      float extra_len = cur_len - half_len;
      float alpha = extra_len/tmp_len;
      float cx = alpha*nlist[longest_path[i]].x +
        (1-alpha)*nlist[longest_path[i+1]].x;
      float cy = alpha*nlist[longest_path[i]].y +
        (1-alpha)*nlist[longest_path[i+1]].y;
      
      return Skeleton::Point(cx, cy);
    }
  }

  return Skeleton::Point(INFINITY, INFINITY); ///TODO: check on fix for no return-type?
}
