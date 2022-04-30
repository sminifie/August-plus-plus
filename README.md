# August++

A modern C++ library focusing on fast and efficient parsing of JSON. (Only parsing to begin with). Why "August"? It's the missing month abbreviation in JSON.

This is a highly optimised library that isn't intended to be a demonstration of a neat language implementation with a highly maintainable class structure. 
It's full of (legal) cheeky tricks to avoid or minimise any copies, moves or allocations. 
This turns a dangerous corner to low level (more like C) tactics using type aliasing. 
Good modern every-day C++ should enjoy a higher level abstraction and still be blisteringly fast; not sacrificing type and runtime safety by cutting corners like I do here to optimise the poop out of it.

Why would anyone want to do this? There are dozens of C++ JSON libraries out there but I couldn't find one that ticked all the right boxes. 
A lot didn't even compile on modern compilers, used techniques that no longer fit with the standard library, or are no longer maintained. 
Most support both reading and writing from the same structure and so made trade-offs and comprimises. 
I could fork any of them but I felt my desires and approach fundamentally differed from others. 
I would like to separate the read structure from write structure to eke out that extra little performance, and reduce allocations (some implementations submanaged allocations for speed, but not doing it at all will always be faster).

My goals:
* Only a single header include required (no .cpp files)
* Conformance checked, so badly formatted JSON throws instead of undefined behaviour
* Accurate floating point parsing (some libraries allow inaccuracies to creep in and diverge from the intended value)
* Compact, low memory overhead so great for embedded platforms
* Learning point for me: leverage C++20's UTF-8 support
* Not need RTTI and avoid vtables to help save storage space

Of course leveraging an existing codebase should be a quicker route to getting to a solution, but I'd really like to see how we can improve on what has gone before. I don't see why the whole serialisation of JSON may be considered so complicated. There are 7 possible marker tokens, 6 of them identifiable from the first character. I think UTF-8 is well known, used and understood these days. I want to learn by implementing my own take on the challenge.

A mutable buffer is one part of my approach that I think is novel. It gives us the ability to transform unicode code points in-place. Hand in hand with string_view, there are no string allocations or copies. The only concession there is that the buffer provided needs to be persisted for as long as any content from the document.

The zero termination requirement extends from my initial attemps parsing and comparing position with an end marker, having to pass that end marker around everywhere, and the fact that we're checking the value of each byte anyhow - why not with 0 too? So in making this awkward requirement we're reducing code complexity (comparing with 0 instead of two pointers for equality) and stack (removed a parameter from functions which potentially recurse quite deep).

This solution requires the whole JSON file to remain in memory, so is best recommended for small to medium files where this approach is acceptable. For embedded platforms parsing large files should perhaps consider looking for a sequential streaming (SAX-style) method instead.

So let's look at an example of use.
The buffer provided for parsing needs to exist for as long as the document, but for convenience in how that buffer is provided, it's not managed by this library.
~~~
#include <August++/Document.hpp>
#include <utility> // std::pair<>
#include <vector> // std::vector<>
#include <memory> // std::unique_ptr<> & friends

// An example of bundling the buffer and parsed content together so lifetimes match
using BufferAndDocument = std::pair<std::vector<August::Character>, std::unique_ptr<August::Document>>;

BufferAndDocument json;
// TODO: Allocate buffer "json.first.resize(<to number of bytes + 1 for null terminator>);"
// TODO: Copy content into "json.first.data();"
// TODO: Apply terminator "buffer[<number of bytes>] = 0;"
json.second = std::make_unique<August::Document>(json.first.data());
// Now "json.second" is the document and root token
// TODO: E.g. Use root token when execpting object "auto const& root = json.second->GetAs<August::Object>();"
~~~

