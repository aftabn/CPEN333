#pragma once
#include "C:\CPEN Labs\Labs\rt.h"

class Car :
	public ActiveClass
{
private:
	int myNumber;
	bool isAccelerating;
	bool isStopping;
	bool isCruising;

	int main();
	void PrintStuff();
public:
	Car(int num);
	~Car();

	void Accelerate();
	void Stop();
	void Cruise();
};
