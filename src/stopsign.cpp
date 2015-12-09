/*
 * stopsign.cpp
 *
 *  Created on: Dec 5, 2015
 *      Author: root
 */
#include "stopsign.h"

#include <iostream>
#include <queue>

//Thread Utilities
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mutex>

//Timing Utilities
#include <time.h>
#include <unistd.h>

using namespace std;

//Global Variables
vector< queue<clock_t> > carQueues2;
vector<int> headOfTraffic2;
//if 0, noone in lane
//else, turn order
//e.g. 1,3,2,4 => release lane 0, then lane 2, then lane 1, then lane 3.

//Functions
void *Direction(argument Load); //Releases cars from Load into shared queues.
statistics Sign(int DailyLoad); //Take no arguments. It's just going to interact with the carQueues2.
					//Needs access to each head of line and to record the order of the cars at the front.

//Lock guards carQueues2
pthread_mutex_t StopSignLock2 = PTHREAD_MUTEX_INITIALIZER; //CarQueues2
pthread_mutex_t HeadLock2 = PTHREAD_MUTEX_INITIALIZER; //HeadOfTraffic2


statistics stopsign(int numDirections, double simulationLength, double** workLoad)
{
/*	cout<<"Inside stopSign"<<endl;*/

	//Reset all global variables
	pthread_mutex_lock( &HeadLock2 );
	headOfTraffic2.resize(numDirections);
	pthread_mutex_unlock( &HeadLock2 );

	pthread_mutex_lock( &StopSignLock2 );
	//Empty existing queues, followed by resizing for appropriate length
	carQueues2.resize(numDirections);
	//For safety, remove anything within the size.
	for(int direction = 0; direction<numDirections;direction++)
	{
		while(!carQueues2[direction].empty())
			carQueues2[direction].pop();
	}
	pthread_mutex_unlock( &StopSignLock2 );

	int DailyLoad = 0;
	for (int i=0; i<numDirections;i++)
	{
		for (int j=0; j<simulationLength*10;j++)
		{
			//cout<<workLoad[i][j]<<" ";
			if(workLoad[i][j]!=-1)
				DailyLoad++;
			else
				break;
		}
		//cout<<endl;
	}

	/*http://stackoverflow.com/questions/7686939/c-simple-return-value-from-stdthread
	int func() { return 1; }
	std::future<int> ret = std::async(&func);
	int i = ret.get();
	*/

	//Turns out that this function launches a thread as well. In the interest of time, I'm going to leave the spare threads[0];
	future<statistics> signReturn = async(&Sign, DailyLoad);
	thread threads[numDirections+1];
	for (int direction = 1; direction<numDirections+1;direction++)
	{
		if(direction)
		{
			argument load; load.size = simulationLength*10;
			load.direction=direction-1;
			vector<double>loadContents(load.size,-1);
			for (int j=0; j<load.size; j++)
			{
				double temp = workLoad[direction-1][j];
				loadContents[j]=temp;
			}
			load.contents=loadContents;
			threads[direction] = thread(Direction,load);
		}
		else
		{
			threads[direction] = thread(&Sign, DailyLoad);
		}
	}

	for (int direction = 1; direction<numDirections+1;direction++)
	{
		threads[direction].join();
	}

	statistics yay = signReturn.get();
	return yay;
}

void *Direction(argument Load)
{
	double checksum=0;
	for(int i=0; i<Load.size; i++)
	{
		if(Load.contents[i]==-1)
			break;
		checksum+=Load.contents[i];
	}
	//printf("I've a load in my pocket, CheckSum = %G\n",checksum);

	//Retrieve the current time t.
	clock_t t; t=clock();//Measured in Clocks

	//Iterate through all cars.
	for (int i=0;  i<Load.size;i++)
	{

		//SAMPLE: printf("Wait time: %Lf \n",((long double)(nowWait-busyWait))/CLOCKS_PER_SEC);
		//At each car, get the current time, and wait for the current time + the car's double value
		clock_t nowTime = clock();
		while(nowTime<t+Load.contents[i]*CLOCKS_PER_SEC)//NOTE MEASURED IN CLOCKS: While current Time is less than the time to launch, wait.
		{nowTime = clock();}

		//When the car's time has come, push it to the appropriate CarQueues2[direction] with the current time
		//We push said current time in order to get the statistics for later.

		pthread_mutex_lock( &StopSignLock2 );
		pthread_mutex_lock( &HeadLock2 );

		//On push, we need to check if(!headOfTraffic[direction]). If that is true, we need to assign it the next largest value of the values specified.
		if(!headOfTraffic2[Load.direction]) //Head of Traffic is 0
		{
			int max = 0; //We set max to be the value 1 above the maximum value. This tells us when it will be our turn to go.
			for(int j=0; j<headOfTraffic2.size();j++)
			{
				if(headOfTraffic2[j]>=max)
					max=headOfTraffic2[j]+1;
			}
			headOfTraffic2[Load.direction]=max;
		}

		//Get an accurate time read
		nowTime = clock();
		carQueues2[Load.direction].push(nowTime); //NOTE, WE ARE SPECIFICALLY PASSING CLOCKS

		pthread_mutex_unlock( &HeadLock2 );
		pthread_mutex_unlock( &StopSignLock2 );
		printf("StopSign: Direction: %d, time of arrival: %Lf! \n", Load.direction,(long double) nowTime/CLOCKS_PER_SEC);
		//Refresh the time t, so that the next car launches at the correct time.
		t=clock();
		//Once all cars have been pushed (signified by a -1) we break and call it a day for this function.
		if(i+1==Load.size||Load.contents[i+1]==-1)
			break;
	}
	return NULL;
}

