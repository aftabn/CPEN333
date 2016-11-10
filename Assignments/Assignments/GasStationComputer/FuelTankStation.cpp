#include "..\..\rt.h"
#include "FuelTankStation.h"

FuelTankStation::FuelTankStation()
{
	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i] = new CMutex("__Mutex__" + string("FuelTank") + to_string(i));
		dps[i] = new CDataPool("FuelTank" + to_string(i), sizeof(struct FuelTankData));
		data[i] = (struct FuelTankData *)(dps[i]->LinkDataPool());
	}
}

void FuelTankStation::Initialize()
{
	const double DBL_GasCost[] = { 0.98, 1.02, 1.10, 1.25 };

	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i]->Wait();
		data[i]->gasTankLevel = INT_MaxTankLevel;
		data[i]->tankStatus = INT_FuelTankOkStatus;
		data[i]->gasCost = DBL_GasCost[i];
		mutex[i]->Signal();
	}
}

double FuelTankStation::WithdrawGas(double amount, int gasType)
{
	double gas = 0;

	mutex[gasType]->Wait();

	// If there's enough gas, withdraw
	if (data[gasType]->gasTankLevel >= amount)
	{
		data[gasType]->gasTankLevel -= amount;
		gas = amount;
	}

	// If gas drops below level, update status
	if (data[gasType]->gasTankLevel <= INT_LowTankLevel)
	{
		data[gasType]->tankStatus = INT_FuelTankLowStatus;
	}

	if (data[gasType]->gasTankLevel == 0)
	{
		data[gasType]->tankStatus = INT_FuelTankEmptyStatus;
	}

	mutex[gasType]->Signal();

	return gas;
}

void FuelTankStation::RefillTanks()
{
	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i]->Wait();
		data[i]->tankStatus = INT_FuelTankRefillingStatus;
		mutex[i]->Signal();
	}

	SLEEP(1000);

	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i]->Wait();
		data[i]->gasTankLevel = INT_MaxTankLevel;
		data[i]->tankStatus = INT_FuelTankOkStatus;
		mutex[i]->Signal();
	}
}

double FuelTankStation::GetGas(int gasType) const
{
	return data[gasType]->gasTankLevel;
}

string FuelTankStation::GetStatus(int gasType) const
{
	switch (GetStatusNumber(gasType))
	{
	case INT_FuelTankOkStatus: return "OK";
	case INT_FuelTankRefillingStatus: return "Refilling";
	case INT_FuelTankLowStatus: return "Low";
	case INT_FuelTankEmptyStatus: return "Empty";
	default: return "Error";
	}
}

int FuelTankStation::GetStatusNumber(int gasType) const
{
	return data[gasType]->tankStatus;
}

int FuelTankStation::GetOctaneGrade(int gasType) const
{
	return INT_OctaneGrade[gasType];
}

void FuelTankStation::setGasCost(int gasType, double gasCost)
{
	mutex[gasType]->Wait();
	data[gasType]->gasCost = gasCost;
	mutex[gasType]->Signal();
}

double FuelTankStation::GetGasCost(int gasType) const
{
	return data[gasType]->gasCost;
}