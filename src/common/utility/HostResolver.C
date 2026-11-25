// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <HostResolver.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

HostResolver::HostResolver() : addresses(nullptr), errorCode(0)
{
}

HostResolver::HostResolver(const std::string &host, int addressFamily,
                           int socketType, int flags) :
    addresses(nullptr), errorCode(0)
{
    Resolve(host, addressFamily, socketType, flags);
}

HostResolver::~HostResolver()
{
    Reset();
}

HostResolver::HostResolver(HostResolver &&other) noexcept :
    addresses(other.addresses), errorCode(other.errorCode)
{
    other.addresses = nullptr;
    other.errorCode = 0;
}

HostResolver &
HostResolver::operator=(HostResolver &&other) noexcept
{
    if(this != &other)
    {
        Reset();
        addresses = other.addresses;
        errorCode = other.errorCode;
        other.addresses = nullptr;
        other.errorCode = 0;
    }
    return *this;
}

bool
HostResolver::Resolve(const std::string &host, int addressFamily,
                      int socketType, int flags)
{
    Reset();

    struct addrinfo hints = {};
    hints.ai_family = addressFamily;
    hints.ai_socktype = socketType;
    hints.ai_flags = flags;
    errorCode = getaddrinfo(host.c_str(), nullptr, &hints, &addresses);

    if(errorCode != 0 && addresses != nullptr)
    {
        freeaddrinfo(addresses);
        addresses = nullptr;
    }

    return IsValid();
}

void
HostResolver::Reset()
{
    if(addresses != nullptr)
    {
        freeaddrinfo(addresses);
        addresses = nullptr;
    }
    errorCode = 0;
}

bool
HostResolver::IsValid() const
{
    return addresses != nullptr;
}

int
HostResolver::GetErrorCode() const
{
    return errorCode;
}

const struct addrinfo *
HostResolver::GetAddresses() const
{
    return addresses;
}

const char *
HostResolver::GetCanonicalName() const
{
    return addresses != nullptr ? addresses->ai_canonname : nullptr;
}
