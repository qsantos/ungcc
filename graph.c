#include "graph.h"

#include <math.h>

#define ATTRACT 0.01
#define PUSH    100000

static void attract(double* x, double* y, const block_t* a, const block_t* b, double f)
{
	double dx = b->x - a->x;
	double dy = b->y - (a->y + a->h + 200);
	dx *= 10;
	dy *= 1 + 4*(dy < 0);

	*x -= dx * ATTRACT * f;
	*y -= dy * ATTRACT * f;
}

static void pushAway(double* x, double* y, const block_t* a, const block_t* b)
{
	double dx = b->x - a->x;
	double dy = b->y - a->y;
	double d2 = dx*dx + dy*dy;

	if (d2 == 0)
		return;

	double factor = 1/sqrt(d2) * PUSH * a->size * b->size / d2;
	*x -= dx * factor;
	(void) y;
//	*y -= dy * factor;
}

void spreadNodes(block_t* fun, size_t funlen)
{
	double cury = 0;
	for (size_t k = 0; k < funlen; k++)
	{
		fun[k].x = rand() % 10000;
		fun[k].y = cury;
		cury += fun[k].h + 100;
	}
	for (size_t bla = 0; bla < 1000; bla++)
	{
		for (size_t k = 0; k < funlen; k++)
		{
			block_t* b = fun + k;
			double dx = 0;
			double dy = 0;// 2* ( (double)k - funlen/2 ); // verticality

			for (size_t k = 0; k < funlen; k++)
				if (fun+k == b->next || fun+k == b->branch);     // b is parent
				else if (fun[k].next == b) // b is next
					attract(&dx, &dy, fun+k, b, 10);
				else if (fun[k].branch == b) // b is branch
					attract(&dx, &dy, fun+k, b, 1);
				else
					pushAway(&dx, &dy, b, fun+k);

//			if (b->next)   attract(&dx, &dy, b, b->next);
//			if (b->branch) attract(&dx, &dy, b, b->branch);
//			dx /= b->size;
//			dy /= b->size;
			b->x += dx;
			b->y += dy;
		}
	}
}
