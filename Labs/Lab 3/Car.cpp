#include "Car.h"
#include <stdio.h>
#include "..\rt.h"

Car::Car(int num)
{
	myNumber = num;
	isAccelerating = false;
	isCruising = false;
	isStopping = false;
}

Car::~Car()
{
}

void Car::Accelerate()
{
	isAccelerating = true;
}

void Car::Stop()
{
	isStopping = true;
}

void Car::Cruise()
{
	isCruising = true;
}

void Car::PrintStuff()
{
	for (int i = 0; i < 50; i++)
	{
		if (isAccelerating)
		{
			printf("Car %d is accelerating\n", myNumber);
			isAccelerating = false;
		}
		else if (isStopping)
		{
			printf("Car %d is stopping\n", myNumber);
			isStopping = false;
		}
		else if (isCruising)
		{
			printf("Car %d is cruising\n", myNumber);
			isCruising = false;
		}
		else printf("Say Hello to Car %d\n", myNumber);

		SLEEP(200);
	}
}

PerThreadStorage int myThreadNumber;

UINT __stdcall ChildThread(void *args)
{
	myThreadNumber = *(int *)(args);

	for (int i = 0; i < 10; i++) {
		printf("Speed of Car %d is %d\n", myThreadNumber, myThreadNumber * 10);
		Sleep(2000);
	}

	return 0;
}

int Car::main()
{
	int threadNum = myNumber;
	CThread *speedometer;
	speedometer = new CThread(ChildThread, ACTIVE, &threadNum);

	PrintStuff();

	speedometer->WaitForThread();

	return 0;
}