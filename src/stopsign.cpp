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

statistics stopsign(int numDirections, double simulationLength)
{
	cout<<"Architecture works!";

	//We seek to make a numDirections intersection with two directional moves.
	//This will be run alongside the traffic light for comparison.
	//Eventually, we will have to modify this to take in a "workload" of cars.
	//For now, we implement the structure.

	statistics yay;
	yay.dummy=1;
	return yay;
}
