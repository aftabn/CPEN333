#include "Pump.h"
#include <ctime>
#include "../GasStationComputer/FuelTankStation.h"

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

void Pump::LogMessage(char const *message, int line) const
{
	gasStationMutex->Wait();
	MOVE_CURSOR(0, line);
	printf("                                                                  \n");
	MOVE_CURSOR(0, line);
	printf("PUMP: %s\n", message);
	fflush(stdout);
	gasStationMutex->Signal();
}

string Pump::ReadStatus(int status) const
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

Pump::Pump(int num, FuelTankStation *fuelTankStation)
{
	this->fuelTankStation = fuelTankStation;
	pumpNumber = num;

	Initialize();
}

Pump::~Pump()
{
	delete gasStationMutex;
	delete pumpMutex;
	delete ps;
	delete cs;
	delete pipe;
	delete dp;
	delete data;
}

void Pump::Initialize()
{
	gasStationMutex = new CMutex(string("__Mutex__") + string("GasStation"));
	pumpMutex = new CMutex(string("__Mutex__") + string("Pump") + to_string(pumpNumber));

	ps = new CSemaphore(string("PS") + to_string(pumpNumber), 0, 1);
	cs = new CSemaphore(string("CS") + to_string(pumpNumber), 1, 1);

	pipe = new CTypedPipe<CustomerData>(string("Pipe") + to_string(pumpNumber), 1024);
	dp = new CDataPool(string("Pump") + to_string(pumpNumber), sizeof(struct GasPumpData));
	data = (struct GasPumpData *)(dp->LinkDataPool());

	data->pumpStatus = INT_WaitingCustomerStatus;
	transactionCost = 0;

	PrintEmptyDetails();
}

void Pump::PrintCustomerDetails(CustomerData &cData) const
{
	int x = (pumpNumber % 2) * INT_xCustomerInfo;
	int y = (pumpNumber / 2) * INT_yCustomerInfo;

	int grade = fuelTankStation->GetOctaneGrade(cData.fuelGrade);

	gasStationMutex->Wait();
	MOVE_CURSOR(x, y);		printf("                                 ");
	MOVE_CURSOR(x, y + 1);	printf("                                 ");
	MOVE_CURSOR(x, y + 2);	printf("                                 ");
	MOVE_CURSOR(x, y + 3);	printf("                                 ");
	MOVE_CURSOR(x, y + 4);	printf("                                 ");
	MOVE_CURSOR(x, y + 5);	printf("                                 ");
	MOVE_CURSOR(x, y + 6);	printf("                                 ");
	MOVE_CURSOR(x, y + 7);	printf("                                 ");

	MOVE_CURSOR(x, y);		printf("PUMP %d:\n", pumpNumber);
	MOVE_CURSOR(x, y + 1);	printf("Customer Name: %s\n", cData.customerName.c_str());
	MOVE_CURSOR(x, y + 2);	printf("Credit Card: %lld\n", cData.creditCard);
	MOVE_CURSOR(x, y + 3);	printf("Requested Vol.: %3.1f\n", cData.requestedVolume);
	MOVE_CURSOR(x, y + 4);	printf("Dispensed Vol.: %3.1f\n", data->dispensedVolume);
	MOVE_CURSOR(x, y + 5);	printf("Fuel Grade: Octane %d\n", grade);
	MOVE_CURSOR(x, y + 6);	printf("Cost: $%.2f\n", transactionCost * data->dispensedVolume);

	TEXT_COLOUR(data->pumpStatus, 0);
	MOVE_CURSOR(x, y + 7);	printf("Status: %s\n", ReadStatus(data->pumpStatus).c_str());
	TEXT_COLOUR(15, 0);

	fflush(stdout);
	gasStationMutex->Signal();
}

