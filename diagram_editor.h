#include "pipeline_components.h"

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

/*A wrapper around pipeline_components to represent the pipeline in ImGui*/
/*Note: component_data must occur at the top*/
/*It is there so we can cast (Diagram_Component *) to (Pipeline_Component *) */
/*C guarantees that there is no padding before it and that it occurs as the first element in memory.*/
/*This enables separation of concerns. Pipeline execution details are implemented in pipeline_components.h*/
/*While imgui details are managed here.*/
struct Diagram_Component
{
    struct Pipeline_Component component_data;

    char *title; 

    float pos_x;
    float pos_y;

    char **input_labels;
    char **output_labels;
    char **parameter_labels;

    bool is_active;
};

struct Diagram
{
    struct Pipeline pipeline;

    /*Stores the component index and output index of the last output button clicked*/
    /*Will be used for linking components through the GUI*/
    int component_clicked_index;
    int output_clicked_index;
};

struct Diagram diagram_init()
{
    struct Diagram out;
    out.pipeline = pipeline_init();
    out.component_clicked_index = -1;
    out.output_clicked_index = -1;
    return out;
}

int diagram_add_component(struct Diagram *diagram, char *title, float pos_x, float pos_y, char **input_labels, char **output_labels, char **parameter_labels, int n_inputs, int n_params, int n_outputs, void (*action)(void **, void **, void ***))
{
    struct Diagram_Component *new_component;
    char *new_title = NULL;
    int n_max = 0;
    int i;
    int pipeline_component_index = pipeline_add_component((struct Pipeline *)diagram, n_inputs, n_params, n_outputs, action);

    /*Extend the new component's memory to be sufficient for the Diagram elements*/
    diagram->pipeline.components[pipeline_component_index] = (struct Pipeline_Component *)realloc(diagram->pipeline.components[pipeline_component_index], sizeof(struct Diagram_Component));
    if(diagram->pipeline.components[pipeline_component_index] == NULL)
    {
        fprintf(stderr, "diagram_add_component: realloc error\n");
        exit(4);
    }
    /*We will assume all parameters are integers for now*/
    /*(easily changed)*/
    for(i = 0; i < n_params; i++)
    {
        int *p = (int *)malloc(sizeof(int));
        if(!p)
        {
            fprintf(stderr, "diagram_add_component: malloc error\n");
            exit(4);
        }
        *p = 0;
        pipeline_set_param((struct Pipeline *)diagram, pipeline_component_index, i, p);
    }
    /*Create a new title by appending a number after the old one*/
    for(i = 0; i < diagram->pipeline.n_components-1; i++)
    {
        struct Diagram_Component *component = (struct Diagram_Component *)(diagram->pipeline.components[i]);
        if(strlen(component->title) > strlen(title))
        {
            printf("%s, %s\n", component->title, title);
            if(strncmp(component->title, title, strlen(title)) == 0)
            {
                int title_num;
                title_num = atoi((char *)(component->title + strlen(title)));
                if(title_num > n_max)
                    n_max = title_num;
            }
        }
    }
    int char_count;
    char_count = snprintf(new_title, 0, "%s%d", title, (n_max + 1));
    new_title = (char *)malloc(sizeof(char) * (char_count + 1));
    if(new_title == NULL)
    {
        fprintf(stderr, "diagram_add_component: malloc error\n");
        exit(4);
    }
    snprintf(new_title, sizeof(char) * (char_count + 1), "%s%d", title, (n_max + 1));

    /*Re-interpret the new memory as a Diagram_Component*/
    new_component = (struct Diagram_Component *)diagram->pipeline.components[pipeline_component_index];

    new_component->title = new_title;
    new_component->pos_x = pos_x;
    new_component->pos_y = pos_y;
    new_component->input_labels = input_labels;
    new_component->output_labels = output_labels;
    new_component->parameter_labels = parameter_labels;
    new_component->is_active = 1;
    
    /*So, each component in the pipeline can be read as a Pipeline_Component with no issue*/
    /*However, each component has extra space allocated after it to contain the rest of the
    data stored in a Diagram_Component. It can be interpreted as either. (mimicking inheritance)*/

    return pipeline_component_index;
}

