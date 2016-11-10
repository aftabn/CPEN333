#pragma once
class Transaction
{
private:

public:
	Transaction(const char *purchaseTime, const char *customerName,
		long long creditCard, int fuelGrade, double dispensedFuel, double totalCost);
	~Transaction();

	char PurchaseTime[30];
	char CustomerName[30];
	long long CreditCard;
	int FuelGrade;
	double DispensedFuel;
	double TotalCost;
};
