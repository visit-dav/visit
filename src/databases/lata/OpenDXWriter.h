/*****************************************************************************
*
* Copyright (c) 2011, CEA
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of CEA, nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#ifndef OpenDXWriter_H_
#define OpenDXWriter_H_
#include <ArrOfInt.h>
#include <Lata_tools.h>

class Domain;
class LataField_base;
// Usage:
//   init_cout(...) or init_file(...)
//   for (i=0; i < nb_geometries; i++) {
//     write_geometry(...);
//     for (j=0; j < nb_fields; j++)
//       write_component(...);
//   }
//   finish();
class DX_stream;
class OpenDXWriter
{
public:
  OpenDXWriter();
  ~OpenDXWriter();
  void init_cout(double time, int ascii = 0);
  void init_file(double time, Nom & filename_, int ascii = 0);

  void write_geometry(const Domain & dom);
  void write_component(const LataField_base & field);

  entier finish(int force_group = 0);
  enum DXErrors { DXInternalError };
protected:
  void reset();
  void finish_geometry();

  int dx_time_index_;
  int index_counter_;
  // Indexes of all DXfield objects in the file (to build the final group)
  ArrOfInt fields_indexes_;
  // Names of the DXfields:
  Noms fields_names_;

  // Index of the nodes array of the last geometry
  int nodes_index_;
  // Index of the elements array of the last geometry
  int elements_index_;
  // Index of these arrays:
  int invalid_positions_;
  int invalid_connections_;
  // Indexes of the components associated with the last geometry
  ArrOfInt components_indexes_;
  Noms components_names_;

  DX_stream * os_;
};
#endif
