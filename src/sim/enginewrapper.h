#ifndef ENGINE_WRAPPER_H
#define ENGINE_WRAPPER_H

extern "C" {

void *get_engine();
int   get_descriptor(void*);
void  initialize(void*, int argc, char *argv[]);
void  connect_to_viewer(void*, int argc, char *argv[]);
void  process_input(void*);

}

#endif
