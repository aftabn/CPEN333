#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\rt.h"
#include "FuelTankStation.h"
#include "Transaction.h"
#include <vector>

const int INT_NumPumps = 4;
const int INT_NumFuelTanks = 4;

const int INT_xCustomerInfo = 35;
const int INT_yCustomerInfo = 8;
const int INT_xFuelTankInfo = 70;
const int INT_yFuelTankInfo = 0;
const int INT_xTransactionInfo = 70;
const int INT_yTransactionInfo = 8;

const int INT_ConsoleOutputLine = 15;
const int INT_ConsoleInputLine = 17;

FuelTankStation fuelTankStation;
CMutex mutex(string("__Mutex__") + string("GSC"));
CSemaphore *cSemaphores[INT_NumPumps];
CSemaphore *pSemaphores[INT_NumPumps];
CDataPool *dps[INT_NumPumps];
struct GasPumpData *gpData[INT_NumPumps];

PerThreadStorage int threadPumpNumber;

bool isPumpAuthorized[INT_NumPumps];
bool isCustomerDone[INT_NumPumps];

// Command line buffers and constants
const int INT_LineSizeMax = 20;
const int INT_ParameterCountMax = 3;
const int INT_ParameterLengthMax = 10;
char gParameters[INT_ParameterCountMax][INT_ParameterLengthMax + 1];
char lineBuffer[INT_LineSizeMax + 1];

vector<Transaction*> transactions;

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
	double totalCost;
	int pumpStatus;
	bool isAuthorized;
	bool isDone;
	bool isEnabled;
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

	int grade = fuelTankStation.GetOctaneGrade(gpData[pumpNumber]->fuelGrade);

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
	MOVE_CURSOR(x, y + 4);	printf("Fuel Grade: Octane %d\n", grade); fflush(stdout);

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
		printf("Octane %d [$%.2f]: %.1fL ", fuelTankStation.GetOctaneGrade(i),
			fuelTankStation.GetGasCost(i), fuelTankStation.GetGas(i));
		TEXT_COLOUR(fuelTankStation.GetStatusNumber(i), 0);
		printf("-> %s\n", fuelTankStation.GetStatus(i).c_str());
		TEXT_COLOUR(15, 0);
	}

	fflush(stdout);

	MOVE_CURSOR(0, INT_ConsoleOutputLine);
	mutex.Signal();
}

void WaitForGSCAuthorization(int num)
{
	while (!isPumpAuthorized[num])
	{
		SLEEP(100);
	}

	isPumpAuthorized[num] = false; // Reset it
	gpData[num]->isAuthorized = true;
}

void PrintTransactions()
{
	const int INT_TransactionBlockHeight = 7;
	int x = INT_xTransactionInfo;
	int y = INT_yTransactionInfo;

	MOVE_CURSOR(x, y - 1);		printf("                                   "); fflush(stdout);
	MOVE_CURSOR(x, y - 1);		printf("TRANSACTIONS:                           \n"); fflush(stdout);

	mutex.Wait();
	for (int i = 0; i < transactions.size(); i++)
	{
		MOVE_CURSOR(x, y + 1 + i*INT_TransactionBlockHeight);	printf("                                   "); fflush(stdout);
		MOVE_CURSOR(x, y + 2 + i*INT_TransactionBlockHeight);	printf("                                   "); fflush(stdout);
		MOVE_CURSOR(x, y + 3 + i*INT_TransactionBlockHeight);	printf("                                   "); fflush(stdout);
		MOVE_CURSOR(x, y + 4 + i*INT_TransactionBlockHeight);	printf("                                   "); fflush(stdout);
		MOVE_CURSOR(x, y + 5 + i*INT_TransactionBlockHeight);	printf("                                   "); fflush(stdout);
		MOVE_CURSOR(x, y + 6 + i*INT_TransactionBlockHeight);	printf("                                   "); fflush(stdout);

		MOVE_CURSOR(x, y + 1 + i*INT_TransactionBlockHeight);	printf("Transaction #%d:\n", i + 1); fflush(stdout);
		MOVE_CURSOR(x, y + 2 + i*INT_TransactionBlockHeight);	printf("Purchase Time: %s\n", transactions[i]->PurchaseTime); fflush(stdout);
		MOVE_CURSOR(x, y + 3 + i*INT_TransactionBlockHeight);	printf("Customer Name: %s\n", transactions[i]->CustomerName); fflush(stdout);
		MOVE_CURSOR(x, y + 4 + i*INT_TransactionBlockHeight);	printf("Credit Card: %lld\n", transactions[i]->CreditCard); fflush(stdout);
		MOVE_CURSOR(x, y + 5 + i*INT_TransactionBlockHeight);	printf("Fuel Grade: Octane %d\n", transactions[i]->FuelGrade); fflush(stdout);
		MOVE_CURSOR(x, y + 6 + i*INT_TransactionBlockHeight);	printf("Dispensed Vol.: %4.1f\n", transactions[i]->DispensedFuel); fflush(stdout);
	}

	MOVE_CURSOR(0, INT_ConsoleInputLine);
	mutex.Signal();
}

