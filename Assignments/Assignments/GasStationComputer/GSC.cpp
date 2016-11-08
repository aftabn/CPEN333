#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\rt.h"
#include "FuelTankStation.h"

const int INT_NumPumps = 1;
const int INT_xCustomerInfoWidth = 35;
const int INT_yCustomerInfoWidth = 8;

bool isPumpAuthorized[INT_NumPumps];
bool needPumpAuthorization[INT_NumPumps];

FuelTankStation fuelTankStation;
CMutex mutex(string("__Mutex__") + string("GSC"));
CSemaphore *cSemaphores[INT_NumPumps];
CSemaphore *pSemaphores[INT_NumPumps];
CDataPool *dps[INT_NumPumps];
struct GasPumpData *gpData[INT_NumPumps];

PerThreadStorage int threadPumpNumber;
PerThreadStorage bool isCustomerDone;

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

void LogMessage(char const *message, int line)
{
	mutex.Wait();
	MOVE_CURSOR(0, line);
	printf("                                                                  \n");
	MOVE_CURSOR(0, line);
	printf("GSC: %s\n", message);
	fflush(stdout);
	mutex.Signal();
}

void PrintPumpDetails(double dispensedVolume, int pumpNumber)
{
	mutex.Wait();
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth);
	printf("PUMP %d:\n", pumpNumber);

	string name = gpData[pumpNumber]->customerName;
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 1);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 1);
	printf("Customer Name: %s\n", name.c_str());

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 2);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 2);
	printf("Credit Card: %lld\n", gpData[pumpNumber]->creditCard);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 3);
	printf("Dispensed Vol.: %3.1f\n", dispensedVolume);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 4);
	printf("Fuel Grade: %c\n", (char)('A' + gpData[pumpNumber]->fuelGrade));

	fflush(stdout);
	mutex.Signal();
}

void AlertGSCForAuthorization(int num)
{
	mutex.Wait();
	needPumpAuthorization[num] = true;
	printf("Pump %d needs authorization.\n", num);
	fflush(stdout);
	mutex.Signal();
}

void WaitForGSCAuthorization(int num)
{
	LogMessage(string("Waiting for authorization for pump " + to_string(num)).c_str(), 21);
	while (!isPumpAuthorized[num])
	{
		SLEEP(100);
	}

	isPumpAuthorized[num] = false; // Reset it
	gpData[num]->isAuthorized = true;
}

UINT __stdcall PumpThread(void *args)	// thread function
{
	threadPumpNumber = *(int *)(args);
	isCustomerDone = false;

	while (1)
	{
		pSemaphores[threadPumpNumber]->Wait();
		AlertGSCForAuthorization(threadPumpNumber);
		WaitForGSCAuthorization(threadPumpNumber);
		cSemaphores[threadPumpNumber]->Signal();

		isCustomerDone = false;
		while (!isCustomerDone)
		{
			LogMessage(string("Waiting for ps at " + to_string(threadPumpNumber)).c_str(), 21);
			pSemaphores[threadPumpNumber]->Wait();

			PrintPumpDetails(gpData[threadPumpNumber]->dispensedVolume, threadPumpNumber);
			if (gpData[threadPumpNumber]->isDone)
			{
				gpData[threadPumpNumber]->isDone = false;
				isCustomerDone = true;
			}

			LogMessage(string("Waiting for cs at " + to_string(threadPumpNumber)).c_str(), 21);
			cSemaphores[threadPumpNumber]->Signal();
		}

		LogMessage(string("Broken out of loop " + to_string(threadPumpNumber)).c_str(), 21);
	}

	return 0;
}

void ProcessCommand()
{
	if (gChar1 == 'R' && gChar2 == 'F')
	{
		mutex.Wait();
		printf("Refilling fuel tanks\n");
		mutex.Signal();

		fuelTankStation.RefillTanks();
	}
	else if (gChar1 == 'F' && (gChar2 - '0' >= 0 && gChar2 - '0' < INT_NumPumps))
	{
		int pump = gChar2 - '0';

		mutex.Wait();
		printf("Authorizing pump %d\n", pump);
		mutex.Signal();

		needPumpAuthorization[pump] = false;
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

		pSemaphores[i] = new CSemaphore(string("PS") + to_string(i), 0, 1);
		cSemaphores[i] = new CSemaphore(string("CS") + to_string(i), 1, 1);
		dps[i] = new CDataPool(string("Pump") + to_string(i), sizeof(struct GasPumpData));
		gpData[i] = (struct GasPumpData *)(dps[i]->LinkDataPool());

		threadNums[i] = i;
		threads[i] = new  CThread(PumpThread, ACTIVE, &threadNums[i]);
	}

	fuelTankStation.Initialize();
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

	p1.WaitForProcess();

	return 0;
}