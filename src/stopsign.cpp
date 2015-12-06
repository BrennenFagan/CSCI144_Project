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

//Functions
void *Direction(void *Load); //Releases cars from Load into shared queues.
void *Sign(void *); //Take no arguments. It's just going to interact with the carQueues2.
					//Needs access to each head of line and to record the order of the cars at the front.

pthread_mutex_t stopSignLock = PTHREAD_MUTEX_INITIALIZER;

statistics stopsign(int numDirections, double simulationLength, double** workLoad)
{
	cout<<"Inside stopSign"<<endl;
	for (int i=0; i<numDirections;i++)
	{
		for (int j=0; j<simulationLength*10;j++)
		{
			cout<<workLoad[i][j]<<" ";
		}
		cout<<endl;
	}

	//Using what worked earlier, and changing names:
	/*
	pthread_t threads[numDirections+1];
	//Create Arguments to pass;
	TimeandDirection arguments[numDirections+1];
	for (int i = 0; i<numDirections+1; i++)
	{
		arguments[i].initialTime=t;
		arguments[i].simulationLength=simulationLength;
		arguments[i].direction=i;
	}
	TimeandDirection * argpointer = arguments;

	//Officially Create Threads
	pthread_create(&threads[0], NULL, &TrafficLight, (void*) argpointer);
	for (int i = 1; i<numDirections+1; i++)
	{
		pthread_create(&threads[i], NULL, &Sensor, (void*) (argpointer+i-1));
	}
	 */

	pthread_t threads[numDirections+1];
	argument Load[numDirections];
	for (int i=0; i<numDirections;i++)
	{
		Load[i].size=simulationLength*10;
		for (int j=0; j<Load[i].size; j++)
			Load[i].contents[j]=workLoad[i][j];
	}
	argument * argpointer = Load;

	pthread_create(&threads[0], NULL, &Sign, (void*) argpointer);
	for (int i = 1; i<numDirections+1; i++)
	{
		pthread_create(&threads[i], NULL, &Direction, (void*) (argpointer+i-1));
		cout<<"Thread created"<<endl;
	}


	statistics yay;
	yay.dummy=1;
	return yay;
}

void *Direction(void *Load)
{
	cout<<"Inside Thread!";


	return NULL;
}

void *Sign(void *)
{
	return NULL;
}
