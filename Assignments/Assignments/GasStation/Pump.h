#pragma once
#include "C:\CPEN Labs\Assignments\rt.h"
class Pump :
	public ActiveClass
{
private:
	static const double DBL_GasCost[4];

	const int INT_xCustomerInfoWidth = 35;
	const int INT_yCustomerInfoWidth = 8;
	const double DBL_GasFlowRate = 0.5;

	//Fuel Cost

	int pumpNumber;
	int main();
	void PrintCustomerDetails(CMutex &gasStationMutex, struct CustomerData &data,
		double dispensedVolume) const;

public:
	Pump(int num);
	~Pump();
};
