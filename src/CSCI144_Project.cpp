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

#include <iostream>

//Thread Utilities
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Timing Utilities
#include <time.h>

using namespace std;

void *TrafficLight(void *);
void *Sensor(void *arguments); //of TimeandDirection class

class TimeandDirection{
public:
	clock_t initialTime;
	double simulationLength;
	int direction;
};

int main() {
	clock_t t; t=clock();
	double simulationLength;
	cout<<"Simulation Length (in Seconds): ";cin>>simulationLength;cout<<endl;

	//1: Multithreading
	//Thread 0: Intersection Sensor (Controls Queue)
	//Threads 1-4: Directional Sensors (Generate Cars, place in Queue)
	//Direction = 1 if Northbound, 2 if Eastbound, 3 if Southbound, 4 if Westbound
	pthread_t threads[5];
	//Create Arguments to pass;
	TimeandDirection arguments[4];
	for (int i = 0; i<4; i++)
	{
		arguments[i].initialTime=t;
		arguments[i].simulationLength=simulationLength;
		arguments[i].direction=i;
	}
	TimeandDirection * argpointer = arguments;

	//Officially Create Threads
	pthread_create(&threads[0], NULL, &TrafficLight, NULL);
	for (int i = 1; i<5; i++)
	{
		pthread_create(&threads[i], NULL, &Sensor, (void*) (argpointer+i-1));
	}

	//Return from all threads: We wait for all Car Generators to be done before Queue
	for (int i = 1; i<5; i++)
	{
		pthread_join(threads[i], NULL);
	}
	pthread_join(threads[0],NULL);

	//	Returns the time taken.
	cout<<"Time Taken: "<<((float)clock()-t)/CLOCKS_PER_SEC<<" Seconds";

	return 0;
}

void *TrafficLight(void *)
{
	cout<<"WHOOOOO!"<<endl;
	return NULL;
}

void *Sensor(void *arguments) //of TimeandDirection class
{

	clock_t t = ((class TimeandDirection*)arguments)->initialTime;
	double simulationLength = ((class TimeandDirection*)arguments)->simulationLength;
	int direction = ((class TimeandDirection*)arguments)->direction;

	cout<<"WHEEEEEE!"<<endl;
	while((float)clock()/CLOCKS_PER_SEC-t>0)
	{

	}
	return NULL;
}
