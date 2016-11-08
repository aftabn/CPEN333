#include "../rt.h"
#include <sstream>

// put your semaphores here as global variables accessible by all threads
CSemaphore *entryGate, *exitGate;
CSemaphore *full, *Empty;

struct mydatapooldata {
	int passengerCount;
	int rideCount;
	bool isLoading;
};

struct mydatapooldata *dataPtr;

class Passenger : public ActiveClass
{
public: 	Passenger(int n) { passengerNum = n; }

private:
	int passengerNum;

	int main(void)
	{
		while (1)
		{
			entryGate->Wait();
			full->Signal();

			exitGate->Wait();
			Empty->Signal();
		}

		return 0;
	}
};

class RollerCoaster : public ActiveClass
{
public: 	RollerCoaster() { }
private:
	int main(void)
	{
		while (1)
		{
			for (int i = 0; i < 30; i++) entryGate->Signal();
			for (int i = 0; i < 30; i++) full->Wait();

			printf("Roller coaster is going for a ride\n");
			SLEEP(750);

			for (int i = 0; i < 30; i++)  exitGate->Signal();
			for (int i = 0; i < 30; i++)  Empty->Wait();
		}

		return 0;
	}
};

int main()
{
	entryGate = new CSemaphore("Entry", 0, 30);
	exitGate = new CSemaphore("Exit", 0, 30);
	full = new CSemaphore("Full", 0, 30);
	Empty = new CSemaphore("Empty", 0, 30);

	RollerCoaster car;
	car.Resume();

	SLEEP(1000);			// wait for it initialise itself

	Passenger	*thePassengers[100];

	for (int i = 0; i < 100; i++) {
		thePassengers[i] = new Passenger(i);
		thePassengers[i]->Resume();
	}

	// wait for rollercoater thread to terminate, even though it doesn't to prevent the parent thread
	// (i.e. this one from terminating and halting the program).

	car.WaitForThread();
	return 0;
}