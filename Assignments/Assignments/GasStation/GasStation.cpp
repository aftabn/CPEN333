#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\rt.h"
#include "Pump.h"
#include "Customer.h"
#include <queue>

CMutex mutex(string("__Mutex__") + string("GasStation"));
const int INT_NumPumps = 1;

void LogMessage(char const *message)
{
	mutex.Wait();
	MOVE_CURSOR(0, 16);
	printf("                                                                  ");
	MOVE_CURSOR(0, 16);
	printf("GS: %s\n", message);
	mutex.Signal();
}

void InitializePumps(Pump *pumps[], Customer *activeCustomers[])
{
	for (int i = 0; i < INT_NumPumps; i++)
	{
		pumps[i] = new Pump(i);
		pumps[i]->Resume();

		activeCustomers[i] = nullptr;
	}
	string message = "GS: Created " + INT_NumPumps + string(" pumps");
	LogMessage(message.c_str());
}

void CreateNewCustomer(queue<Customer*> &pumpQueue)
{
	Customer *customer = new Customer();
	pumpQueue.push(customer);

	/*mutex.Wait();
	printf("GS: Queue size increased to %d\n", pumpQueue.size());
	mutex.Signal();*/
}

void RemoveFinishedCustomers(Customer *activeCustomers[])
{
	for (int i = 0; i < INT_NumPumps; i++)
	{
		Customer *customer = activeCustomers[i];
		if (customer == nullptr) continue;

		if (customer->IsDoneTransaction)
		{
			delete customer;
			activeCustomers[i] = nullptr;
			customer = nullptr;
		}
	}
}

void AssignNewCustomers(Customer *activeCustomers[], queue<Customer*> &pumpQueue)
{
	if (pumpQueue.size() == 0) return;

	for (int i = 0; i < INT_NumPumps; i++)
	{
		if (activeCustomers[i] == nullptr)
		{
			Customer *customer = pumpQueue.front();

			pumpQueue.pop();
			activeCustomers[i] = customer;
			customer->AssignToPump(i);
			customer->Resume();

			/*mutex.Wait();
			printf("GS: Queue size decreased to %d. Assigned customer %s to pump %d\n",
				pumpQueue.size(), customer->CustomerName.c_str(), i);
			mutex.Signal();*/

			if (pumpQueue.size() == 0) return;
		}
	}
}

int main()
{
	// Step 1, create pumps, and queues for each pump
	Pump *pumps[INT_NumPumps];
	Customer *activeCustomers[INT_NumPumps];
	queue<Customer*> pumpQueue;
	InitializePumps(pumps, activeCustomers);

	// Step 2, start creating customers at random intervals and add to the smallest queue
	srand(time(nullptr));
	int customerWaitTime, count;

	while (1)
	{
		CreateNewCustomer(pumpQueue);
		/*string message = "Customer queue is at " + to_string(pumpQueue.size());
		LogMessage(message.c_str());*/

		// Recalculate random customer time
		customerWaitTime = rand() % 6 + 5;
		count = 0;

		while (count < 1)
		{
			RemoveFinishedCustomers(activeCustomers);
			AssignNewCustomers(activeCustomers, pumpQueue);

			SLEEP(1000);
			count++;
		}
	}

	return 0;
}