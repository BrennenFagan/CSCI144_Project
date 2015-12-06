//============================================================================
// Name        : CSCI144_Project.cpp
// Author      : Brennen Fagan
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

/*
 * Title: Intelligent Traffic Light System
 * Language: C++
 * Description: In this project, you are required to simulate an intelligent
 * traffic light system with multithreading. A sensor is placed at each street
 * intersection and communicates sensors in the car when close enough
 * (50-100 meters). A FIFO queue is maintained by the sensor at the
 * intersection to store RTP (Request To Pass) by cars. Cars whose request is
 * at the HOL (Head of Line) is granted permission without stopping at the
 * intersection. Your program should simulate random car arriving on the four
 * directions and keeps running without deadlock, crashing, or unexpected termination.
 *
 * Note:
 * (i)   FIFO queue can be considered a shared resources for all the cars
 * 			when they arrive and leave;
 * (ii)  you can assume each car takes 3 seconds to
 * 			drive through the intersection;
 * (iii) turns are not required for implementation.
 *
 * Bonus (15%): implement an improvement where N cars in one direction is given
 * permission before allowing cars in another direction to pass. In addition,
 * cars in opposite directions should be able to pass simultaneously without
 * waiting for each other.
 *
 * Bonus (10%): analyze the performance of your program through extensive
 * experimental data. Metrics to analyze include average time to pass the
 * intersection (waiting + time to drive through) and average waiting time.
 * You can compare it with the "stop sign rule".
 */

/*
 * Summarized:
 *  1: Multithreading
 *  2: Shared FIFO Queue
 *  3: Head of Line Automatically Goes in Base Implementation
 *  4: Random Car Arrivals
 *  5: 3 Seconds to go through Intersection
 *  6: No Turning
 *
 *  B1 : Implement Multiple Cars going at once
 *  B2 : Implement a "Stop Sign Rule" for comparison and do Data Gathering
 */

/*
 * We assume:
 * 	People follow the rules of the road.
 * 	The stop-lights themselves are run by another system that reads the state of the sensor.
 */

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

//stopsign files
#include "stopsign.h"

//Workload files
#include <random>
#include <map>

using namespace std;

//Functions and Arguments
void *TrafficLight(void *arguments);//of TimeandDirection: direction = 0; Using this entirely to know when to stop
void *Sensor(void *arguments); //of TimeandDirection class

class TimeandDirection{
public:
	clock_t initialTime;
	double simulationLength;
	int direction;
};

class car{
public:
	clock_t arrivalTime;
	int direction;
};

vector<int> headOfLines; //Stores the positions of the directions. If (0) then there are no cars. Otherwise, 1, 1st ... n, nth in line.
vector<queue<float> > carQueues; //place to store ALL THE CARS, sorted by direction, and storing the cars' arrival times.
int backOfLine = 1;

//Locks
pthread_mutex_t sensorLock = PTHREAD_MUTEX_INITIALIZER;

int main() {
	//NUMBER OF DIRECTIONS
	int numDirections;
	srand(time(NULL));
	clock_t t; t=clock();
	double simulationLength;

	cout<<"Car Making Length (in Seconds): ";cin>>simulationLength;
	cout<<"Number of directions (min: 1)?: ";cin>>numDirections;
	if(numDirections<1)
		numDirections = 1;

	int runmode;
	cout<<"Please enter 0/1: Stop Sign(0) or Traffic Light(1): ";cin>>runmode;

	double mean;
	cout<<"How busy is your intersection on average in cars/second?: ";cin>>mean;
	double lambda=1/mean;

	//Create workloads: http://stackoverflow.com/questions/11491458/how-to-generate-random-numbers-with-exponential-distribution-with-mean

	default_random_engine generator;
	exponential_distribution<double> distribution(lambda);

	double workLoad[numDirections][int(simulationLength*10)];
	//Assuming max of 1 car per second
	//x by y	1				 2				  3	...	simulationLength
	//North		car arrival time car arrival time
	//East
	//South
	//West
	//...
	//numDirections
	for (int j = 0; j<numDirections;j++)
	{
		double sum=0; int i = 0;
		while (i<simulationLength*10 && sum<simulationLength*.9) //We can run outside of our time interval. Don't bother once we have. End if we are "close enough"
		{
		    double number = distribution(generator);
		    if (number<simulationLength-sum)
		    {
		    	workLoad[j][i]=number;
		    	cout<<number<<" ";
		    	sum+=number;
		    	i++;
		    }
		}
		for (i; i<simulationLength*10;i++) // populate with dummy information
		{
			workLoad[j][i] = -1;
		}
	    cout<<"Total: "<<sum;
		cout<<endl;
	}

	if(!runmode)
		stopsign(numDirections, simulationLength, &workLoad[numDirections][int(simulationLength*10)]);
	else
	{
		//Initialize the shared structures.
		headOfLines.assign(numDirections,0);
		carQueues.assign(numDirections, queue<float>());

		//1: Multithreading
		//Thread 0: Intersection Sensor (Controls Queue)
		//Threads 1-4: Directional Sensors (Generate Cars, place in Queue)
		//Direction = 1 if Northbound, 2 if Eastbound, 3 if Southbound, 4 if Westbound
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

		//Return from all threads: We wait for all Car Generators to be done before Queue
		for (int i = 1; i<numDirections+1; i++)
		{
			pthread_join(threads[i], NULL);
		}
		pthread_join(threads[0],NULL);

		//	Returns the time taken.
		cout<<"Time Taken: "<<((float)clock()-t)/CLOCKS_PER_SEC<<" Seconds";
	}

	return 0;
}

