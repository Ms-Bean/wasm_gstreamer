#include <stdlib.h>
#include <stdio.h>

#include "diagram_editor.h"

void add(void **inputs, void **params, void ***outputs);
void number_source(void **inputs, void **params, void ***outputs);
void nop(void **inputs, void **params, void ***outputs);

int main(void)
{
    int a, b;

    a = 4;
    b = 5;

    struct Pipeline pipeline = pipeline_init();
    int source_index_1 = pipeline_add_component(&pipeline, 0, 1, 1, number_source);
    int source_index_2 = pipeline_add_component(&pipeline, 0, 1, 1, number_source);
    int add_index = pipeline_add_component(&pipeline, 2, 0, 1, add);
    int sink_component_index = pipeline_add_component(&pipeline, 1, 0, 0, nop);

    pipeline_add_connection(&pipeline, source_index_1, 0, add_index, 0);
    pipeline_remove_component(&pipeline, sink_component_index);
    sink_component_index = pipeline_add_component(&pipeline, 1, 0, 0, nop);
    pipeline_add_connection(&pipeline, source_index_1, 0, add_index, 0);

    pipeline_set_param(&pipeline, source_index_1, 0, &a);
    pipeline_set_param(&pipeline, source_index_2, 0, &b);

    pipeline_add_connection(&pipeline, source_index_1, 0, add_index, 0);
    pipeline_add_connection(&pipeline, source_index_2, 0, add_index, 1);

    pipeline_add_connection(&pipeline, add_index, 0, sink_component_index, 0);

    pipeline_execute(&pipeline);
    printf("%d\n", *((int *)(pipeline_fetch_inputs(&pipeline, sink_component_index)[0])));

    pipeline_free_outputs(&pipeline);
    a = 10;
    b = 10;
    pipeline_execute(&pipeline);

    printf("%d\n", *((int *)(pipeline_fetch_inputs(&pipeline, sink_component_index)[0])));

    pipeline_destroy(&pipeline);
    return 0;
}

void add(void **inputs, void **params, void ***outputs)
{
    int a = *((int *)inputs[0]);
    int b = *((int *)inputs[1]);
    *(outputs[0]) = malloc(sizeof(int));
    if(*(outputs[0]) == NULL)
    {
        fprintf(stderr, "add: malloc error\n");
        exit(4);
    }
    *((int *)(*(outputs[0]))) = a + b;
}

void number_source(void **inputs, void **params, void ***outputs)
{
    int num = *((int *)params[0]);
    *(outputs[0]) = malloc(sizeof(int));
    if(*(outputs[0]) == NULL)
    {
        fprintf(stderr, "main: malloc error\n");
        exit(4);
    }
    *((int *)(*(outputs[0]))) = num;
}

void nop(void **inputs, void **params, void ***outputs)
{
    return;
}