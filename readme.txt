GOST R 34.11-2012 (streebog) hash function library and wrapper

Modifications by me:
Removed buggy MMX optimizations
Fixed compilation errors on SSE41 header
Added 2 pairs of brackets around one statement guard with macro
Added guards on all headers
Moved string.h inclusion
Added stddef inclusion
Replaced wrapper utility (checksum)

In the release page you can find optimized binaries packages for IA32/x86-64

Links:
http://tools.ietf.org/html/rfc6986
http://www.streebog.info

LICENSE:
The library author is Alexey Degtyarev (www.streebog.net)
See LICENSE file for details
I made some bugfix/change to the library.

I rewrote the wrapper from scratch (streebog.c), that is GPL lincensed.
