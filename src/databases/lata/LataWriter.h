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

#ifndef LataWriter_H
#define LataWriter_H
#include <LataDB.h>

class Domain;
class LataField_base;

// This class provides general services to write lata files
//  from the "high level" objects Domain and Field (the LataDB class provides
//  only low level services to write arrays)
class LataWriter
{
public:
  enum FileSplittingOption { MULTIPLE_LATA_FILES, SINGLE_LATA_FILE };
  enum ERRORS { InternalError };

  void init_file(const Nom& path, const Nom& basename, const LataDBDataType& default_int_format, LataDBDataType::Type default_float_type);
  void write_faces_fields() { write_faces_fields_ = true; }
  void write_component(const LataField_base& field);
  void write_geometry(const Domain& dom);
  void write_time(double t);
  void finish();

  void set_file_splitting_option(const FileSplittingOption op) { lata_option_ = op; }

protected:
  FileSplittingOption lata_option_ = MULTIPLE_LATA_FILES;
  LataDB db_; // This is the database where we put all data...
  Nom base_name_; // Basename for files and lata master file:
  Size_t offset_ = 0;
  bool write_faces_fields_ = false;
};

#endif /* LataWriter_H */
