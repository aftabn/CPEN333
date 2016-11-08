#include "Car.h"
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include "..\rt.h"

void PartA()
{
	Car	a(1), b(2), c(3);
	Car	d(4), e(5), f(6);
	Car	g(7), h(8), i(9), j(10);

	a.Resume();
	b.Resume();
	c.Resume();
	d.Resume();
	e.Resume();
	f.Resume();
	g.Resume();
	h.Resume();
	i.Resume();
	j.Resume();

	a.WaitForThread();
	b.WaitForThread();
	c.WaitForThread();
	d.WaitForThread();
	e.WaitForThread();
	f.WaitForThread();
	g.WaitForThread();
	h.WaitForThread();
	i.WaitForThread();
	j.WaitForThread();

	printf("Finished\n");
}

void CallRandomFunction(Car &car, int r)
{
	if (r == 0) car.Accelerate();
	if (r == 1) car.Cruise();
	else car.Stop();
}

void PartB()
{
	/* initialize random seed: */
	srand(time(NULL));

	Car	a(1), b(2), c(3);
	Car	d(4), e(5), f(6);
	Car	g(7), h(8), i(9), j(10);

	a.Resume();
	b.Resume();
	c.Resume();
	d.Resume();
	e.Resume();
	f.Resume();
	g.Resume();
	h.Resume();
	i.Resume();
	j.Resume();

	for (int k = 0; k < 50; k++)
	{
		int r = rand() % 3;

		if (k % 10 == 0) CallRandomFunction(a, r);
		if (k % 10 == 1) CallRandomFunction(b, r);
		if (k % 10 == 2) CallRandomFunction(c, r);
		if (k % 10 == 3) CallRandomFunction(d, r);
		if (k % 10 == 4) CallRandomFunction(e, r);
		if (k % 10 == 5) CallRandomFunction(f, r);
		if (k % 10 == 6) CallRandomFunction(g, r);
		if (k % 10 == 7) CallRandomFunction(h, r);
		if (k % 10 == 8) CallRandomFunction(i, r);
		else CallRandomFunction(j, r);

		SLEEP(100);
	}

	a.WaitForThread();
	b.WaitForThread();
	c.WaitForThread();
	d.WaitForThread();
	e.WaitForThread();
	f.WaitForThread();
	g.WaitForThread();
	h.WaitForThread();
	i.WaitForThread();
	j.WaitForThread();

	printf("Finished\n");
}

int main(void)
{
	PartB();

	return 0;
}