void diagram_add_connection(struct Diagram *diagram, int component_index_from, int output_index_from, int component_index_to, int input_index_to)
{
    pipeline_add_connection((struct Pipeline *)diagram, component_index_from, output_index_from, component_index_to, input_index_to);
}
void diagram_set_param(struct Diagram *diagram, int component_index, int param_index, void *param)
{
    pipeline_set_param((struct Pipeline *)diagram, component_index, param_index, param);
}
void diagram_remove_param(struct Diagram *diagram, int component_index, int param_index)
{
    pipeline_remove_param((struct Pipeline *)diagram, component_index, param_index);
}
void diagram_remove_connection(struct Diagram *diagram, int component_index_from, int output_index_from)
{
    pipeline_remove_connection((struct Pipeline *)diagram, component_index_from, output_index_from);
}
void diagram_remove_component(struct Diagram *diagram, int component_index)
{
    /*Each Pipeline_Component was realloced to have enough space to store the Diagram_Component elements*/
    /*However, it should be at the same address, so the calls to `free` made by pipeline_remove_component should free the Diagram_Component. */
    struct Diagram_Component *component = (struct Diagram_Component *)(diagram->pipeline.components[component_index]);
    int i;
    free(component->title);
    for(i = 0; i < ((struct Pipeline_Component *)component)->n_params; i++)
    {
        free(((struct Pipeline_Component *)component)->params[i]);
    }
    pipeline_remove_component((struct Pipeline *)diagram, component_index);

}

void diagram_free_outputs(struct Diagram *diagram)
{
    pipeline_free_outputs((struct Pipeline *)diagram);
}

void diagram_destroy(struct Diagram *diagram)
{
    int n_components = diagram->pipeline.n_components;
    int i;
    pipeline_free_outputs(&(diagram->pipeline));

    for(i = 0; i < n_components; i++)
        diagram_remove_component(diagram, 0);
    
    free(diagram->pipeline.components);
}

void show_diagram(struct Diagram *diagram)
{
    int i;
    for(i = 0; i < diagram->pipeline.n_components; i++)
    {
        int j;
        ImVec2 v;
        ImVec2 pos;
        struct Diagram_Component *component = (struct Diagram_Component *)(diagram->pipeline.components[i]);
        v.x = 200;
        v.y = 200;
        ImGui::SetNextWindowSize(v);
        ImGui::Begin(component->title, &(component->is_active), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        
        pos = ImGui::GetWindowPos();
        component->pos_x = pos.x;
        component->pos_y = pos.y;

        for(j = 0; j < ((struct Pipeline_Component *)component)->n_inputs; j++)
        {
            if(ImGui::Button(component->input_labels[j]))
            {
                if(diagram->component_clicked_index != -1 && diagram->output_clicked_index != -1)
                {
                    diagram_add_connection(diagram, diagram->component_clicked_index, diagram->output_clicked_index, i, j);
                    diagram->component_clicked_index = -1;
                    diagram->output_clicked_index = -1;
                }
            }
        }
        ImGui::NextColumn();        
        for(j = 0; j < ((struct Pipeline_Component *)component)->n_params; j++)
        {
            ImGui::InputInt(component->parameter_labels[j], (int *)((struct Pipeline_Component *)component)->params[j]);
        }
        ImGui::NextColumn();
        for(j = 0; j < ((struct Pipeline_Component *)component)->n_outputs; j++)
        {
            if(ImGui::Button(component->output_labels[j]))
            {
                if(diagram->component_clicked_index == i && diagram->output_clicked_index == j)
                {
                    diagram->component_clicked_index = -1;
                    diagram->output_clicked_index = -1;
                }
                diagram->component_clicked_index = i;
                diagram->output_clicked_index = j;
            }
            if(((struct Pipeline_Component *)component)->outgoing_connections[j] != NULL)
            {
                struct Diagram_Component *connected_component = (struct Diagram_Component *)(((struct Pipeline_Component *)component)->outgoing_connections[j]);

                ImVec2 from;
                ImVec2 to;

                from.x = component->pos_x;
                from.y = component->pos_y;

                to.x = connected_component->pos_x;
                to.y = connected_component->pos_y;
                ImGui::GetBackgroundDrawList()->AddLine(from, to, 0xFFFFFFFF, 5);
            }
        }
        /*Show execution results*/
        for(j = 0; j < ((struct Pipeline_Component *)component)->n_inputs; j++)
        {
            if(((struct Pipeline_Component *)component)->inputs[j] != NULL)
            {
                ImGui::Text("Input %d received: %d", j + 1, *((int *)(((struct Pipeline_Component *)component)->inputs[j])));
            }
        }
        ImGui::End();
    }
    ImGui::Begin("Execute", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    if(ImGui::Button("Execute"))
    {
        pipeline_free_outputs((struct Pipeline *)diagram);
        pipeline_execute((struct Pipeline *)diagram);
    }
    ImGui::End();
}