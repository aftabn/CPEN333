#include "Pump.h"
#include <ctime>
#include "../GasStationComputer/FuelTankStation.h"

struct CustomerData
{
	long long creditCard;
	int fuelGrade;
	double requestedVolume;
	string customerName;
};

struct GasPumpData
{
	long long creditCard;
	int fuelGrade;
	double dispensedVolume;
	bool isAuthorized;
	bool isDone;
	string customerName;
};

void LogMessage(CMutex &gasStationMutex, char const *message)
{
	gasStationMutex.Wait();
	MOVE_CURSOR(0, 18);
	printf("                                                                  ");
	MOVE_CURSOR(0, 18);
	printf("PUMP: %s\n", message);
	gasStationMutex.Signal();
}

const double Pump::DBL_GasCost[] = { 0.98, 1.02, 1.10, 1.25 };

Pump::Pump(int num)
{
	pumpNumber = num;
}

Pump::~Pump()
{
}

void Pump::PrintCustomerDetails(CMutex &gasStationMutex, struct CustomerData &data,
	double dispensedVolume) const
{
	gasStationMutex.Wait();
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth);
	printf("PUMP %d:\n", pumpNumber);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 1);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 1);
	printf("Customer Name: %s\n", data.customerName.c_str());

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 2);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 2);
	printf("Credit Card: %lld\n", data.creditCard);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 3);
	printf("Requested Vol.: %3.1f\n", data.requestedVolume);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 4);
	printf("Dispensed Vol.: %3.1f\n", dispensedVolume);

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 5);
	printf("Fuel Grade: %c\n", (char)('A' + data.fuelGrade));

	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 6);
	printf("                                 ");
	MOVE_CURSOR((pumpNumber % 2) * INT_xCustomerInfoWidth, (pumpNumber / 2) * INT_yCustomerInfoWidth + 6);
	printf("Cost: $ %.2f\n", (DBL_GasCost[data.fuelGrade] * dispensedVolume));

	fflush(stdout);
	gasStationMutex.Signal();
}

int Pump::main()
{
	// Create mutexes and pipeline
	CMutex gasStationMutex(string("__Mutex__") + string("GasStation"));
	CMutex pumpMutex(string("__Mutex__") + string("Pump") + to_string(pumpNumber));
	CTypedPipe<CustomerData> pipe(string("Pipe") + to_string(pumpNumber), 1024);

	CSemaphore ps(string("PS") + to_string(pumpNumber), 0, 1);
	CSemaphore cs(string("CS") + to_string(pumpNumber), 0, 1);

	// Connect to datapool
	CDataPool dp(string("DataPool") + to_string(pumpNumber), sizeof(struct GasPumpData));
	struct GasPumpData *data = (struct GasPumpData *)(dp.LinkDataPool());

	double dispensedVolume;

	while (1)
	{
		dispensedVolume = 0;

		struct CustomerData temp;
		pipe.Read(&temp);
		pumpMutex.Wait();

		PrintCustomerDetails(gasStationMutex, temp, dispensedVolume);

		// Wait until allowed to send details
		cs.Wait();
		data->creditCard = temp.creditCard;
		data->customerName = temp.customerName;
		data->fuelGrade = temp.fuelGrade;
		data->dispensedVolume = 0;
		data->isAuthorized = false;
		data->isDone = false;
		ps.Signal();

		LogMessage(gasStationMutex, string("Waiting for authorization").c_str());

		// Wait for authorization
		cs.Wait();

		LogMessage(gasStationMutex, string("Waiting for fuel tank station to be ready").c_str());

		// Reset this for the next customer
		if (data->isAuthorized) data->isAuthorized = false;

		// while gas station tank grade less than 200, wait
		while (FuelTankStation::GetGas(data->fuelGrade) < 200) SLEEP(200);

		LogMessage(gasStationMutex, string("Starting dispensing").c_str());

		while (FuelTankStation::GetGas(data->fuelGrade) > 0 && dispensedVolume < temp.requestedVolume)
		{
			if (!FuelTankStation::WithdrawGas(DBL_GasFlowRate, data->fuelGrade)) break;
			// If above condition happened, maybe show some flashy colors or some shit

			dispensedVolume += DBL_GasFlowRate;
			PrintCustomerDetails(gasStationMutex, temp, dispensedVolume);

			data->dispensedVolume = dispensedVolume;
			ps.Signal();
			cs.Wait();

			SLEEP(1000);
		}

		data->isDone = true;
		ps.Signal();

		pumpMutex.Signal();

		// Wait for fuel tank station to be ready
		// Once ready,

		// Signal start of transaction
		/*cs1.Signal();
		ps1.Wait();*/

		// Start dispensing and updating the pool
		/*int dispensed = 0;
		int rate = data->requestedVolume / 10;
		while (dispensed < gpData->requestedVolume)
		{
			dispensed += rate;
			gpData->dispensedVolume = dispensed;
			SLEEP(500);
		}*/

		//cs1.Signal();
	}

	// End transaction and "leave"
	return 0;
}