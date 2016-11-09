#pragma once
#include "C:\CPEN Labs\Assignments\rt.h"
class Customer :
	public ActiveClass
{
	static const int INT_NumFuelGrades = 4;

private:
	int pumpNumber;
	long long creditCard;
	double requestedVolume;
	int fuelGrade;

	int main();
	static long long Customer::CreateRandomCreditCardNumber();
	static string Customer::CreateRandomName();
	static int Customer::CreateRandomFuelGrade();
	static double Customer::CreateRandomRequestedVolume();

public:
	Customer();
	~Customer();

	string CustomerName;
	bool IsDoneTransaction;
	void AssignToPump(int num);
};
