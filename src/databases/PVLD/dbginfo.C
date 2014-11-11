#include <DebugStream.h>
#include <avtMemory.h>

#include <statm.h>

void OutputMemoryInfo( const char* str )
{
#if 0
  int totsize,rss,share,text,lib,data,dt;
  getstatm_( &totsize, &rss, &share, &text, &lib, &data, &dt);
  debug1 << str << " TOT(" << totsize << ")  RSS(" << rss << ").\n";
#else
  unsigned long size, rss2;
  avtMemory::GetMemorySize(size, rss2);
  debug1 << str << " [VISIT] SIZE(" << size << ")  RSS(" << rss2 << ").\n";
#endif
}

