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

struct Graph graph_init(size_t size, int num_vertices)
{
    struct Graph graph;
    int i;

    graph.size = size;
    graph.num_vertices = num_vertices;

    graph.adjacency_matrix = (unsigned char **)malloc(sizeof(unsigned char *) * graph.num_vertices);
    if(graph.adjacency_matrix == NULL)
    {
        printf("graph_init: malloc error\n");
        exit(4);
    }
    for(i = 0; i < graph.num_vertices; i++)
    {
        graph.adjacency_matrix[i] = (unsigned char *)calloc(graph.num_vertices / CHAR_BIT + 1, sizeof(unsigned char));
        if(graph.adjacency_matrix[i] == NULL)
        {
            printf("graph_init: calloc error\n");
            exit(4);
        }
    }
    graph.vertices = (void **)calloc(graph.num_vertices, sizeof(void *));
    if(graph.vertices == NULL)
    {
        printf("graph_init: calloc error\n");
        exit(4);
    }
    return graph;
}
void graph_resize(struct Graph *graph, int num_vertices)
{
    int i;

    for(i = num_vertices; i < graph->num_vertices; i++)
    {
        if(graph->vertices[i] != NULL)
            free(graph->vertices[i]);    
        free(graph->adjacency_matrix[i]);
    }
    
    graph->adjacency_matrix = (unsigned char **)realloc(graph->adjacency_matrix, sizeof(unsigned char *) * num_vertices);
    if(graph->adjacency_matrix == NULL)
    {
        printf("graph_resize: realloc error\n");
        exit(4);
    }

    graph->vertices = (void **)realloc(graph->vertices, sizeof(void *) * num_vertices);
    if(graph->vertices == NULL)
    {
        printf("graph_resize: realloc error\n");
        exit(4);
    }

    if(num_vertices > graph->num_vertices)
    {
        memset(graph->vertices + graph->num_vertices, 0, sizeof(void *) * (num_vertices - graph->num_vertices));
        memset(graph->adjacency_matrix + graph->num_vertices, 0, sizeof(unsigned char *) * (num_vertices - graph->num_vertices));

        for(i = 0; i < num_vertices; i++)
        {
            graph->adjacency_matrix[i] = (unsigned char *)realloc(graph->adjacency_matrix[i], sizeof(unsigned char) * (num_vertices / CHAR_BIT + 1));
            if(graph->adjacency_matrix[i] == NULL)
            {
                printf("graph_resize: realloc error\n");
                exit(4);
            }
            memset(graph->adjacency_matrix[i] + (graph->num_vertices / CHAR_BIT + 1), 0, sizeof(unsigned char) * (num_vertices/CHAR_BIT - graph->num_vertices/CHAR_BIT));
        }
    }
    else
    { 
        for(i = 0; i < num_vertices; i++)
        {
            printf("%d\n", i);
            graph->adjacency_matrix[i] = (unsigned char *)realloc(graph->adjacency_matrix[i], sizeof(unsigned char) * (num_vertices / CHAR_BIT + 1));
            if(graph->adjacency_matrix[i] == NULL)
            {
                printf("graph_resize: realloc error\n");
                exit(4);
            }
        }
    }
    graph->num_vertices = num_vertices;
}
void graph_add_edge(struct Graph *graph, int from, int to)
{
    graph->adjacency_matrix[from][to/CHAR_BIT] |= (1 << (to % CHAR_BIT));
}
void graph_delete_edge(struct Graph *graph, int from, int to)
{
    graph->adjacency_matrix[from][to/CHAR_BIT] &= (UCHAR_MAX - (1 << (to % CHAR_BIT)));
}
int graph_has_edge(struct Graph *graph, int from, int to)
{
    return graph->adjacency_matrix[from][to/CHAR_BIT] && (UCHAR_MAX - (1 << (to % CHAR_BIT)));
}
void graph_setval(struct Graph *graph, int vertex, void *val)
{
    if(graph->vertices[vertex] == NULL) 
    {
        graph->vertices[vertex] = malloc(graph->size);
    }
    if(graph->vertices[vertex] == NULL)
    {
        printf("graph_setval: malloc error\n");
    }
    memcpy(graph->vertices[vertex], val, graph->size);
}
void *graph_getval(struct Graph *graph, int vertex)
{
    return graph->vertices[vertex];
}
void graph_destroy(struct Graph *graph)
{
    int i;
    for(i = 0; i < graph->num_vertices; i++)
    {
        if(graph->vertices[i] != NULL)
            free(graph->vertices[i]);

        free(graph->adjacency_matrix[i]);
    }
    free(graph->adjacency_matrix);
    free(graph->vertices);
}