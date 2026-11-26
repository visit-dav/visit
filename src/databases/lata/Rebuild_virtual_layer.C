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

#include <LataFilter.h>
#include <Connectivite_som_elem.h>
#include <Operator.h>
#include <Static_Int_Lists.h>
#include <Rebuild_virtual_layer.h>

void find_virtual_layer(DomainUnstructured& domain, BigTIDTab& virtual_elements, BigTIDTab& joints_virtual_elements, double tolerance)
{
  Journal(4) << "Searching virtual elements for domain " << domain.id_.name_ << endl;
  // Step 1 : find duplicate nodes
  BigArrOfTID nodes_renumber;
  Reconnect::search_duplicate_nodes(domain.nodes_, nodes_renumber, tolerance, 0);

  // Build reconnected elements
  Reconnect::apply_renumbering(nodes_renumber, domain.elements_);

  Static_Int_Lists_64 som_elem;
  construire_connectivite_som_elem<trustIdType>(domain.nb_nodes(), domain.elements_, som_elem, 0 /* include virtual */);

  virtual_elements.resize(0, 1);

  // Step 2 : for each sub_zone, add to virtual_elements list all elements
  // touching the zone and not included in the zone
  const BigTIDTab& joints_sommets = domain.get_joints(LataField_base::SOM);
  const BigTIDTab& joints_elements = domain.get_joints(LataField_base::ELEM);
  const int nprocs = (int)joints_sommets.dimension(0);
  joints_virtual_elements.resize(nprocs, 2);
  BigArrOfTID tmp;
  for (int i_proc = 0; i_proc < nprocs; i_proc++)
    {
      trustIdType first_elem_zone = joints_elements(i_proc, 0);
      trustIdType end_elems_zone = first_elem_zone + joints_elements(i_proc, 1);
      trustIdType first_node_zone = joints_sommets(i_proc, 0);
      trustIdType end_nodes_zone = first_node_zone + joints_sommets(i_proc, 1);
      const trustIdType first_virtual_element = virtual_elements.dimension(0);
      tmp.resize_array(0);
      for (trustIdType i_node = first_node_zone; i_node < end_nodes_zone; i_node++)
        {
          const trustIdType renum_node = nodes_renumber[i_node];
          const trustIdType nb_elems_voisins = som_elem.get_list_size(renum_node);
          for (trustIdType i = 0; i < nb_elems_voisins; i++)
            {
              const trustIdType elem = som_elem(renum_node, i);
              if (elem < first_elem_zone || elem >= end_elems_zone)
                tmp.append_array(elem);
            }
        }
      // Retirer les doublons
      tmp.ordonne_array();
      const trustIdType n = tmp.size_array();
      trustIdType last = -1;
      for (trustIdType i = 0; i < n; i++)
        {
          const trustIdType elem = tmp[i];
          if (elem != last)
            {
              const trustIdType idx = virtual_elements.dimension(0);
              virtual_elements.resize(idx + 1, 1);
              virtual_elements(idx, 0) = elem;
              last = elem;
            }
        }
      joints_virtual_elements(i_proc, 0) = first_virtual_element;
      joints_virtual_elements(i_proc, 1) = virtual_elements.dimension(0) - first_virtual_element;
      Journal(5) << "Zone " << i_proc << " has " << joints_virtual_elements(i_proc, 1) << " virtual elements" << endl;
    }
}

int rebuild_virtual_layer(LataDB& lataDB, Domain_Id id, double reconnect_tolerance)
{
  Journal(4) << "rebuilt_virtual_layer domain " << id.name_ << " " << id.timestep_ << endl;
  if (lataDB.field_exists(id.timestep_, id.name_, "VIRTUAL_ELEMENTS"))
    {
      Journal(4) << " Virtual elements data already exist. Skip" << endl;
      return 1;
    }
  if (!lataDB.field_exists(id.timestep_, id.name_, "JOINTS_ELEMENTS"))
    {
      Journal(4) << " Domain has no processor splitting information. Skip" << endl;
      return 0;
    }
  // Load all domain, without faces:
  id.block_ = -1;
  DomainUnstructured dom;
  dom.fill_domain_from_lataDB(lataDB, id, 0 /* no faces */);
  // Compute virtual zones:
  BigTIDTab joints_virtual_elements;
  BigTIDTab virtual_elements;
  find_virtual_layer(dom, virtual_elements, joints_virtual_elements, reconnect_tolerance);
  // Write data to disk
  const LataDBField& joints = lataDB.get_field(id.timestep_, id.name_, "JOINTS_ELEMENTS", "*");
  LataDBField fld(joints);
  // Append virtual_elements data to JOINTS_ELEMENTS, same format, etc
  fld.name_ = "JOINTS_VIRTUAL_ELEMENTS";
  fld.uname_ = Field_UName(fld.geometry_, fld.name_, "");
  fld.nb_comp_ = 2;
  fld.datatype_.file_offset_ = 0;
  fld.filename_ += ".ghostdata";
  lataDB.add_field(fld);
  lataDB.write_data(id.timestep_, fld.uname_, joints_virtual_elements);
  fld.name_ = "VIRTUAL_ELEMENTS";
  fld.uname_ = Field_UName(fld.geometry_, fld.name_, "");
  fld.nb_comp_ = 1;
  fld.datatype_.file_offset_ = 1; // append
  fld.size_ = virtual_elements.dimension(0);
  lataDB.add_field(fld);
  lataDB.write_data(id.timestep_, fld.uname_, virtual_elements);
  return 1;
}
