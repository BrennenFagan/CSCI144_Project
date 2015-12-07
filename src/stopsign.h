/*
 * stopsign.h
 *
 *  Created on: Dec 5, 2015
 *      Author: root
 */



#ifndef STOPSIGN_H_
#define STOPSIGN_H_

#include<vector>

class statistics
{
public:
	int dummy;
};

class argument
{
public:
	int size;
	std::vector<double> contents;
	/*argument(int sz, double cont[]){
		size = sz;
		for (int i=0; i<size;i++)
			contents[i]=cont[i];
	};*/
};

statistics stopsign(int numDirections, double simulationLength, double **workLoad);


#endif /* STOPSIGN_H_ */