statistics Sign(int DailyLoad)
{
	//printf("Expected Load: %d\n",DailyLoad);
	vector<long double> timeDifferences={};

	int carsThrough=0;
	//This function monitors the carQueues2, while it waits for the dailyLoad to be done.
	while(carsThrough<DailyLoad)
	{
		int anyoneWaiting=-1;
		pthread_mutex_lock( &HeadLock2 ); //Request Permission to access HeadOfTraffic
		for (int direction = 0; direction < headOfTraffic2.size(); direction++)
		{
			if(headOfTraffic2[direction]&&											//If there is anyone waiting in any lane
					(anyoneWaiting==-1 || 											//AND we have not Detected anyone OR
							headOfTraffic2[direction]<headOfTraffic2[anyoneWaiting]))	//This someone has higher (<current) priority
				anyoneWaiting=direction;											//Assign our direction of interest to them.
		}
		pthread_mutex_unlock( &HeadLock2 );
		if(anyoneWaiting==-1)
			{
				usleep(100);
				continue; //If we couldn't find anyone, try again.
			}

		//We now have the one person at the head of the line.
		//We pop the car in the lane with the lowest value, acquire lock, and decrement all values. If there is a car still in the lane, they get max value (size).

		//Acquire all relevant Locks. Note we do this in the same order that we do it in the other function.
		pthread_mutex_lock( &StopSignLock2 );
		pthread_mutex_lock( &HeadLock2 );

		//We pop the car in the lane with the lowest value...
		//Recall that anyoneWaiting has the direction of the lowest value
		clock_t carTimeLoaded =
				carQueues2[anyoneWaiting].front();
				carQueues2[anyoneWaiting].pop();
		clock_t carTimeEnters = clock();

		//...decrement all values...:	HeadOfLine
		for(int direction = 0; direction<headOfTraffic2.size();direction++)
		{
			if(direction==anyoneWaiting)//If this is the lane we popped from
			{
				//Check to see if there are more cars
				if(carQueues2[direction].empty())
					//If not, then put headOfTraffic to 0.
					headOfTraffic2[direction]=0;
				else
					//If so, then put headOfTraffic = max + 1 of all other directions
				{
					int max = 0; //We set max to be the value 1 above the maximum value. This tells us when it will be our turn to go.
					for(int j=0; j<headOfTraffic2.size();j++)
					{
						if(headOfTraffic2[j]>=max)
							max=headOfTraffic2[j]+1;
					}
					headOfTraffic2[direction]=max;
				}
			}
			else
			{
				if(headOfTraffic2[direction])//If it has someone waiting at the head of the Line
					headOfTraffic2[direction]--;//Move them towards the front of the line.
			}
		}
		pthread_mutex_unlock( &HeadLock2 );
		pthread_mutex_unlock( &StopSignLock2 );

		//It takes 3 seconds for a car to pass through the intersection from a complete stop, which we have since we are simulating a stopsign.
		printf("Service time: %Lf \n",(long double)(carTimeEnters-carTimeLoaded)/CLOCKS_PER_SEC);

		//sleep(3);//Functionally, we want to wait for 3 seconds, but sleep doesn't appear to effect the actual service time. Hence, we busy wait. Sadness.
		clock_t busyWait = clock();
		clock_t nowWait = clock();
		while((((long double)(nowWait-busyWait))/CLOCKS_PER_SEC)<3)
		{/*Busy Wait of extreme sadness*/
			//printf("Wait time: %Lf \n",((long double)(nowWait-busyWait))/CLOCKS_PER_SEC);
			nowWait = clock();
		}

		timeDifferences.push_back((long double)(carTimeEnters-carTimeLoaded));


		//On Pop, we increment the daily load counter
		carsThrough++;
	}

	//We need to perform statistics, such as max, min, mean, and median. Easier to do if sorted.
	sort(timeDifferences.begin(),timeDifferences.end());

	long double mean=0;//initialize

	long double median=0;
	if(timeDifferences.size()%2 == 0)//If 2|timeDifferences.size()
		median = (timeDifferences[timeDifferences.size()/2] + timeDifferences[timeDifferences.size()/2-1]) /2;
	else							 //If 2|timeDifferences.size()+1
		median = timeDifferences[timeDifferences.size()/2]; //Abuse of cast to some extent

	long double min=timeDifferences[0];
	long double max=timeDifferences[timeDifferences.size()-1];

	for(int current = 0; current<timeDifferences.size(); current++)
	{
		mean+=timeDifferences[current];
	}
	mean/=timeDifferences.size();

	mean/=CLOCKS_PER_SEC;
	median/=CLOCKS_PER_SEC;
	min/=CLOCKS_PER_SEC;
	max/=CLOCKS_PER_SEC;

	statistics yay(mean,median,min,max);
	return yay;
}
