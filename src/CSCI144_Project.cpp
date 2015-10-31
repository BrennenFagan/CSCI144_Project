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
void *Sensor(void *direction);

//Simple Runtime Pass using Global Variable
double SimulationLength=0;

int main() {
	clock_t t; t=clock();
	cout<<"Simulation Length (in Seconds): ";cin>>SimulationLength;cout<<endl;

	//1: Multithreading
	//Thread 0: Intersection Sensor (Controls Queue)
	//Threads 1-4: Directional Sensors (Generate Cars, place in Queue)
	//Direction = 1 if Northbound, 2 if Eastbound, 3 if Southbound, 4 if Westbound
	pthread_t threads[5];

	pthread_create(&threads[0], NULL, &TrafficLight, NULL);
	int *direction = 1; //Northbound
	pthread_create(&threads[1], NULL, &Sensor, (void*) direction);
	int *direction = 2; //Eastbound
	pthread_create(&threads[2], NULL, &Sensor, (void*) direction);
	int *direction = 3; //Southbound
	pthread_create(&threads[3], NULL, &Sensor, (void*) direction);
	int *direction = 4; //Westbound
	pthread_create(&threads[4], NULL, &Sensor, (void*) direction);


	//	Returns the time taken.
	cout<<"Time Taken: "<<((float)clock()-t)/CLOCKS_PER_SEC<<" Seconds";

	return 0;
}
