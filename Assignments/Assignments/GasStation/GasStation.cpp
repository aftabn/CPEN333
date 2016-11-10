#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\rt.h"
#include "Pump.h"
#include "Customer.h"
#include <queue>
#include "../GasStationComputer/FuelTankStation.h"

const int INT_NumPumps = 8;

FuelTankStation *fuelTankStation;
CMutex mutex(string("__Mutex__") + string("GasStation"));
Pump *pumps[INT_NumPumps];
Customer *activeCustomers[INT_NumPumps];
queue<Customer*> pumpQueue;

void LogMessage(char const *message)
{
	mutex.Wait();
	MOVE_CURSOR(0, 2);
	printf("                                                                  \n");
	MOVE_CURSOR(0, 2);
	printf("GS: %s\n", message);
	fflush(stdout);
	mutex.Signal();
}

void Initialize()
{
	CURSOR_OFF();

	FuelTankStation *fuelTankStation = new FuelTankStation();

	for (int i = 0; i < INT_NumPumps; i++)
	{
		pumps[i] = new Pump(i, fuelTankStation);
		pumps[i]->Resume();

		activeCustomers[i] = nullptr;
	}
}

void CreateNewCustomer()
{
	Customer *customer = new Customer();
	pumpQueue.push(customer);

	string message = "Queue Size: " + to_string(pumpQueue.size());

	LogMessage(message.c_str());
}

void RemoveFinishedCustomers()
{
	for (int i = 0; i < INT_NumPumps; i++)
	{
		Customer *customer = activeCustomers[i];
		if (customer == nullptr) continue;

		if (customer->IsDoneTransaction)
		{
			delete customer;
			activeCustomers[i] = nullptr;
		}
	}
}

void AssignNewCustomers()
{
	if (pumpQueue.size() == 0) return;

	for (int i = 0; i < INT_NumPumps; i++)
	{
		if (activeCustomers[i] == nullptr && pumps[i]->IsEnabled())
		{
			Customer *customer = pumpQueue.front();

			pumpQueue.pop();
			activeCustomers[i] = customer;
			customer->AssignToPump(i);
			customer->Resume();

			string message = "Queue Size: " + to_string(pumpQueue.size()) + " ";

			LogMessage(message.c_str());

			if (pumpQueue.size() == 0) return;
		}
	}
}

int main()
{
	Initialize();

	srand(time(nullptr));
	int customerWaitTime, count;

	while (1)
	{
		CreateNewCustomer();

		// Calculate random time until next customer
		customerWaitTime = rand() % 6 + 5;
		count = 0;

		while (count < customerWaitTime)
		{
			RemoveFinishedCustomers();
			AssignNewCustomers();

			SLEEP(1000);
			count++;
		}
	}

	return 0;
}