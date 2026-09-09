/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:19:20 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:19:21 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_init(t_heap *heap, int capacity)
{
	heap->data = malloc(sizeof(t_request) * capacity);
	if (!heap->data)
		return (0);
	heap->size = 0;
	heap->capacity = capacity;
	return (1);
}

void	heap_destroy(t_heap *heap)
{
	free(heap->data);
	heap->data = NULL;
}

int	heap_insert(t_heap *heap, t_request request)
{
	if (heap->size >= heap->capacity)
		return (0);
	heap->data[heap->size] = request;
	heapify_up(heap, heap->size);
	heap->size++;
	return (1);
}

static int	heap_peek(t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (0);
	*out = heap->data[0];
	return (1);
}

int	heap_remove_min(t_heap *heap, t_request *out)
{
	if (!heap_peek(heap, out))
		return (0);
	heap->size--;
	heap->data[0] = heap->data[heap->size];
	heapify_down(heap, 0);
	return (1);
}
