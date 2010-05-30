#ifndef DESKA_DB_NETWORK_H
#define DESKA_DB_NETWORK_H

#include <string>

namespace Deska {

namespace DB {

typedef std::string IPv4;
typedef std::string IPv6;

class Network {
public:
    std::string name() const;

    virtual int vlan() const;
    virtual void setVlan( const int num );

    virtual const IPv4& ipv4() const;
    virtual void setIpv4( const IPv4& addr );
    virtual const int ipv4Mask() const;
    virtual void setIpv4Mask( const int num );

    virtual const IPv6& ipv6() const;
    virtual void setIpv6( const IPv6& addr );
    virtual const int ipv6Mask() const;
    virtual void setIpv6Mask( const int num );

private:
    // we want to control the lifetime of these objects
    Network();
    Network( const Network& );
    ~Network();
};

}

}

#endif // DESKA_DB_NETWORK_H
