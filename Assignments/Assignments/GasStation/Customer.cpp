#include "Customer.h"
#include <ctime>

struct CustomerData
{
	long long creditCard;
	int fuelGrade;
	double requestedVolume;
	string customerName;
};

void LogMessage(CMutex &gasStationMutex, char const *message)
{
	gasStationMutex.Wait();
	MOVE_CURSOR(0, 20);
	printf("                                                                  \n");
	MOVE_CURSOR(0, 20);
	printf("CUSTOMER: %s\n", message);
	fflush(stdout);
	gasStationMutex.Signal();
}

int Customer::main()
{
	// Create pump mutex and pipeline
	CMutex gasStationMutex(string("__Mutex__") + string("GasStation"));
	CMutex mutex(string("__Mutex__") + string("Pump") + to_string(pumpNumber));
	CTypedPipe<CustomerData> pipe(string("Pipe") + to_string(pumpNumber), 1024);

	mutex.Wait();

	// Create customer data struct and then send off data
	struct CustomerData data;
	data.creditCard = this->creditCard;
	data.fuelGrade = this->fuelGrade;
	data.requestedVolume = this->requestedVolume;
	data.customerName = CustomerName;

	pipe.Write(&data);
	mutex.Signal();
	SLEEP(1000);

	mutex.Wait();
	IsDoneTransaction = true;
	mutex.Signal();
	SLEEP(2000); // Give the GasStation simulation time to delete customer object and assign new one

	return 0;
}

Customer::Customer()
{
	srand(time(0));

	IsDoneTransaction = false;
	creditCard = CreateRandomCreditCardNumber();
	CustomerName = CreateRandomName();
	fuelGrade = CreateRandomFuelGrade();
	requestedVolume = CreateRandomRequestedVolume();
}

Customer::~Customer()
{
}

void Customer::AssignToPump(int num)
{
	pumpNumber = num;
}

long long Customer::CreateRandomCreditCardNumber()
{
	long long creditCardNumber = 4LL;

	for (int i = 0; i < 15; i++)
	{
		creditCardNumber *= 10;
		creditCardNumber += rand() % 10;
	}

	return creditCardNumber;
}

string Customer::CreateRandomName()
{
	//Array of First Names
	string firstNames[] = { "Harley", "Rahul", "Armin", "Alexander", "Renee",  "Brandon", "Herman", "Evelyn", "Sasha", "Susan", "Monika", "Sophia", "Harish", "Zenia", "Mela", "Cameron", "Mira", "Anita" };
	string lastNames[] = { "Shepard", "Bob", "Bobson", "Petersan", "Chang", "Chun", "Chin", "Zhang", "Lee", "Lu", "Liu", "Davan", "Bob", "Peter", "TheGreat", "Ho" };

	//Choosing a random first name and last name
	srand(time(nullptr));

	int lengthFirstName = sizeof(firstNames) / sizeof(firstNames[0]);
	int lengthLastName = sizeof(lastNames) / sizeof(lastNames[0]);

	string firstName = firstNames[rand() % lengthFirstName];
	string lastName = lastNames[rand() % lengthLastName];

	string name = firstName + " " + lastName;

	return name;
}

int Customer::CreateRandomFuelGrade()
{
	srand(time(nullptr));
	return rand() % INT_NumFuelGrades;
}

double Customer::CreateRandomRequestedVolume()
{
	return rand() % 61 + 10;
}