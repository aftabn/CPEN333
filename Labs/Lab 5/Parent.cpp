#include "Monitor.h"
#include "..\rt.h"

PerThreadStorage int myThreadNumber;

void ThreadPartB()
{
	Monitor m("Tango");

	for (int i = 0; i < 10000; i++) {
		m.Increment();
	}

	printf("The total is %d\n", m.Read());
}

void ThreadPartC()
{
	for (int i = 0; i < 10000; i++) {
		MOVE_CURSOR(20, 20);             // move cursor to cords [x,y] = 10,10
		printf("G'bye from Thread 2\n");
		fflush(stdout);		      // force output to be written to screen
	}
}

UINT __stdcall ChildThread(void *args)
{
	//ThreadPartB();
	ThreadPartC();

	return 0;
}

void PartB()
{
	int threadNums[2] = { 1, 2 };
	CThread *threads[2];

	for (int i = 0; i < 2; i++)
		threads[i] = new CThread(ChildThread, ACTIVE, &threadNums[i]);

	for (int i = 0; i < 2; i++)
		threads[i]->WaitForThread();

	printf("The reason for the mismatch is that thread one starts much %s", \
		"earlier than thread 2 due to initialization delays");
}

void PartC()
{
	int threadNum = 2;
	CThread *thread;
	thread = new CThread(ChildThread, ACTIVE, &threadNum);

	for (int i = 0; i < 10000; i++) {
		MOVE_CURSOR(10, 10);             // move cursor to cords [x,y] = 10,10
		printf("Hello from Thread 1\n");
		fflush(stdout);		      // force output to be written to screen
	}

	thread->WaitForThread();

	// The reason for this weirdness is likely due to the inabilty to control what gets printed first
	// at each new location. You can't guarentee what order any of the operations will happen.
	// The text on the new lines are due to the \n character at the end of each message.

	// I would fix this by adding a mutex for each set of the 3 operations
}

int main(void)
{
	//PartB();
	PartC();

	getchar();
	return 0;
}