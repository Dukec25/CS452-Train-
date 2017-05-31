#include <heap.h>

int is_heap_full(heap_t *ph)
{
	return (ph->len + 1) >= ph->size;
}

int is_heap_empty(heap_t *ph)
{
	return ph->len == 0;
}

heap_t heap_init(node_t *nodes, int size)
{
	heap_t h;
	h.nodes = nodes;
	h.size = size;
	h.len = 0;
	return h;
}

node_t heap_root(heap_t *ph)
{
	return ph->nodes[1];
}

int heap_insert(heap_t *ph, int priority, void *data)
{
	if (is_heap_full(ph)) return -1;

	node_t n;
	n.priority = priority;
	n.data = data; 
	int i = ph->len + 1;
	int j = i / 2;
	while (i > 1 && ph->nodes[j].priority < priority) {
		ph->nodes[i] = ph->nodes[j];
		i = j;
		j = j / 2;
	}
	ph->nodes[i] = n;
	ph->len++;
	return 0;
}
 
int heap_delete(heap_t *ph, node_t *proot)
{
	if (is_heap_empty(ph)) return -1;

	*proot = ph->nodes[1];
	ph->nodes[1] = ph->nodes[ph->len];
	ph->len--;

	int i, j, k;
	i = 1;
	while (1) {
		k = i;
		j = 2 * i;
		if (j <= ph->len && ph->nodes[j].priority > ph->nodes[k].priority) {
			k = j;
		}
		if (j + 1 <= ph->len && ph->nodes[j + 1].priority > ph->nodes[k].priority) {
			k = j + 1;
		}
		if (k == i) {
			break;
		}
		ph->nodes[i] = ph->nodes[k];
		i = k;
	}
	ph->nodes[i] = ph->nodes[ph->len + 1];
	return 0;
}