void CreateTransaction(int num)
{
	time_t t = time(0);
	tm *localTm = localtime(&t);

	Transaction *transaction = new Transaction(
		asctime(localTm),
		gpData[num]->customerName,
		gpData[num]->creditCard,
		fuelTankStation.GetOctaneGrade(gpData[num]->fuelGrade),
		gpData[num]->dispensedVolume
		);

	transactions.push_back(transaction);
}

UINT __stdcall PumpThread(void *args)	// thread function
{
	threadPumpNumber = *(int *)(args);

	while (1)
	{
		PrintEmptyPumpDetails(threadPumpNumber);
		pSemaphores[threadPumpNumber]->Wait();
		PrintPumpDetails(threadPumpNumber);
		WaitForGSCAuthorization(threadPumpNumber);
		cSemaphores[threadPumpNumber]->Signal();

		isCustomerDone[threadPumpNumber] = false;

		while (pSemaphores[threadPumpNumber]->Read() == 0)
		{
			PrintPumpDetails(threadPumpNumber);
			SLEEP(100);
		}

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
		CreateTransaction(threadPumpNumber);
	}

	return 0;
}

bool isFuelTankNumberCorrect(char *tankArg)
{
	char str[10];

	for (int tank = 0; tank < INT_NumFuelTanks; tank++)
	{
		sprintf(str, "%d", tank);
		if (0 == _stricmp(tankArg, str))
		{
			return true;
		}
	}

	return false;
}

bool isPumpNumberCorrect(char *pumpArg)
{
	char str[10];

	for (int pump = 0; pump < INT_NumPumps; pump++)
	{
		sprintf(str, "%d", pump);
		if (0 == _stricmp(pumpArg, str))
		{
			return true;
		}
	}

	return false;
}

