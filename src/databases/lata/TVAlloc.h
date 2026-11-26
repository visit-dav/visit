/****************************************************************************
* Copyright (c) 2024, CEA
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

#ifndef TVAlloc_included
#define TVAlloc_included

/*! @brief Allocator adaptor that intercepts the 'construct' calls to convert value initialization into default initialization.
 * Written by Casey Carter (@codercasey)
 * Taken from https://hackingcpp.com/cpp/recipe/uninitialized_numeric_array.html
 *
 * This allows "std::vector<T>(30)" to only allocate, not initialize, the data.
 */
template< typename T, typename Alloc = std::allocator<T> >
class TVAlloc : public Alloc
{
  using a_t = std::allocator_traits<Alloc>;
public:
  // Obtain alloc<U> where U != T
  template<typename U>
  struct rebind
  {
    using other = TVAlloc<U, typename a_t::template rebind_alloc<U> >;
  };

  // Make inherited ctors visible
  using Alloc::Alloc;

  // Ddefault-construct objects - WITHOUT initialisation!
  template<typename U>
  void construct (U* ptr)     noexcept(    std::is_nothrow_default_constructible<      U>::value)
  {
    ::new(static_cast<void*>(ptr)) U; // 'placement new':
  }

  // Construct with ctor arguments
  template<typename U, typename... Args>
  void construct (U* ptr, Args&& ... args)
  {
    a_t::construct(
      static_cast<Alloc&>(*this),
      ptr, std::forward<Args>(args)...);
  }

};

#endif /* TVAlloc_included */
