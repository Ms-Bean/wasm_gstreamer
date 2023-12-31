#include "graph.h"
int main(void)
{
	struct Graph graph;

	int p;
	p = 54;
	graph = graph_init(sizeof(int));

	graph_add_vertex(&graph, &p);
	p = 44;
	graph_add_vertex(&graph, &p);

	graph_add_edge(&graph, 1, 0);


	printf("%d\n", graph_has_edge(&graph, 1, 0));
	printf("%d\n", graph_has_edge(&graph, 1, 1));


	graph_destroy(&graph);
	return 0;
}