void *TrafficLight(void *arguments) //of TimeandDirection class
{
	clock_t t = ((class TimeandDirection*)arguments)->initialTime;
	double simulationLength = ((class TimeandDirection*)arguments)->simulationLength;
	//int direction = ((class TimeandDirection*)arguments)->direction; //not used.
	//We continue to operate the queue until there are both no cars remaining and our time is up.
	//Hence, we need to be passed the start time and stop time.
	cout<<"Houston, Traffic Queue is Go. \n";
	printf("size of carQueues: %d \n", carQueues.size());
	bool empty = true;
	while( not(empty) || (((float)clock()-t)/CLOCKS_PER_SEC<simulationLength) )
	{
		pthread_mutex_lock( &sensorLock );
		for (int i=0; i< carQueues.size();i++)
			carQueues[i]=queue<float>();

		pthread_mutex_unlock( &sensorLock );


		//Check if empty
		empty = true;
		for (int i=0; i< carQueues.size(); i++)
		{
			//If any are not empty, then the TrafficLight is not empty.
			if(not(carQueues[i].empty()))
			{
				empty = false;
				break;
			}
		}

	}
	return NULL;
}

void *Sensor(void *arguments) //of TimeandDirection class
{

	clock_t t = ((class TimeandDirection*)arguments)->initialTime;
	double simulationLength = ((class TimeandDirection*)arguments)->simulationLength;
	int direction = ((class TimeandDirection*)arguments)->direction;

	printf("Launch! %d \n", direction);

	//Generate a Car that arrives at some random time between the current clock and stop of the simulation.
	//Start = measured time
	//Stop = simulationLength + start time (t)
	//Length = Stop-Start

	clock_t measuredTime = clock(); // in clocks
	double stopTime = simulationLength + t/CLOCKS_PER_SEC;//in seconds
	double remainingTime = stopTime - measuredTime/CLOCKS_PER_SEC;//in seconds
	//Linear relationship between remainingTime and car probability.
	long double carTime = ((double)(rand()%100)/100)*remainingTime;//in seconds

	//While the current time is inside the simulation window
	//If the time has come for the next car to launch
	//Try to acquire the lock, and load the car into the queue
	//Generate the next car's arrival time within the remaining time
	while(((float)clock()-t)/CLOCKS_PER_SEC<simulationLength)
	{
		if(((float)clock()-t)/CLOCKS_PER_SEC>carTime)
		{
			//Time to load a car
			pthread_mutex_lock( &sensorLock );
			//printf("Load: carTime: %f, measuredTime: %f, remainingTime: %f \n", (double)carTime, (measuredTime/CLOCKS_PER_SEC), (double)remainingTime);
			//Load car into queue

			clock_t toLoad=clock()/CLOCKS_PER_SEC;
			//make sure our arrival is logged
			if(headOfLines[direction])
				;
			else//i.e. 0
			{
				headOfLines[direction]=backOfLine++;
			}
			//load the cars into their appropriate queue
			carQueues[direction].push((float)toLoad);

			//Done Loading Car
			pthread_mutex_unlock( &sensorLock );
			usleep(100000);// causes entire thread to sleep, and appears not to alter internal state of the thread, or something to that extent.
			//So instead of just delaying another car generation, this instead delays, without changing, the time measurements.

			//Generate another Car~
			measuredTime = clock(); // in clocks
			stopTime = simulationLength + t/CLOCKS_PER_SEC;//in seconds
			remainingTime = stopTime - measuredTime/CLOCKS_PER_SEC;//in seconds
			carTime = ((double)(rand()%100)/100)*remainingTime;//in seconds
		}
	}
	printf("Return, %d \n",direction);
	return NULL;
}
