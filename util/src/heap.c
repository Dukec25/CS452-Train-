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
	while (i > 1 && ph->nodes[j].priority > priority) {
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
    debug(DEBUG_INFO, "new node idx=%d", ph->nodes[1].priority);
	ph->len--;

	int parent_idx, children_idx, k;
	parent_idx = 1;
	while (1) {
		k = parent_idx;
		children_idx = 2 * parent_idx;
        /*debug(DEBUG_INFO, "left child priority idx=%d", ph->nodes[children_idx].priority);*/
        /*debug(DEBUG_INFO, "parent node priority idx=%d", ph->nodes[k].priority);*/
		if (children_idx <= ph->len && ph->nodes[children_idx].priority < ph->nodes[k].priority) {
			k = children_idx;
		}
        /*debug(DEBUG_INFO, "right child priority idx=%d", ph->nodes[children_idx+1].priority);*/
		if (children_idx + 1 <= ph->len && ph->nodes[children_idx + 1].priority < ph->nodes[k].priority) {
			k = children_idx + 1;
		}
		if (k == parent_idx) {
			break;
		}
        /*debug(DEBUG_INFO, "exchanged i prior idx=%d", ph->nodes[i].priority);*/
        /*debug(DEBUG_INFO, "exchanged k prior idx=%d", ph->nodes[k].priority);*/

        node_t temp = ph->nodes[parent_idx]; 
		ph->nodes[parent_idx] = ph->nodes[k];
        ph->nodes[k] = temp;

		parent_idx = k;
        /*debug(DEBUG_INFO, "i idx=%d", i);*/
        /*debug(DEBUG_INFO, "ph len =%d", ph->len);*/
        
	}
	ph->nodes[parent_idx] = ph->nodes[ph->len + 1];
	return 0;
}