void processCommand(char *command)
{
	// Commands:
	// RF - Refill
	// A # - Authorize
	// C # ##.## - Change cost of gas
	// E # - Enable pump
	// D # - Disable pump

	if (0 == _stricmp(command, "RF"))
	{
		fuelTankStation.RefillTanks();
	}
	else if (0 == _stricmp(command, "C"))
	{
		if (isPumpNumberCorrect(gParameters[0]))
		{
			int pump = atoi(gParameters[0]);
			double cost = atof(gParameters[1]);

			if (cost > 0 && cost < 20)
			{
				fuelTankStation.setGasCost(pump, cost);
			}
			else
			{
				LogMessage("Gas cost needs to be between $0-20 per liter");
			}
		}

		LogMessage("Invalid pump number");
	}
	else if (0 == _stricmp(command, "A"))
	{
		if (isPumpNumberCorrect(gParameters[0]))
		{
			int pump = atoi(gParameters[0]);
			isPumpAuthorized[pump] = true;
		}

		LogMessage("Invalid pump number");
	}
	else if (0 == _stricmp(command, "E"))
	{
		if (isPumpNumberCorrect(gParameters[0]))
		{
			int pump = atoi(gParameters[0]);

			// Enable pump here
		}

		LogMessage("Invalid pump number");
	}
	else if (0 == _stricmp(command, "D"))
	{
		if (isPumpNumberCorrect(gParameters[0]))
		{
			int pump = atoi(gParameters[0]);

			// Disable pump
		}

		LogMessage("Invalid pump number");
	}
	else if (0 == _stricmp(command, "TRANS"))
	{
		PrintTransactions();
	}

	//if (ch1 == 'R' && ch2 == 'F')
	//{
	//	fuelTankStation.RefillTanks();
	//}
	//else if (ch1 == 'F')
	//{
	//	int pump = ch2 - '0';
	//	if (pump >= 0 && pump < INT_NumPumps)
	//	{
	//		isPumpAuthorized[pump] = true;
	//	}
	//}
	//else if (ch1 == 'E')
	//{
	//	int pump = ch2 - '0';
	//	if (pump >= 0 && pump < INT_NumPumps)
	//	{
	//		// Enable pump
	//	}
	//}
	//else if (ch1 == 'D')
	//{
	//	int pump = ch2 - '0';
	//	if (pump >= 0 && pump < INT_NumPumps)
	//	{
	//		// Disable pump
	//	}
	//}
	//else if (ch1 == 'P' && ch2 == 'T')
	//{
	//	PrintTransactions();
	//}
}

void clearParameters()
{
	for (int i = 0; i < INT_ParameterCountMax; i++)
	{
		strcpy(gParameters[i], "");
	}
}

void processParameters(char *parameterString)
{
	clearParameters();

	if (NULL != parameterString && strlen(parameterString))
	{
		uint8_t i = 0;
		const char delimiter[] = " ";
		char *token;

		// break up remaining strings into parameters
		token = strtok(parameterString, delimiter);

		while (NULL != token)
		{
			// copy parameters into the global parameter variables
			strcpy(gParameters[i++], token);
			token = strtok(NULL, delimiter);
		}
	}
}

void processLine(char *line)
{
	char buffer[INT_LineSizeMax + 1];
	const char delimiter[] = " ";
	char *commandString = NULL;
	char *remainingString = NULL;

	strcpy(buffer, line);

	// get first token
	commandString = strtok(buffer, delimiter);

	if (NULL != commandString)
	{
		if (strlen(line) - strlen(commandString) > 0)
		{
			remainingString = line + strlen(commandString) + 1;
		}

		processParameters(remainingString);
		processCommand(commandString);
	}
}

void ScanKeyboard()
{
	uint16_t fuelTankInfoTimer = 0;
	uint8_t linePointer = 0;
	char incomingChar;
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
			incomingChar = toupper(_getch());

			mutex.Wait();
			MOVE_CURSOR(0, INT_ConsoleInputLine);
			printf("                               ");
			MOVE_CURSOR(0, INT_ConsoleInputLine);
			mutex.Signal();

			//ProcessCommand(inch1, inch2);

			if (incomingChar)
			{
				if (incomingChar == '\n') // End of input
				{
				}
				else if (incomingChar == '\r') // Discard the linefeed
				{
					lineBuffer[linePointer] = 0;
					linePointer = 0;
					sprintf_s(tmpstr, INT_LineSizeMax, "%s", lineBuffer);
					processLine(lineBuffer);
				}
				else // Store any other characters in the buffer
				{
					lineBuffer[linePointer++] = incomingChar;
					lineBuffer[linePointer] = 0;
					if (linePointer >= INT_LineSizeMax - 1)
					{
						linePointer = INT_LineSizeMax - 1;
						lineBuffer[linePointer] = 0;
					}
				}
			}
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