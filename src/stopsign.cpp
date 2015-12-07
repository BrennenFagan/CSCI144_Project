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

	//Basing Off of HW3.1
	//Create threads space
	pthread_t *threads;
	threads = (pthread_t *)malloc((numDirections+1) * sizeof(*threads)); //allocate memory space
/*
	for (int row = 0; row<rowsA; row++)
	{
		for (int col = 0; col<colsB; col++)
		{
			//Now we've broken the threads and pairs down by coordinate
			//each thread has coordinate: row*colsB + col

			//create a victim, assign its location and memory, and assign its vectors
			vectorpair *victim;
			victim = (vectorpair*)malloc(sizeof(*victim));

			//convert array to vector
			vector<int> Arow(A[row], A[row]+colsA);
			vector<int> BTrow(BT[col], BT[col] + colsA);

			victim->Arow=Arow;
			victim->Brow=BTrow;

			cout<<"Creating thread: "<<row*colsB+col<<endl;

			int success = pthread_create(&threads[row*colsB+col], NULL, &VectorDot, (void*)victim);
			if(success != 0)
			{
	            fprintf(stderr,"ERROR: LINE 201: THREAD CREATION");
			}
		}
	}
*/
	int check=0;
	for (int direction = 0; direction<numDirections+1; direction++)
	{
		argument *victim; victim = (argument*)malloc(sizeof(*victim));
		//move arguments into the victim
		if(direction)
			victim->size=simulationLength*10;
		else
			victim->size=0;
		if(direction)
		{
			//convert array to vector
			vector<double> LoadRow(workLoad[direction], workLoad[direction]+(simulationLength*10));
			victim->contents=LoadRow;
		}

		int success;
		if(direction)
			success = pthread_create(&threads[direction], NULL, &Direction, (void*)victim);
		else
			success = pthread_create(&threads[direction], NULL, &Sign, (void*)victim);
		if(success != 0)
		{
            fprintf(stderr,"ERROR: LINE 201: THREAD CREATION");
		}
		check-=(success-1);
	}
	cout<<"Number of Threads:"<<check;

	statistics yay;
	yay.dummy=1;
	return yay;
}

void *Direction(void *Load)
{
	//Now to reclaim our Load.
	int loadSize = ((class argument*) Load)->size;
	printf("Inside Thread with loadSize: %d \n",loadSize);

	//And the contents of course
	//vector <int> Arow = ((class vectorpair*)vectors)->Arow;
	vector<double> loadContents = ((class argument*) Load)->contents;
	double checksum = 0;
	for (int i = 0; i< loadSize; i++)
	{
		checksum+=loadContents[i];
	}
	printf("CheckSum: %G \n",checksum);


	return NULL;
}

void *Sign(void *)
{
	//Not showing up? Mild Concern.
	printf("Look Ma, Imma thread!");
	return NULL;
}
