#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
struct Graph
{
    int num_vertices;

    size_t size;
    void **vertices;

    unsigned char **adjacency_matrix;
};

struct Graph graph_init(size_t size)
{
    struct Graph graph;

    graph.size = size;
    graph.num_vertices = 0;

    graph.adjacency_matrix = NULL;
    graph.vertices = NULL;

    return graph;
}
void graph_set_edge(struct Graph *graph, int from, int to, unsigned char weight)
{
    graph->adjacency_matrix[from][to] = weight;
}
unsigned char graph_get_edge(struct Graph *graph, int from, int to)
{
    return graph->adjacency_matrix[from][to];
}
void graph_add_vertex(struct Graph *graph, void *val)
{
    int i;

    /*Update the vertex count*/
    graph->num_vertices++;

    /*Create and insert the value into the new vertex*/
    graph->vertices = (void **)realloc(graph->vertices, sizeof(void *) * graph->num_vertices);
    if(graph->vertices == NULL)
    {
        printf("graph_add_vertex: realloc error\n");
        exit(4);
    }
    graph->vertices[graph->num_vertices - 1] = malloc(graph->size);
    if(graph->vertices[graph->num_vertices - 1] == NULL)
    {
        printf("graph_setval: malloc error\n");
    }
    memcpy(graph->vertices[graph->num_vertices - 1], val, graph->size);

    /*Update the adjacency matrix*/
    graph->adjacency_matrix = (unsigned char **)realloc(graph->adjacency_matrix, sizeof(unsigned char *) * graph->num_vertices);
    if(graph->adjacency_matrix == NULL)
    {
        printf("graph_add_vertex: realloc error\n");
        exit(4);
    }
    graph->adjacency_matrix[graph->num_vertices - 1] = (unsigned char *)calloc(graph->num_vertices, sizeof(unsigned char));
    if(graph->adjacency_matrix[graph->num_vertices - 1] == NULL)
    {
        printf("graph_add_vertex: calloc error\n");
        exit(4);
    }
    for(i = 0; i < graph->num_vertices - 1; i++)
    {
        graph->adjacency_matrix[i] = (unsigned char *)realloc(graph->adjacency_matrix[i], sizeof(unsigned char) * graph->num_vertices);
        if(graph->adjacency_matrix[i] == NULL)
        {
            printf("graph_add_vertex: realloc error\n");
            exit(4);
        }
        graph->adjacency_matrix[i][graph->num_vertices - 1] = 0;
    }
}
void graph_delete_vertex(struct Graph *graph, int index)
{
    int i, j;

    /*
        Case: There is only one vertex. 
        The index must be 0.
        Free everything and set to NULL
        to avoid calling realloc(X, 0);
    */
    if(graph->num_vertices == 1)
    {
        free(graph->vertices[0]);
        free(graph->adjacency_matrix[0]);
        free(graph->vertices);
        free(graph->adjacency_matrix);
        graph->vertices = NULL;
        graph->adjacency_matrix = NULL;
        graph->num_vertices = 0;

        return;
    }

    /* << */
    for(i = 0; i < graph->num_vertices; i++)
        for(j = index; j < graph->num_vertices-1; j++)
            graph_set_edge(graph, i, j, graph_get_edge(graph, i, j+1));

    /* ^^ */
    free(graph->adjacency_matrix[index]);
    memmove(graph->adjacency_matrix + index, graph->adjacency_matrix + index + 1, sizeof(unsigned char *) * (graph->num_vertices - index - 1));
    
    free(graph->vertices[index]);
    memmove(graph->vertices + index, graph->vertices + index + 1, sizeof(void *) * (graph->num_vertices - index - 1));

    graph->num_vertices--;
    for(i = 0; i < graph->num_vertices; i++)
    {
        graph->adjacency_matrix[i] = (unsigned char *)realloc(graph->adjacency_matrix[i], sizeof(unsigned char) * graph->num_vertices);
        if(graph->adjacency_matrix[i] == NULL)
        {
            printf("graph_delete_vertex: realloc error\n");
            exit(4);
        }
    }
}
void graph_setval(struct Graph *graph, int vertex, void *val)
{
    memcpy(graph->vertices[vertex], val, graph->size);
}
void *graph_getval(struct Graph *graph, int vertex)
{
    return graph->vertices[vertex];
}
void graph_destroy(struct Graph *graph)
{
    int i;
    if(graph->vertices == NULL) /*In case all the vertices had been manually deleted 1 by 1*/
        return;
    for(i = 0; i < graph->num_vertices; i++)
    {
        free(graph->vertices[i]);
        free(graph->adjacency_matrix[i]);
    }
    free(graph->adjacency_matrix);
    free(graph->vertices);
}