#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\rt.h"
#include "FuelTankStation.h"

const int INT_NumPumps = 4;
const int INT_NumFuelTanks = 4;

const int INT_xCustomerInfo = 35;
const int INT_yCustomerInfo = 8;
const int INT_xFuelTankInfo = 70;
const int INT_yFuelTankInfo = 0;

const int INT_ConsoleOutputLine = 16;
const int INT_ConsoleInputLine = 15;
const int INT_LineSizeMax = 20;

FuelTankStation fuelTankStation;
CMutex mutex(string("__Mutex__") + string("GSC"));
CSemaphore *cSemaphores[INT_NumPumps];
CSemaphore *pSemaphores[INT_NumPumps];
CDataPool *dps[INT_NumPumps];
struct GasPumpData *gpData[INT_NumPumps];

PerThreadStorage int threadPumpNumber;

bool isPumpAuthorized[INT_NumPumps];
bool isCustomerDone[INT_NumPumps];
char lineBuffer[INT_LineSizeMax + 1];

//Pump status constants
static const int INT_WaitingCustomerStatus = 6;				// Dark Yellow
static const int INT_WaitingAuthorizationStatus = 12;		// Red
static const int INT_WaitingForFuelTankStationStatus = 14;	// Yellow
static const int INT_DispensingGas = 10;					// Green

struct GasPumpData
{
	long long creditCard;
	int fuelGrade;
	double dispensedVolume;
	int pumpStatus;
	bool isAuthorized;
	bool isDone;
	char customerName[];
};

void LogMessage(char const *message)
{
	mutex.Wait();
	MOVE_CURSOR(0, INT_ConsoleOutputLine); printf("                                   \n"); fflush(stdout);
	MOVE_CURSOR(0, INT_ConsoleOutputLine); printf("GSC: %s\n", message); fflush(stdout);
	mutex.Signal();

	MOVE_CURSOR(0, INT_ConsoleInputLine);
}

string ReadPumpStatus(int status)
{
	switch (status)
	{
	case INT_WaitingCustomerStatus: return "Waiting for Customer";
	case INT_WaitingAuthorizationStatus: return "Waiting for Authorization";
	case INT_WaitingForFuelTankStationStatus: return "Waiting for Fuel";
	case INT_DispensingGas: return "Dispensing Gas";
	default: return "Error";
	}
}

void PrintEmptyPumpDetails(int pumpNumber)
{
	int x = (pumpNumber % 2) * INT_xCustomerInfo;
	int y = (pumpNumber / 2) * INT_yCustomerInfo;

	string name = gpData[pumpNumber]->customerName;
	int pumpStatus = gpData[pumpNumber]->pumpStatus;

	mutex.Wait();
	MOVE_CURSOR(x, y);		printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 1);	printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 2);	printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 3);	printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 4);	printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 5);	printf("                                   "); fflush(stdout);

	MOVE_CURSOR(x, y);		printf("PUMP %d:                           \n", pumpNumber); fflush(stdout);
	MOVE_CURSOR(x, y + 1);	printf("Customer Name:                     \n"); fflush(stdout);
	MOVE_CURSOR(x, y + 2);	printf("Credit Card:                       \n"); fflush(stdout);
	MOVE_CURSOR(x, y + 3);	printf("Dispensed Vol.:                    \n"); fflush(stdout);
	MOVE_CURSOR(x, y + 4);	printf("Fuel Grade:                        \n"); fflush(stdout);

	TEXT_COLOUR(pumpStatus, 0);
	MOVE_CURSOR(x, y + 5);	printf("Status: %s\n", ReadPumpStatus(pumpStatus).c_str()); fflush(stdout);
	TEXT_COLOUR(15, 0);

	MOVE_CURSOR(0, INT_ConsoleInputLine);
	mutex.Signal();
}

void PrintPumpDetails(int pumpNumber)
{
	int x = (pumpNumber % 2) * INT_xCustomerInfo;
	int y = (pumpNumber / 2) * INT_yCustomerInfo;

	string name = gpData[pumpNumber]->customerName;
	int pumpStatus = gpData[pumpNumber]->pumpStatus;

	mutex.Wait();
	MOVE_CURSOR(x, y);		printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 1);	printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 2);	printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 3);	printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 4);	printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y + 5);	printf("                                   "); fflush(stdout);

	MOVE_CURSOR(x, y);		printf("PUMP %d:\n", pumpNumber); fflush(stdout);
	MOVE_CURSOR(x, y + 1);	printf("Customer Name: %s\n", name.c_str()); fflush(stdout);
	MOVE_CURSOR(x, y + 2);	printf("Credit Card: %lld\n", gpData[pumpNumber]->creditCard); fflush(stdout);
	MOVE_CURSOR(x, y + 3);	printf("Dispensed Vol.: %3.1f\n", gpData[threadPumpNumber]->dispensedVolume); fflush(stdout);
	MOVE_CURSOR(x, y + 4);	printf("Fuel Grade: %c\n", (char)('A' + gpData[pumpNumber]->fuelGrade)); fflush(stdout);

	TEXT_COLOUR(pumpStatus, 0);
	MOVE_CURSOR(x, y + 5);	printf("Status: %s\n", ReadPumpStatus(pumpStatus).c_str()); fflush(stdout);
	TEXT_COLOUR(15, 0);

	MOVE_CURSOR(0, INT_ConsoleInputLine);
	mutex.Signal();
}

