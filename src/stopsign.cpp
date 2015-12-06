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
	//Take the double array workLoad and extract subarrays.

	for (int i = 0; i<numDirections; i++)
	{
		for (int j = 0; j<simulationLength*10; j++)
		{
			cout<<workLoad[i][j]<<" ";
		}
		cout<<endl;
	}

	/*pthread_t threads[numDirections+1];

	pthread_create(&threads[0], NULL, &Sign, NULL);
			for (int i = 1; i<numDirections+1; i++)
			{
				pthread_create(&threads[i], NULL, &Direction, (void*) workLoad[i-1]);
			}
*/
	statistics yay;
	yay.dummy=1;
	return yay;
}

void *Direction(void *Load)
{
	return NULL;
}

void *Sign()
{
	return NULL;
}
