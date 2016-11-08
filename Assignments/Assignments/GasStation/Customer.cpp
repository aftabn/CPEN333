#include "Customer.h"
#include <ctime>

struct CustomerData
{
	long long creditCard;
	int fuelGrade;
	double requestedVolume;
	string customerName;
};

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
	data.customerName = this->CustomerName;

	pipe.Write(&data);
	mutex.Signal();
	SLEEP(1000);

	// Wait for whether transaction successful
	mutex.Wait();
	IsDoneTransaction = true;
	/*gasStationMutex.Wait();
	printf("CUSTOMER: I (%s) am done at pump %d\n\n", CustomerName.c_str(), pumpNumber);
	gasStationMutex.Signal();*/
	mutex.Signal();

	return 0;
}

Customer::Customer()
{
	srand(time(0));

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
	//String output
	long long creditCardNumber = 4000000000000000;

	for (int i = 0; i < 15; i++) {
		int digit = rand() % 10;
		creditCardNumber += digit * 10 ^ (15 - i);
	}

	return creditCardNumber;
}

string Customer::CreateRandomName()
{
	//Array of First Names
	string firstNames[] = { "Harley", "Rahul", "Armin", "Alexander", "Renee",  "Brandon", "Herman", "Evelyn", "Sasha", "Susan", "Monika", "Sophia", "Harish", "Zenia", "Mela", "Cameron", "Mira" };
	string lastNames[] = { "Shepard", "Bob", "Bobson", "Petersan", "Chang", "Chun", "Chin", "Zhang", "Lee", "Lu", "Liu", "Davan", "Bob", "Peter", "TheGreat" };

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