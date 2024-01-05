#include <stdlib.h>
#include <stdio.h>

#include "pipeline_editor.h"

void add(void **inputs, void **params, struct Pipeline_Component **outgoing_connections, int *outgoing_connection_input_indices);
void number_source(void **inputs, void **params, struct Pipeline_Component **outgoing_connections, int *outgoing_connection_input_indices);
void nop(void **inputs, void **params, struct Pipeline_Component **outgoing_connections, int *outgoing_connection_input_indices);

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

    pipeline_set_param(&pipeline, source_index_1, 0, &a);
    pipeline_set_param(&pipeline, source_index_2, 0, &b);

    pipeline_add_connection(&pipeline, source_index_1, 0, add_index, 0);
    pipeline_add_connection(&pipeline, source_index_2, 0, add_index, 1);

    pipeline_add_connection(&pipeline, add_index, 0, sink_component_index, 0);

    pipeline_execute(&pipeline);

    printf("%d\n", *((int *)(pipeline_fetch_inputs(&pipeline, sink_component_index)[0])));

    return 0;
}

void add(void **inputs, void **params, struct Pipeline_Component **outgoing_connections, int *outgoing_connection_input_indices)
{
    int a = *((int *)inputs[0]);
    int b = *((int *)inputs[1]);
    printf("%d, %d\n", a, b);
    outgoing_connections[outgoing_connection_input_indices[0]]->inputs[0] = malloc(sizeof(int));
    if(outgoing_connections[outgoing_connection_input_indices[0]]->inputs[0] == NULL)
    {
        fprintf(stderr, "main: malloc error\n");
        exit(4);
    }
    *((int *)outgoing_connections[0]->inputs[outgoing_connection_input_indices[0]]) = a + b;
}

void number_source(void **inputs, void **params, struct Pipeline_Component **outgoing_connections, int *outgoing_connection_input_indices)
{
    int num = *((int *)params[0]);
    outgoing_connections[0]->inputs[outgoing_connection_input_indices[0]] = malloc(sizeof(int));
    if(outgoing_connections[0]->inputs[0] == NULL)
    {
        fprintf(stderr, "main: malloc error\n");
        exit(4);
    }
    *((int *)outgoing_connections[0]->inputs[outgoing_connection_input_indices[0]]) = num;
}

void nop(void **inputs, void **params, struct Pipeline_Component **outgoing_connections, int *outgoing_connection_input_indices)
{
    return;
}