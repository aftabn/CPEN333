#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Transaction.h"
#include <cstring>

Transaction::Transaction(const char *purchaseTime, const char *customerName,
	long long creditCard, int fuelGrade, double dispensedFuel, double totalCost)
{
	strcpy(PurchaseTime, purchaseTime);
	strcpy(CustomerName, customerName);
	CreditCard = creditCard;
	FuelGrade = fuelGrade;
	DispensedFuel = dispensedFuel;
	TotalCost = totalCost;
}

Transaction::~Transaction()
{
}