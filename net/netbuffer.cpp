

#include <errno.h>
#include <algorithm>
#include "netbuffer.h"


using namespace SzFiu::Network;
using namespace std;

const char NetBuffer::kCRLF[] = "\r\n";

const size_t NetBuffer::kCheapPrepend;
const size_t NetBuffer::kInitialSize;

const char* NetBuffer::findCRLF() const
{
	// FIXME: replace with memmem()?
	const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
	return crlf == beginWrite() ? NULL : crlf;
}

const char* NetBuffer::findCRLF(const char* start) const
{
	assert(peek() <= start);
	assert(start <= beginWrite());
	// FIXME: replace with memmem()?
	const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
	return crlf == beginWrite() ? NULL : crlf;
}