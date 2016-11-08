#include "..\..\rt.h"
#include "FuelTankStation.h"

double FuelTankStation::gasTankLevel[INT_NumTanks];
CMutex* FuelTankStation::mutex[INT_NumTanks];

void FuelTankStation::Initialize()
{
	for (int i = 0; i < INT_NumTanks; i++) {
		mutex[i] = new CMutex("__Mutex__" + string("FuelTank") + to_string(i));
		gasTankLevel[i] = maxTankLevel;
	}
}

bool FuelTankStation::WithdrawGas(double amount, int gasType) {
	mutex[gasType]->Wait();
	bool status = false;

	if (gasTankLevel[gasType] >= amount) {
		status = true;
		gasTankLevel[gasType] -= amount;
		//printf("Successfully withdrew %f amount of gas from tank %d", amount, gasType);
	}

	else {
		//printf("Cannot withdraw %f amount of gas from tank %d . Not enough gas in tank", amount, gasType);
	}

	mutex[gasType]->Signal();
	return status;
}

void FuelTankStation::RefillTanks() {
	for (int i = 0; i < INT_NumTanks; i++)
	{
		mutex[i]->Wait();
		gasTankLevel[i] = maxTankLevel;
		mutex[i]->Signal();
	}
}

double FuelTankStation::GetGas(int gasType) {
	return gasTankLevel[gasType];
}