void Pump::PrintEmptyDetails() const
{
	int x = (pumpNumber % 2) * INT_xCustomerInfo;
	int y = (pumpNumber / 2) * INT_yCustomerInfo;

	gasStationMutex->Wait();
	MOVE_CURSOR(x, y);		printf("                                 "); fflush(stdout);
	MOVE_CURSOR(x, y + 1);	printf("                                 "); fflush(stdout);
	MOVE_CURSOR(x, y + 2);	printf("                                 "); fflush(stdout);
	MOVE_CURSOR(x, y + 3);	printf("                                 "); fflush(stdout);
	MOVE_CURSOR(x, y + 4);	printf("                                 "); fflush(stdout);
	MOVE_CURSOR(x, y + 5);	printf("                                 "); fflush(stdout);
	MOVE_CURSOR(x, y + 6);	printf("                                 "); fflush(stdout);
	MOVE_CURSOR(x, y + 7);	printf("                                 "); fflush(stdout);

	MOVE_CURSOR(x, y);		printf("PUMP %d:                         \n", pumpNumber); fflush(stdout);
	MOVE_CURSOR(x, y + 1);	printf("Customer Name:                   \n"); fflush(stdout);
	MOVE_CURSOR(x, y + 2);	printf("Credit Card:                     \n"); fflush(stdout);
	MOVE_CURSOR(x, y + 3);	printf("Requested Vol.:                  \n"); fflush(stdout);
	MOVE_CURSOR(x, y + 4);	printf("Dispensed Vol.:                  \n"); fflush(stdout);
	MOVE_CURSOR(x, y + 5);	printf("Fuel Grade:                      \n"); fflush(stdout);
	MOVE_CURSOR(x, y + 6);	printf("Cost:                            \n"); fflush(stdout);

	TEXT_COLOUR(data->pumpStatus, 0);
	MOVE_CURSOR(x, y + 7);	printf("Status: %s\n", ReadStatus(data->pumpStatus).c_str());
	TEXT_COLOUR(15, 0);

	fflush(stdout);
	gasStationMutex->Signal();
}

void Pump::StartTransaction(CustomerData &cData)
{
	// Wait until allowed to send details
	cs->Wait();
	data->creditCard = cData.creditCard;
	strcpy_s(data->customerName, 30, cData.customerName.c_str());
	data->fuelGrade = cData.fuelGrade;
	data->dispensedVolume = 0;
	data->totalCost = 0;
	data->isAuthorized = false;
	data->isDone = false;
	transactionCost = fuelTankStation->GetGasCost(cData.fuelGrade);
	ps->Signal();
}

void Pump::WaitForAuthorizationFromGSC(CustomerData &cData) const
{
	//LogMessage(string("Waiting for authorization").c_str());
	PrintCustomerDetails(cData);
	cs->Wait();

	// Reset this for the next customer
	if (data->isAuthorized) data->isAuthorized = false;
}

void Pump::WaitUntilGasStationReady() const
{
	//LogMessage(string("Waiting for gas to be ready").c_str());

	while (fuelTankStation->GetGas(data->fuelGrade) < 200)
	{
		SLEEP(200);
	}
}

void Pump::DispenseFuelUntilComplete(CustomerData &cData) const
{
	//LogMessage(string("Starting dispensing").c_str());

	while (fuelTankStation->GetGas(data->fuelGrade) > 0 && data->dispensedVolume < cData.requestedVolume)
	{
		data->dispensedVolume += fuelTankStation->WithdrawGas(DBL_GasFlowRate, data->fuelGrade);
		PrintCustomerDetails(cData);

		if (data->dispensedVolume >= cData.requestedVolume || (int)fuelTankStation->GetGas(data->fuelGrade) <= 0)
		{
			data->isDone = true;
		}

		ps->Signal();
		cs->Wait();

		SLEEP(1000);
	}

	//LogMessage(string("Finished Dispensing").c_str());

	ps->Signal();
}

int Pump::main()
{
	while (1)
	{
		data->pumpStatus = INT_WaitingCustomerStatus;
		PrintEmptyDetails();

		struct CustomerData cData;
		pipe->Read(&cData);
		pumpMutex->Wait();

		data->pumpStatus = INT_WaitingAuthorizationStatus;
		PrintCustomerDetails(cData);
		StartTransaction(cData);
		WaitForAuthorizationFromGSC(cData);

		data->pumpStatus = INT_WaitingForFuelTankStationStatus;
		PrintCustomerDetails(cData);
		WaitUntilGasStationReady();

		data->pumpStatus = INT_DispensingGas;
		DispenseFuelUntilComplete(cData);

		pumpMutex->Signal();
	}
	return 0;
}