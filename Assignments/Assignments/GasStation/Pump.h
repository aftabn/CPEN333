#pragma once
#include "../../rt.h"
#include "../GasStationComputer/FuelTankStation.h"

class Pump :
	public ActiveClass
{
	struct CustomerData
	{
		long long creditCard;
		int fuelGrade;
		double requestedVolume;
		string customerName;
	};

private:

	const int INT_LineNumber = 19;

	const int INT_xCustomerInfo = 35;
	const int INT_yCustomerInfo = 9;
	const double DBL_GasFlowRate = 20;

	//Status constants
	static const int INT_PumpDisabledStatus = 7;				// Gray
	static const int INT_WaitingCustomerStatus = 6;				// Yellow
	static const int INT_WaitingAuthorizationStatus = 12;		// Red
	static const int INT_WaitingForFuelTankStationStatus = 14;	// Yellow
	static const int INT_DispensingGas = 10;					// Green

	FuelTankStation *fuelTankStation;
	CTypedPipe<CustomerData> *pipe;
	struct GasPumpData *data;
	CDataPool *dp;
	CMutex *gasStationMutex;
	CMutex *pumpMutex;
	CSemaphore *ps;
	CSemaphore *cs;

	int pumpNumber;
	double transactionCost;

	int main() override;
	void Initialize();

	void PrintEmptyDetails() const;
	void PrintCustomerDetails(struct CustomerData &cData) const;
	void StartTransaction(struct CustomerData &cData);
	void WaitForAuthorizationFromGSC(struct CustomerData &cData) const;
	void WaitUntilGasStationReady() const;
	void DispenseFuelUntilComplete(struct CustomerData &cData) const;
	string ReadStatus(int status) const;

	void LogMessage(char const *message, int line) const;
	void LogMessage(char const *message) const { LogMessage(message, INT_LineNumber); };

public:
	Pump(int num, FuelTankStation *fuelTankStation);
	~Pump();

	bool IsEnabled() const;
};
