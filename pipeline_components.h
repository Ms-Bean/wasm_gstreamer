#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Pipeline_Component
{
    int n_inputs; /*The number of input elements*/
    void **inputs; /*An array of input values.*/

    int n_params; /*Parameters associated with the component*/
    void **params; /*The parameters, can be set using pipeline_set_param*/

    int n_outputs; /*The number of outputs*/
    struct Pipeline_Component **outgoing_connections; /* The component that the output is fed as input to*/
    int *outgoing_connection_indices; /*The index of the input in that component */
    void ***outgoing_connection_inputs; /*An array of pointers to the inputs that the output is fed into*/

    /* The action function. Takes in the inputs, params, outgoing_connection_inputs for the first 3 parameters.*/
    /* It should read the input values and parameters
       and then it should allocate space for and send its output to the inputs of other components with the last argument*/
    void (*action)(void **, void **, void ***);
};
struct Pipeline
{
    int n_components;
    struct Pipeline_Component **components;
};

struct Pipeline pipeline_init(void)
{
    struct Pipeline out;
    out.n_components = 0;
    out.components = NULL;
    return out;
}

int pipeline_add_component(struct Pipeline *pipeline, int n_inputs, int n_params, int n_outputs, void (*action)(void **, void **, void ***))
{
    struct Pipeline_Component *new_component;

    new_component = (struct Pipeline_Component *)malloc(sizeof(struct Pipeline_Component));
    if(new_component == NULL)
    {
        fprintf(stderr, "pipeline_add_component: malloc error\n");
        exit(4);
    }

    new_component->n_inputs = n_inputs;
    if(n_inputs > 0)
    {
        new_component->inputs = (void **)calloc(n_inputs, sizeof(void *));
        if(new_component->inputs == NULL)
        {
            fprintf(stderr, "pipeline_add_component: calloc error\n");
            exit(4);
        }
    }
    else
    {
        new_component->inputs = NULL;
    }
    new_component->n_params = n_params;
    if(n_params > 0)
    {
        new_component->params = (void **)malloc(n_params * sizeof(void *));
        if(new_component->params == NULL)
        {
            fprintf(stderr, "pipeline_add_component: malloc error\n");
            exit(4);
        }
    }
    else
    {
        new_component->params = NULL;
    }
    new_component->n_outputs = n_outputs;
    if(n_outputs > 0)
    {
        new_component->outgoing_connections = (struct Pipeline_Component **)calloc(n_outputs, sizeof(struct Pipeline_Component *));
        if(new_component->outgoing_connections == NULL)
        {
            fprintf(stderr, "pipeline_add_component: calloc error\n");
            exit(4);
        }

        new_component->outgoing_connection_indices = (int *)calloc(n_outputs, sizeof(int));
        if(new_component->outgoing_connection_indices == NULL)
        {
            fprintf(stderr, "pipeline_add_component: calloc error\n");
            exit(4);
        }
        new_component->outgoing_connection_inputs = (void ***)calloc(n_outputs, sizeof(void **));
        if(new_component->outgoing_connection_inputs == NULL)
        {
            fprintf(stderr, "pipeline_add_component: calloc error\n");
            exit(4);
        }
    }
    else
    {
        new_component->outgoing_connections = NULL;
        new_component->outgoing_connection_indices = NULL;
        new_component->outgoing_connection_inputs = NULL;
    }

    pipeline->components = (struct Pipeline_Component **)realloc(pipeline->components, sizeof(struct Pipeline_Component * ) * (pipeline->n_components + 1));
    if(pipeline->components == NULL)
    {
        fprintf(stderr, "pipeline_add_component: realloc error\n");
        exit(4);
    }
    new_component->action = action;
    pipeline->components[pipeline->n_components] = new_component;

    return (pipeline->n_components)++;
}
void pipeline_add_connection(struct Pipeline *pipeline, int component_index_from, int output_index_from, int component_index_to, int input_index_to)
{
    pipeline->components[component_index_from]->outgoing_connections[output_index_from] = pipeline->components[component_index_to];
    pipeline->components[component_index_from]->outgoing_connection_indices[output_index_from] = input_index_to;
    pipeline->components[component_index_from]->outgoing_connection_inputs[output_index_from] = &(pipeline->components[component_index_to]->inputs[input_index_to]);
}
void pipeline_set_param(struct Pipeline *pipeline, int component_index, int param_index, void *param)
{
    pipeline->components[component_index]->params[param_index] = param;
}
void pipeline_remove_param(struct Pipeline *pipeline, int component_index, int param_index, void *param)
{
    pipeline->components[component_index]->params[param_index] = NULL;
}
void pipeline_remove_connection(struct Pipeline *pipeline, int component_index_from, int output_index_from)
{
    pipeline->components[component_index_from]->outgoing_connections[output_index_from] = NULL;
    pipeline->components[component_index_from]->outgoing_connection_indices[output_index_from] = 0;
    pipeline->components[component_index_from]->outgoing_connection_inputs[output_index_from] = NULL;
}
void pipeline_remove_component(struct Pipeline *pipeline, int component_index)
{
    int i;
    struct Pipeline_Component *removed = pipeline->components[component_index];

    /*Remove all outgoing connections to the component to be removed*/
    for(i = 0; i < pipeline->n_components; i++)
    {
        int j;
        for(j = 0; j < pipeline->components[i]->n_outputs; j++)
        {
            if(pipeline->components[i]->outgoing_connections[j] == removed)
                pipeline_remove_connection(pipeline, i, j);
        }
    }
    free(removed->outgoing_connections);
    free(removed->outgoing_connection_indices);
    free(removed->outgoing_connection_inputs);
    free(removed->inputs);
    free(removed->params);
    free(removed);

    for(i = component_index; i < pipeline->n_components - 1; i++)
        pipeline->components[i] = pipeline->components[i+1];
    
    (pipeline->n_components)--;
    if(pipeline->n_components == 0)
    {
        free(pipeline->components);
        pipeline->components = NULL;
    }
    else
    {
        pipeline->components = (struct Pipeline_Component **)realloc(pipeline->components, sizeof(struct Pipeline_Component *) * pipeline->n_components);
        if(pipeline->components == NULL)
        {
            fprintf(stderr, "pipeline_remove_component: realloc error\n");
            exit(4);
        }
    }
}

