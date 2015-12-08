/*
 * stopsign.h
 *
 *  Created on: Dec 5, 2015
 *      Author: root
 */



#ifndef STOPSIGN_H_
#define STOPSIGN_H_

#include <vector>
#include <iostream>
#include <queue>
#include <algorithm>

//Thread Utilities
#include <thread>
#include <future>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mutex>

//Timing Utilities
#include <time.h>
#include <unistd.h>

class statistics
{
public:
	long double mean;
	long double median;
	long double min;
	long double max;

	statistics(
				long double mean,
				long double median,
				long double min,
				long double max)
	{
		this->mean=mean;
		this->median=median;
		this->min=min;
		this->max=max;
	}
};

class argument
{
public:
	int size;
	int direction;
	std::vector<double> contents;
	/*argument(int sz, double cont[]){
		size = sz;
		for (int i=0; i<size;i++)
			contents[i]=cont[i];
	};*/
};

statistics stopsign(int numDirections, double simulationLength, double **workLoad);


#endif /* STOPSIGN_H_ */
