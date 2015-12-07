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
void *Direction(argument Load); //Releases cars from Load into shared queues.
void *Sign(); //Take no arguments. It's just going to interact with the carQueues2.
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

	/*
	int **BT;
	BT = new int*[colsB];
	for (int i=0; i<colsB; i++)
	{
		BT[i] = new int[colsA];
		for (int j=0; j<colsA; j++)
		{
			//Populate BT[i][j]
			//BT[i][j]=0;
			BT[i][j]=B[j][i];
		}
	}
	*/

	thread threads[numDirections+1];
	for (int direction = 0; direction<numDirections+1;direction++)
	{
		if(direction)
		{
			argument load; load.size = simulationLength*10;
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
			threads[direction] = thread(Sign);
		}
	}
	for (int direction = 0; direction<numDirections+1;direction++)
	{
		threads[direction].join();
	}

	statistics yay;
	yay.dummy=1;
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


	return NULL;
}

void *Sign()
{
	printf("Look Ma, Imma thread!");

	return NULL;
}
