#include "pipeline_components.h"

struct Diagram_Component
{
    struct Pipeline_Component component_data;

    float pos_x;
    float pos_y;

    char **input_labels;
    char **output_labels;
    char **parameter_labels;

    unsigned int is_active;
};