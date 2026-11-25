// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef HOST_RESOLVER_H
#define HOST_RESOLVER_H

#include <utility_exports.h>

#include <string>

struct addrinfo;

// ****************************************************************************
// Class: HostResolver
//
// Purpose:
//   Provides RAII ownership for addresses returned by getaddrinfo.
//
// ****************************************************************************

class UTILITY_API HostResolver
{
public:
    HostResolver();
    explicit HostResolver(const std::string &host, int addressFamily = 0,
                          int socketType = 0, int flags = 0);
    ~HostResolver();

    HostResolver(const HostResolver &) = delete;
    HostResolver &operator=(const HostResolver &) = delete;
    HostResolver(HostResolver &&) noexcept;
    HostResolver &operator=(HostResolver &&) noexcept;

    bool Resolve(const std::string &host, int addressFamily = 0,
                 int socketType = 0, int flags = 0);
    void Reset();

    bool IsValid() const;
    int GetErrorCode() const;

    // The returned pointers remain owned by this object and are valid until
    // the next call to Resolve or Reset, or until this object is destroyed.
    const struct addrinfo *GetAddresses() const;
    const char *GetCanonicalName() const;

private:
    struct addrinfo *addresses;
    int errorCode;
};

#endif
