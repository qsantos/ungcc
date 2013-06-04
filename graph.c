#include "graph.h"

#include <math.h>

#define ATTRACT 0.1
#define PUSH    100000

static void attract(double* x, double* y, const struct block* a, const struct block* b)
{
	double dx = b->x - a->x;
	double dy = b->y - a->y;

	*x += dx * ATTRACT;
	*y += dy * ATTRACT;
}

static void pushAway(double* x, double* y, const struct block* a, const struct block* b)
{
	double dx = b->x - a->x;
	double dy = b->y - a->y;
	double d2 = dx*dx + dy*dy;

	if (d2 == 0)
	{
		*x += 1;
		*y += 1;
	}
	else
	{
		double factor = 1/sqrt(d2) * PUSH * a->size * b->size / d2;
		*x -= dx * factor;
		*y -= dy * factor;
	}
}

void spreadNodes(struct block* fun, size_t funlen)
{
	for (size_t k = 0; k < funlen; k++)
	{
		fun[k].x = rand() % 10000;
		fun[k].y = rand() % 10000;
	}
	for (size_t bla = 0; bla < 1000; bla++)
	{
		for (size_t k = 0; k < funlen; k++)
		{
			struct block* b = fun + k;
			double dx = 0;
			double dy = 2* ( (double)k - funlen/2 ); // verticality

			for (size_t k = 0; k < funlen; k++)
				if (fun[k].next == b || fun[k].branch == b)
					attract(&dx, &dy, b, fun+k);
				else
					pushAway(&dx, &dy, b, fun+k);

			if (b->next)   attract(&dx, &dy, b, b->next);
			if (b->branch) attract(&dx, &dy, b, b->branch);
			dx /= b->size;
			dy /= b->size;
			b->x += dx;
			b->y += dy;
		}
	}
}