void PrintFuelTankDetails()
{
	int x = INT_xFuelTankInfo;
	int y = INT_yFuelTankInfo;

	mutex.Wait();
	MOVE_CURSOR(x, y);		printf("                                      ");
	MOVE_CURSOR(x, y + 1);	printf("                                      ");
	MOVE_CURSOR(x, y + 2);	printf("                                      ");
	MOVE_CURSOR(x, y + 3);	printf("                                      ");
	MOVE_CURSOR(x, y + 4);	printf("                                      ");

	MOVE_CURSOR(x, y);		printf("Fuel Tank Station:\n");

	//TODO: Refactor this to INT_NumFuelGrades
	for (int i = 0; i < INT_NumFuelTanks; i++)
	{
		MOVE_CURSOR(x, y + i + 1);
		printf("Octane %d: %5.1fL ", fuelTankStation.GetOctaneGrade(i), fuelTankStation.GetGas(i));
		TEXT_COLOUR(fuelTankStation.GetStatusNumber(i), 0);
		printf("-> %s\n", fuelTankStation.GetStatus(i).c_str());
		TEXT_COLOUR(15, 0);
	}

	fflush(stdout);

	MOVE_CURSOR(0, INT_ConsoleOutputLine);
	mutex.Signal();
}

void AlertGSCForAuthorization(int num)
{
	mutex.Wait();
	printf("Pump %d needs authorization.\n", num);
	fflush(stdout);
	mutex.Signal();
}

void WaitForGSCAuthorization(int num)
{
	//LogMessage(string("Waiting for authorization for pump " + to_string(num)).c_str(), 20);
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
	PrintEmptyPumpDetails(threadPumpNumber);

	while (1)
	{
		pSemaphores[threadPumpNumber]->Wait();
		//AlertGSCForAuthorization(threadPumpNumber);
		PrintPumpDetails(threadPumpNumber);
		WaitForGSCAuthorization(threadPumpNumber);
		cSemaphores[threadPumpNumber]->Signal();

		isCustomerDone[threadPumpNumber] = false;

		while (!isCustomerDone[threadPumpNumber])
		{
			pSemaphores[threadPumpNumber]->Wait();
			PrintPumpDetails(threadPumpNumber);
			if (gpData[threadPumpNumber]->isDone) isCustomerDone[threadPumpNumber] = true;
			cSemaphores[threadPumpNumber]->Signal();
		}

		pSemaphores[threadPumpNumber]->Wait();
		gpData[threadPumpNumber]->isDone = false;
		cSemaphores[threadPumpNumber]->Signal();
		PrintPumpDetails(threadPumpNumber);
	}

	return 0;
}

void ProcessCommand(char ch1, char ch2)
{
	if (ch1 == 'R' && ch2 == 'F')
	{
		//LogMessage("Refilling fuel tanks");
		fuelTankStation.RefillTanks();
	}
	else if (ch1 == 'F')
	{
		int pump = ch2 - '0';

		if (pump >= 0 && pump < INT_NumPumps)
		{
			//LogMessage(string("Authorizing pump" + to_string(pump)).c_str());
			isPumpAuthorized[pump] = true;
		}
		else
		{
			//LogMessage("Invalid command");
		}
	}
	else
	{
		//LogMessage("Invalid command");
	}
}

void ScanKeyboard()
{
	uint16_t fuelTankInfoTimer = 0;
	uint8_t linePointer = 0;
	char inch1;
	char inch2;
	char tmpstr[INT_LineSizeMax];

	while (true)
	{
		while (TEST_FOR_KEYBOARD() == 0)
		{
			if (++fuelTankInfoTimer >= 4000)
			{
				fuelTankInfoTimer = 0;
				PrintFuelTankDetails();
			}
		}

		if (TEST_FOR_KEYBOARD() != 0)
		{
			inch1 = toupper(getchar());
			inch2 = toupper(getchar());

			getchar();

			mutex.Wait();
			MOVE_CURSOR(0, INT_ConsoleInputLine);
			printf("                               ");
			MOVE_CURSOR(0, INT_ConsoleInputLine);
			mutex.Signal();

			ProcessCommand(inch1, inch2);

			//if (incomingChar)
			//{
			//	if (incomingChar == '\n') // End of input
			//	{
			//		lineBuffer[linePointer] = 0;
			//		linePointer = 0;

			//		sprintf_s(tmpstr, INT_LineSizeMax, "%s", lineBuffer);
			//		ProcessCommand(lineBuffer);
			//	}
			//	else if (incomingChar == '\r') // Discard the linefeed
			//	{
			//	}
			//	else // Store any other characters in the buffer
			//	{
			//		lineBuffer[linePointer++] = incomingChar;
			//		lineBuffer[linePointer] = 0;

			//		if (linePointer >= INT_LineSizeMax - 1)
			//		{
			//			linePointer = INT_LineSizeMax - 1;
			//			lineBuffer[linePointer] = 0;
			//		}
			//	}
			//}
		}
	}
}

void Initialize(int threadNums[], CThread *threads[])
{
	for (int i = 0; i < INT_NumPumps; i++)
	{
		isPumpAuthorized[i] = false;
		isCustomerDone[i] = false;

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

	CProcess p1("Z:\\CPEN Labs\\Assignments\\Assignments\\Debug\\GasStation.exe",	// pathlist to child program executable
		NORMAL_PRIORITY_CLASS,			// priority
		OWN_WINDOW,						// process has its own window
		ACTIVE							// process is active immediately
		);

	Initialize(threadNums, threads);

	ScanKeyboard();

	p1.WaitForProcess();

	return 0;
}