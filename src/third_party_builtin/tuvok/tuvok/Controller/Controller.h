/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.


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

//!    Author : Tom Fogal
//!             SCI Institute
//!             University of Utah
//!    Date   : March 2009
//!
//!    Copyright (C) 2009 SCI Institute

#pragma once

#ifndef TUVOK_CONTROLLER_H
#define TUVOK_CONTROLLER_H

#include "MasterController.h"
#include "boost/serialization/singleton.hpp"

namespace serial = boost::serialization;

namespace Controller {
  inline MasterController& Instance() {
    return serial::singleton<MasterController>::get_mutable_instance();
  }
  inline const MasterController& ConstInstance() {
    return serial::singleton<MasterController>::get_const_instance();
  }

  namespace Debug {
    inline AbstrDebugOut& Out() {
      return *(Controller::Instance().DebugOut());
    }
    inline const AbstrDebugOut& ConstOut() {
      return *(Controller::ConstInstance().DebugOut());
    }
  };
};

#define T_ERROR(...)                                       \
  do {                                                     \
    Controller::Debug::Out().Error(_func_, __VA_ARGS__);   \
  } while(0)
#define WARNING(...)                                       \
  do {                                                     \
    Controller::Debug::Out().Warning(_func_, __VA_ARGS__); \
  } while(0)
#define MESSAGE(...)                                       \
  do {                                                     \
    Controller::Debug::Out().Message(_func_, __VA_ARGS__); \
  } while(0)
#define OTHER(...)                                         \
  do {                                                     \
    Controller::Debug::Out().Other(_func_, __VA_ARGS__);   \
  } while(0)

#endif // TUVOK_CONTROLLER_H
