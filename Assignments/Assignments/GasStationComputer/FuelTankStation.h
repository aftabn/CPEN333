#pragma once
#include "..\..\rt.h"

class FuelTankStation {
private:
	static const int INT_NumTanks = 4;
	static const int maxTankLevel = 500;

	static double gasTankLevel[INT_NumTanks];
	static CMutex* mutex[INT_NumTanks];

	FuelTankStation() {};
	~FuelTankStation() {}

public:
	static void Initialize();

	//Function to withdraw gas from a specific tank
	static bool WithdrawGas(double amount, int gasType);

	//Function to deposit gas to a specific tank
	static void RefillTanks();

	//Function to display how much gas there is in each tank
	static double GetGas(int gasType);
};