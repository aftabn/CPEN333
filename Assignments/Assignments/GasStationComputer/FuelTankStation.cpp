#include "..\..\rt.h"
#include "FuelTankStation.h"

FuelTankStation::FuelTankStation()
{
	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i] = new CMutex("__Mutex__" + string("FuelTank") + to_string(i));
		dps[i] = new CDataPool("FuelTank" + to_string(i), sizeof(double));
		gasTankLevels[i] = (double *)(dps[i]->LinkDataPool());
	}
}

void FuelTankStation::Initialize()
{
	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i]->Wait();
		*gasTankLevels[i] = INT_MaxTankLevel;
		mutex[i]->Signal();
	}
}

bool FuelTankStation::WithdrawGas(double amount, int gasType) {
	mutex[gasType]->Wait();

	bool status = false;

	if (*gasTankLevels[gasType] >= amount)
	{
		*gasTankLevels[gasType] -= amount;
		status = true;
	}

	mutex[gasType]->Signal();

	return status;
}

void FuelTankStation::RefillTanks()
{
	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i]->Wait();
		*gasTankLevels[i] = INT_MaxTankLevel;
		mutex[i]->Signal();
	}
}

double FuelTankStation::GetGas(int gasType)
{
	return *gasTankLevels[gasType];
}