#ifndef VISIT_ENGINE_H
#define VISIT_ENGINE_H

extern "C" {

// ****************************************************************************
//  Library:  VisItEngine
//
//  Purpose:
//    Wrapper for simulations to control an engine.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Nov  1 17:19:02 PST 2004
//    Added parallel simulation support.
//
// ****************************************************************************

void   *get_engine();
int     get_descriptor(void*);
int     initialize(void*, int argc, char *argv[]);
int     connect_to_viewer(void*, int argc, char *argv[]);
int     process_input(void*);
void    time_step_changed(void*);
void    disconnect();
void    set_slave_process_callback(void(*)());

}

#endif
