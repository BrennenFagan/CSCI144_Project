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
vector< queue<double> > carQueues2;
int currentLoad;
vector<int> headOfTraffic;
//if 0, noone in lane
//else, turn order
//e.g. 1,3,2,4 => release lane 0, then lane 2, then lane 1, then lane 3.

//Functions
void *Direction(argument Load); //Releases cars from Load into shared queues.
statistics Sign(int DailyLoad); //Take no arguments. It's just going to interact with the carQueues2.
					//Needs access to each head of line and to record the order of the cars at the front.

//Lock guards carQueues2
pthread_mutex_t StopSignLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t LoadLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t HeadLock = PTHREAD_MUTEX_INITIALIZER;


statistics stopsign(int numDirections, double simulationLength, double** workLoad)
{
/*	cout<<"Inside stopSign"<<endl;*/

	pthread_mutex_lock( &LoadLock );
	currentLoad=0;
	pthread_mutex_unlock( &LoadLock );
	pthread_mutex_lock( &HeadLock );
	headOfTraffic.resize(numDirections);
	pthread_mutex_unlock( &HeadLock );

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
		cout<<endl;
	}

	/*http://stackoverflow.com/questions/7686939/c-simple-return-value-from-stdthread
int func() { return 1; }
std::future<int> ret = std::async(&func);
int i = ret.get();
	*/
	future<statistics> signReturn = async(&Sign, DailyLoad);
	thread threads[numDirections+1];
	for (int direction = 0; direction<numDirections+1;direction++)
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

	for (int direction = 0; direction<numDirections+1;direction++)
	{
		threads[direction].join();
	}

	statistics yay = signReturn.get();
	return yay;
}

void *Direction(argument Load)
{
	/*double checksum=0;
	for(int i=0; i<Load.size; i++)
	{
		if(Load.contents[i]==-1)
			break;
		checksum+=Load.contents[i];
	}
	printf("I've a load in my pocket, CheckSum = %G\n",checksum);*/

	//Retrieve the current time t.
	clock_t t; t=clock();

	//Iterate through all cars.

	//At each car, get the current time, and wait for the current time + the car's double value

	//When the car's time has come, push it to the appropriate CarQueues2[direction] with the current time
	//We push said current time in order to get the statistics for later.

	//On push, we need to check if(!headOfTraffic[direction]). If that is true, we need to assign it the next largest value of the values specified.

	//Once all cars have been pushed (signified by a -1) we break and call it a day for this function.

	return NULL;
}

statistics Sign(int DailyLoad)
{
	/*printf("Look Ma, Imma thread!");*/

	//This function monitors the carQueues2, while it waits for the dailyLoad to be done.

	//We pop the car in the lane with the lowest value, acquire lock, and decrement all values. If there is a car still in the lane, they get max value (size).

	//On Pop, we increment the daily load.

	//It takes 3 seconds for a car to pass through the intersection from a complete stop, which we have since we are simulating a stopsign.


	statistics yay;
	return yay;
}
