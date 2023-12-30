#include "graph.h"
int main(void)
{
	struct Graph graph;

	int p;
	p = 54;
	graph = graph_init(sizeof(int), 10);
	graph_setval(&graph, 3, &p);
	graph_resize(&graph, 100);
	printf("%d\n", *((int *)graph_getval(&graph, 3)));

	graph_add_edge(&graph, 1, 3);

	printf("%d, %d, %d\n", graph_has_edge(&graph, 3, 1), graph_has_edge(&graph, 1, 3), graph_has_edge(&graph, 4, 3));
	graph_delete_edge(&graph, 1, 3);
	printf("%d, %d, %d\n", graph_has_edge(&graph, 3, 1), graph_has_edge(&graph, 1, 3), graph_has_edge(&graph, 4, 3));
	graph_resize(&graph, 10);
	graph_resize(&graph, 16);
	graph_add_edge(&graph, 3, 1);
	printf("%d, %d, %d\n", graph_has_edge(&graph, 3, 1), graph_has_edge(&graph, 1, 3), graph_has_edge(&graph, 4, 3));
	graph_destroy(&graph);
	return 0;
}
