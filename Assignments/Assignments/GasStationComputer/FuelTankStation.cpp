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
	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i]->Wait();
		data[i]->gasTankLevel = INT_MaxTankLevel;
		data[i]->tankStatus = INT_FuelTankOkStatus;
		mutex[i]->Signal();
	}
}

bool FuelTankStation::WithdrawGas(double amount, int gasType) {
	mutex[gasType]->Wait();

	bool status = false;

	if (data[gasType]->gasTankLevel >= amount)
	{
		data[gasType]->gasTankLevel -= amount;
		status = true;
	}

	if (data[gasType]->gasTankLevel <= INT_LowTankLevel)
	{
		data[gasType]->tankStatus = INT_FuelTankLowStatus;
	}

	mutex[gasType]->Signal();

	return status;
}

void FuelTankStation::RefillTanks()
{
	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i]->Wait();
		data[i]->gasTankLevel = INT_MaxTankLevel;
		data[i]->tankStatus = INT_FuelTankLowStatus;
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