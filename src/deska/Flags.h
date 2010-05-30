/* The Deska::Flags class was shamelessly stolen from Qt's QFlags.  Therefore,
 * it has to respect the license of the QtCore/qglobal.h file.  This file is
 * therefore licensed under the terms of the LGPL version 2.1 or (at your
 * option) the GNU General Public License version 3.
 */

#ifndef DESKA_FLAGS_H
#define DESKA_FLAGS_H

namespace Deska {

typedef unsigned int uint;

template<typename Enum>
class Flags
{
    typedef void **Zero;
    int i;
public:
    typedef Enum enum_type;
    inline Flags(const Flags &f) : i(f.i) {}
    inline Flags(Enum f) : i(f) {}
    inline Flags(Zero = 0) : i(0) {}

    inline Flags &operator=(const Flags &f) { i = f.i; return *this; }
    inline Flags &operator&=(int mask) { i &= mask; return *this; }
    inline Flags &operator&=(uint mask) { i &= mask; return *this; }
    inline Flags &operator|=(Flags f) { i |= f.i; return *this; }
    inline Flags &operator|=(Enum f) { i |= f; return *this; }
    inline Flags &operator^=(Flags f) { i ^= f.i; return *this; }
    inline Flags &operator^=(Enum f) { i ^= f; return *this; }

    inline operator int() const { return i; }

    inline Flags operator|(Flags f) const { Flags g; g.i = i | f.i; return g; }
    inline Flags operator|(Enum f) const { Flags g; g.i = i | f; return g; }
    inline Flags operator^(Flags f) const { Flags g; g.i = i ^ f.i; return g; }
    inline Flags operator^(Enum f) const { Flags g; g.i = i ^ f; return g; }
    inline Flags operator&(int mask) const { Flags g; g.i = i & mask; return g; }
    inline Flags operator&(uint mask) const { Flags g; g.i = i & mask; return g; }
    inline Flags operator&(Enum f) const { Flags g; g.i = i & f; return g; }
    inline Flags operator~() const { Flags g; g.i = ~i; return g; }

    inline bool operator!() const { return !i; }

    inline bool testFlag(Enum f) const { return (i & f) == f && (f != 0 || i == int(f) ); }
};

#define DESKA_DECLARE_OPERATORS_FOR_FLAGS(MyFlags) \
inline ::Deska::Flags<MyFlags::enum_type> operator|(MyFlags::enum_type f1, MyFlags::enum_type f2) \
{ return ::Deska::Flags<MyFlags::enum_type>(f1) | f2; } \
inline ::Deska::Flags<MyFlags::enum_type> operator|(MyFlags::enum_type f1, ::Deska::Flags<MyFlags::enum_type> f2) \
{ return f2 | f1; }

#define DESKA_DECLARE_FLAGS(MyFlags, Enum)\
typedef ::Deska::Flags<Enum> MyFlags; \
DESKA_DECLARE_OPERATORS_FOR_FLAGS(MyFlags)

}

#endif // DESKA_FLAGS_H
