#ifndef SIM_UI_H
#define SIM_UI_H

typedef struct 
{
    char *name;

    /* Data for calling user-defined slots. */
    void (*slot_clicked)(void*);
    void *slot_clicked_data;

    void (*slot_valueChanged)(int, void*);
    void *slot_valueChanged_data;

    void (*slot_stateChanged)(int, void *);
    void *slot_stateChanged_data;

} sim_ui_element;

int             sim_ui_handle(const char *name, char *args);
sim_ui_element *sim_ui_get(const char *name);

#endif
