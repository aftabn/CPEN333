#pragma once
#include "../rt.h"

class Monitor
{
private:
	struct theData {
		int x;	// the data to be protected
	};

	CDataPool	*theDataPool;  // a datapool containing the data to be protected double balance;
	CMutex	    *theMutex;	       // a pointer to a hidden mutex protecting the ‘Balance’ variable above
	theData		*dataPtr;			// pointer to the data

public:
	int Read()
	{
		theMutex->Wait();
		return dataPtr->x;
		theMutex->Signal();
	}

	void Write(int n)
	{
		theMutex->Wait();
		dataPtr->x = n;
		theMutex->Signal();
	}

	void Increment()
	{
		theMutex->Wait();
		dataPtr->x++;
		theMutex->Signal();
	}

	void Decrement()
	{
		theMutex->Wait();
		dataPtr->x--;
		theMutex->Signal();
	}

	Monitor(string Name)
	{
		theMutex = new CMutex(string("__Mutex__") + string(Name));
		theDataPool = new CDataPool(string("__DataPool__") + string(Name), sizeof(struct theData));
		dataPtr = (struct theData *)(theDataPool->LinkDataPool());
		dataPtr->x = 0;
	}
};
