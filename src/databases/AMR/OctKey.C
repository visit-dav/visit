    
#include <OctKey.h>

int  octkey_root_pos;
const long KidTemplate=0x0007;


int ParseOctkey( long key, int lvl, long& root, int* kids )
{
  long tmp=key;
  long 

  root = key;

  for( int i=0; i<lvl-1; i++ ) {
    kk[i]= int( tmp & KidTemplate );
    tmp>>=3;
  }

  for( int i=0; i<21-3; i++ ) {
    if(
  }

}