/*Frees all stored outputs after pipeline execution*/
void pipeline_free_outputs(struct Pipeline *pipeline)
{
    int i;
    for(i = 0; i < pipeline->n_components; i++)
    {
        int j;
        for(j = 0; j < pipeline->components[i]->n_inputs; j++)
        {
            free(pipeline->components[i]->inputs[j]);
            pipeline->components[i]->inputs[j] = NULL;
        }
    }
}

void pipeline_destroy(struct Pipeline *pipeline)
{
    int i;
    int n_components = pipeline->n_components;

    pipeline_free_outputs(pipeline);

    for(i = 0; i < n_components; i++)
        pipeline_remove_component(pipeline, 0);
    
    free(pipeline->components);
}

/*The execution algorithm.*/
/*It will continuously scan through the components and execute the action
function in components that have all if its inputs set until there are none left.*/
void pipeline_execute(struct Pipeline *pipeline)
{
    int i;
    int *visited = (int *)calloc(pipeline->n_components, sizeof(int));
    int visited_count = 0;
    if(visited == NULL)
    {
        fprintf(stderr, "pipeline_execute: calloc error\n");
        exit(4);
    }
    for(;;)
    {
        int prev = visited_count;
        for(i = 0; i < pipeline->n_components; i++)
        {
            int j;
            int flag = 0;
            if(visited[i])
                continue;
            for(j = 0; j < pipeline->components[i]->n_inputs; j++)
            {
                if(pipeline->components[i]->inputs[j] == NULL)
                {
                    flag = 1;
                    break;
                }
            }
            if(flag == 1)
                continue;
            pipeline->components[i]->action(pipeline->components[i]->inputs, pipeline->components[i]->params, pipeline->components[i]->outgoing_connection_inputs);
            visited[i] = 1;
            visited_count++;
        }
        if(visited_count == prev)
            break;
    }
    free(visited);
}
/*Fetches the inputs fed into a component.
 Useful for retrieving final outputs fed into a sink component*/
void **pipeline_fetch_inputs(struct Pipeline *pipeline, int component_index)
{
    return pipeline->components[component_index]->inputs;
}