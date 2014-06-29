#include <stdlib.h>
#include <string.h>
#include "SimUI.h"

static int             nuiHandlers = 0;
static sim_ui_element *uiHandlers = NULL;

static sim_ui_element *
sim_ui_find(const char *name)
{
    int i;
    for(i = 0; i < nuiHandlers; ++i)
    {
       if(strcmp(uiHandlers[i].name, name) == 0)
           return &uiHandlers[i];
    }
    return NULL;
}

static sim_ui_element *
sim_ui_add(void)
{
    sim_ui_element *newuiHandlers = NULL;
    newuiHandlers = (sim_ui_element *)malloc((nuiHandlers+1)*
       sizeof(sim_ui_element));

    if(uiHandlers != NULL)
    {
        memcpy(newuiHandlers, uiHandlers, nuiHandlers*sizeof(sim_ui_element));
        free(uiHandlers);
    }

    ++nuiHandlers;
    uiHandlers = newuiHandlers;
    memset(&uiHandlers[nuiHandlers-1], 0, sizeof(sim_ui_element));

    return &uiHandlers[nuiHandlers-1];
}

sim_ui_element *
sim_ui_get(const char *name)
{
    sim_ui_element *e = NULL;
    if((e = sim_ui_find(name)) == NULL)
    {
        e = sim_ui_add();
        e->name = strdup(name);
    }
    return e;
}

int
sim_ui_handle(const char *name, char *args)
{
    int handled = 0;
    const sim_ui_element *ui = sim_ui_find(name);
    if(ui != NULL)
    {
        /* Get the arguments. */
        char *cptr = NULL;
        char *signalName = NULL, *value = NULL;
        cptr = args;
        while(*cptr != ';')
            cptr++;
        *cptr++ = '\0';

        signalName = cptr;
        while(*cptr != ';')
            cptr++;
        *cptr++ = '\0';

        value = cptr;

        if(strcmp(signalName, "clicked()") == 0 &&
           ui->slot_clicked != NULL)
        {
            (*ui->slot_clicked)(ui->slot_clicked_data);
            ++handled;
        }

        if(strcmp(signalName, "valueChanged(int)") == 0 &&
           ui->slot_valueChanged != NULL)
        {
            int ivalue = atoi(value);
            (*ui->slot_valueChanged)(ivalue, ui->slot_valueChanged_data);
            ++handled;
        }

        if(strcmp(signalName, "stateChanged(int)") == 0 &&
           ui->slot_stateChanged != NULL)
        {
            int ivalue = atoi(value);
            (*ui->slot_stateChanged)(ivalue, ui->slot_stateChanged_data);
            ++handled;
        }
    }

    return (handled > 0) ? 1 : 0;
}
