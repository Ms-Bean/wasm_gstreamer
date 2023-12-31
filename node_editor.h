#include <stdio.h>
#include <stdlib.h>
struct Component_Data
{
    char *title;
    bool is_active;
    float pos_x;
    float pos_y;

    int num_inputs;
    bool *checkboxes_selected;

    bool output_checkbox_selected;
};

struct Component_Data create_component(char *title, int num_inputs)
{
    struct Component_Data out;
    int i;

    out.title = title;
    out.num_inputs = num_inputs;
    out.checkboxes_selected = (bool *)malloc(sizeof(bool) * num_inputs);
    if(out.checkboxes_selected == NULL)
    {
        printf("create_component: malloc error\n");
        exit(4);
    }
    for(i = 0; i < num_inputs; i++)
    {
        out.checkboxes_selected[i] = false;
    }
    out.pos_x = 0;
    out.pos_y = 0;
    out.is_active = true;

    return out;
}

void destroy_component(struct Component_Data *component)
{
    free(component->title);
}