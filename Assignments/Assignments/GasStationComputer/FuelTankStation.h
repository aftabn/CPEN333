#pragma once
#include "..\..\rt.h"

class FuelTankStation {
	struct FuelTankData
	{
		double gasTankLevel;
		int tankStatus;
	};

private:
	static const int INT_NumTanks = 4;
	static const int INT_MaxTankLevel = 500;
	static const int INT_LowTankLevel = 200;

	// Status constants
	static const int INT_FuelTankOkStatus = 10;			// Green
	static const int INT_FuelTankRefillingStatus = 2;	// Dark Green
	static const int INT_FuelTankLowStatus = 6;			// Yellow
	static const int INT_FuelTankEmptyStatus = 12;		// Red

	const int INT_OctaneGrade[INT_NumTanks] = { 87, 89, 92, 93 };

	CMutex *mutex[INT_NumTanks];
	CDataPool *dps[INT_NumTanks];
	struct FuelTankData *data[INT_NumTanks];

public:
	FuelTankStation();
	~FuelTankStation() {}

	void Initialize();
	double WithdrawGas(double amount, int gasType);
	void RefillTanks();
	double GetGas(int gasType) const;
	int GetStatusNumber(int gasType) const;
	string GetStatus(int gasType) const;
	int GetOctaneGrade(int gasType) const;
};