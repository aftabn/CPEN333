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
	static const double DBL_GasCost[4];

	const int INT_LineNumber = 19;

	const int INT_xCustomerInfo = 35;
	const int INT_yCustomerInfo = 9;
	const double DBL_GasFlowRate = 5;

	//Status constants
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

	int main() override;
	void Initialize();

	void PrintEmptyDetails() const;
	void PrintCustomerDetails(struct CustomerData &cData) const;
	void StartTransaction(struct CustomerData &cData) const;
	void WaitForAuthorizationFromGSC(struct CustomerData &cData) const;
	void WaitUntilGasStationReady() const;
	void DispenseFuelUntilComplete(struct CustomerData &cData) const;
	string ReadStatus(int status) const;

	void LogMessage(char const *message, int line) const;
	void LogMessage(char const *message) const { LogMessage(message, INT_LineNumber); };

public:
	Pump(int num, FuelTankStation *fuelTankStation);
	~Pump();
};
