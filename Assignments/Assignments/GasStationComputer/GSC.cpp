#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\rt.h"
#include "FuelTankStation.h"

const int INT_NumPumps = 1;
bool isPumpAuthorized[INT_NumPumps];
bool needPumpAuthorization[INT_NumPumps];

CMutex mutex(string("__Mutex__") + string("GSC"));
CSemaphore *cSemaphores[INT_NumPumps];
CSemaphore *pSemaphores[INT_NumPumps];
CDataPool *dps[INT_NumPumps];
struct GasPumpData *gpData[INT_NumPumps];

PerThreadStorage int threadPumpNumber;

char gChar1;
char gChar2;

struct GasPumpData
{
	long long creditCard;
	int fuelGrade;
	double dispensedVolume;
	bool isAuthorized;
	bool isDone;
	string customerName;
};

UINT __stdcall PumpThread(void *args)	// thread function
{
	threadPumpNumber = *(int *)(args);

	while (1)
	{
		pSemaphores[threadPumpNumber]->Wait();

		// Create function to alert gas station computer that a pump needs authorization
		mutex.Wait();
		needPumpAuthorization[threadPumpNumber] = true;
		printf("Pump %d needs authorization.\n", threadPumpNumber);
		mutex.Signal();

		// Wait until GSC authorizes, and then convey this to pump
		while (!isPumpAuthorized[threadPumpNumber]) SLEEP(100);
		gpData[threadPumpNumber]->isAuthorized = true;

		cSemaphores[threadPumpNumber]->Signal();

		// Wait until the dispensing is finished
		while (!gpData[threadPumpNumber]->isDone)
		{
			pSemaphores[threadPumpNumber]->Wait();

			mutex.Wait();
			printf("Pump %d: Dispensed Vol.: %3.1f", threadPumpNumber,
				gpData[threadPumpNumber]->dispensedVolume);
			mutex.Signal();

			cSemaphores[threadPumpNumber]->Signal();
		}

		// Reset the bool for the next customer and continue
		pSemaphores[threadPumpNumber]->Wait();
		gpData[threadPumpNumber]->isDone = false;
		cSemaphores[threadPumpNumber]->Signal();
	}

	return 0;
}

void ProcessCommand()
{
	string command = "" + gChar1 + gChar2;

	if (command == "RF")
	{
		mutex.Wait();
		printf("Refilling fuel tanks\n");
		mutex.Signal();

		FuelTankStation::RefillTanks();
	}
	else if (gChar1 == 'F' && (gChar2 - '0' >= 0 && gChar2 - '0' < INT_NumPumps))
	{
		int pump = gChar2 - '0';

		mutex.Wait();
		printf("Authorizing pump %d\n", pump);
		mutex.Signal();

		isPumpAuthorized[pump] = true;
	}
	else
	{
		mutex.Wait();
		printf("Invalid command\n");
		mutex.Signal();
	}
}

void Initialize(int threadNums[], CThread *threads[])
{
	for (int i = 0; i < INT_NumPumps; i++)
	{
		isPumpAuthorized[i] = false;
		needPumpAuthorization[i] = false;

		cSemaphores[i] = new CSemaphore("C" + to_string(i), 0, 1);
		pSemaphores[i] = new CSemaphore("P" + to_string(i), 0, 1);
		dps[i] = new CDataPool("Pump" + to_string(i), sizeof(struct GasPumpData));
		gpData[i] = (struct GasPumpData *)(dps[i]->LinkDataPool());

		threadNums[i] = i;
		threads[i] = new  CThread(PumpThread, ACTIVE, &threadNums[i]);
	}

	FuelTankStation::Initialize();
}

int main()
{
	int threadNums[INT_NumPumps];
	CThread *threads[INT_NumPumps];

	Initialize(threadNums, threads);

	CProcess p1("Z:\\CPEN Labs\\Assignments\\Assignments\\Debug\\GasStation.exe",	// pathlist to child program executable
		NORMAL_PRIORITY_CLASS,			// priority
		OWN_WINDOW,						// process has its own window
		ACTIVE							// process is active immediately
		);

	while (true)
	{
		gChar1 = getchar();
		gChar2 = getchar();
		ProcessCommand();
	}

	return 0;
}