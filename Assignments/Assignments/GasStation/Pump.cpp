#include "Pump.h"
#include <ctime>
#include "../GasStationComputer/FuelTankStation.h"

struct GasPumpData
{
	long long creditCard;
	int fuelGrade;
	double dispensedVolume;
	bool isAuthorized;
	bool isDone;
	string customerName;
};

const double Pump::DBL_GasCost[] = { 0.98, 1.02, 1.10, 1.25 };

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
	case INT_WaitingForFuelTankStationStatus: return "Waiting for Fuel Tank Station";
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

	pumpStatus = INT_WaitingCustomerStatus;
	PrintEmptyDetails();
}

void Pump::PrintCustomerDetails(CustomerData &cData) const
{
	gasStationMutex->Wait();
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth);
	printf("PUMP %d:\n", pumpNumber);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 1);
	printf("                                       ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 1);
	printf("Customer Name: %s\n", cData.customerName.c_str());

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 2);
	printf("                                       ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 2);
	printf("Credit Card: %lld\n", cData.creditCard);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 3);
	printf("                                       ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 3);
	printf("Requested Vol.: %3.1f\n", cData.requestedVolume);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 4);
	printf("                                       ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 4);
	printf("Dispensed Vol.: %3.1f\n", data->dispensedVolume);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 5);
	printf("                                       ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 5);
	printf("Fuel Grade: %c\n", (char)('A' + cData.fuelGrade));

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 6);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 6);
	printf("Cost: $%.2f\n", (DBL_GasCost[cData.fuelGrade] * data->dispensedVolume));

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 7);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 7);
	printf("Status: %s\n", ReadStatus(pumpStatus).c_str());

	fflush(stdout);
	gasStationMutex->Signal();
}

void Pump::PrintEmptyDetails() const
{
	gasStationMutex->Wait();
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth);
	printf("PUMP %d:                         \n", pumpNumber);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 1);
	printf("Customer Name:                   \n");

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 2);
	printf("Credit Card:                     \n");

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 3);
	printf("Requested Vol.:                   \n");

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 4);
	printf("Dispensed Vol.:                   \n");

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 5);
	printf("Fuel Grade:                       \n");

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 6);
	printf("Cost:                             \n");

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 7);
	printf("                                    ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 7);
	printf("Status: %s\n", ReadStatus(pumpStatus).c_str());

	fflush(stdout);
	gasStationMutex->Signal();
}

void Pump::StartTransaction(CustomerData &cData) const
{
	// Wait until allowed to send details
	cs->Wait();
	data->creditCard = cData.creditCard;
	data->customerName = cData.customerName;
	data->fuelGrade = cData.fuelGrade;
	data->dispensedVolume = 0;
	data->isAuthorized = false;
	data->isDone = false;
	ps->Signal();
}

void Pump::WaitForAuthorizationFromGSC(CustomerData &cData) const
{
	LogMessage(string("Waiting for authorization").c_str());
	PrintCustomerDetails(cData);
	cs->Wait();

	// Reset this for the next customer
	if (data->isAuthorized) data->isAuthorized = false;
}

void Pump::WaitUntilGasStationReady() const
{
	LogMessage(string("Waiting for gas to be ready").c_str());

	while (fuelTankStation->GetGas(data->fuelGrade) < 200)
	{
		SLEEP(200);
	}
}

void Pump::DispenseFuelUntilComplete(CustomerData &cData) const
{
	LogMessage(string("Starting dispensing").c_str());

	while (fuelTankStation->GetGas(data->fuelGrade) > 0 && data->dispensedVolume < cData.requestedVolume)
	{
		if (!fuelTankStation->WithdrawGas(DBL_GasFlowRate, data->fuelGrade)) break;
		// If above condition happened, maybe show some flashy colors or some shit

		PrintCustomerDetails(cData);
		data->dispensedVolume += DBL_GasFlowRate;

		LogMessage(string("Before sem").c_str(), 21);

		ps->Signal();
		cs->Wait();

		LogMessage(string("After sem").c_str(), 21);

		SLEEP(1000);
	}

	LogMessage(string("Finished Dispensing").c_str());

	data->isDone = true;
	ps->Signal();
}

int Pump::main()
{
	while (1)
	{
		pumpStatus = INT_WaitingCustomerStatus;
		PrintEmptyDetails();

		struct CustomerData cData;
		pipe->Read(&cData);
		pumpMutex->Wait();

		LogMessage(string("Waiting for authorization").c_str(), 21);

		pumpStatus = INT_WaitingAuthorizationStatus;
		PrintCustomerDetails(cData);
		StartTransaction(cData);
		WaitForAuthorizationFromGSC(cData);

		pumpStatus = INT_WaitingForFuelTankStationStatus;
		PrintCustomerDetails(cData);
		WaitUntilGasStationReady();

		pumpStatus = INT_DispensingGas;
		DispenseFuelUntilComplete(cData);

		LogMessage(string("Releasing mutex for next customer").c_str());
		pumpMutex->Signal();
	}
	return 0;
}