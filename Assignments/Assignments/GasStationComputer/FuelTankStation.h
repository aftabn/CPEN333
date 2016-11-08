#pragma once
#include "..\..\rt.h"

class FuelTankStation {
private:
	static const int INT_NumTanks = 4;
	static const int INT_MaxTankLevel = 500;

	CMutex *mutex[INT_NumTanks];
	CDataPool *dps[INT_NumTanks];
	double *gasTankLevels[INT_NumTanks];

public:
	FuelTankStation();
	~FuelTankStation() {}

	void Initialize();
	bool WithdrawGas(double amount, int gasType);
	void RefillTanks();
	double GetGas(int gasType);
};