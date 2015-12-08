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



//stopsign files
#include "stopsign.h"

//Workload files
#include <random>
#include <map>

using namespace std;

//Functions and Arguments
statistics TrafficLight(int DailyLoad);//Based on stopsign.cpp's Sign
void *Sensor(argument Load); //Based on Direction, but modifying different variables and locks
statistics WRAPPER(int numDirections, double simulationLength, double** workLoad);// To make implementation somewhat more modular.

vector<int> headOfTraffic; //Stores the positions of the directions. If (0) then there are no cars. Otherwise, 1, 1st ... n, nth in line.
vector<queue<clock_t> > carQueues; //place to store ALL THE CARS, sorted by direction, and storing the cars' arrival times.


//Locks
pthread_mutex_t sensorLock = PTHREAD_MUTEX_INITIALIZER;//carQueues
pthread_mutex_t headLock = PTHREAD_MUTEX_INITIALIZER;//headOfTraffic

int main() {
	srand(time(NULL));

	//Simulation Length
	double simulationLength;
	cout<<"Car Making Length (in Seconds, non-negative): ";cin>>simulationLength;
	if(simulationLength<=0)
		simulationLength=1;

	//Number of Directions
	int numDirections;
	cout<<"Number of directions (min: 1)?: ";cin>>numDirections;
	if(numDirections<1)
		numDirections = 1;

	//Type of Simulation
	int runmode = 2;
	cout<<"Please enter 0/1/2: Stop Sign(0) or Traffic Light(1) or Both(else): ";cin>>runmode;

	//Determine Distribution
	double mean =1;
	cout<<"How busy is your intersection? Enter exponential distribution mean (small numbers => more cars): ";cin>>mean;
	double lambda=1/mean;

	//Create workloads: http://stackoverflow.com/questions/11491458/how-to-generate-random-numbers-with-exponential-distribution-with-mean

	default_random_engine generator;
	exponential_distribution<double> distribution(lambda);

	double **workLoad;
	workLoad = new double*[numDirections];

	//double workLoad[numDirections][int(simulationLength*10)];
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
		workLoad[j] = new double[(int)simulationLength*10];
		double sum=0; int i = 0;
		while (i<simulationLength*10 && sum<simulationLength*.9)
			//We can run outside of our time interval. Don't bother once we have. End if we are "close enough"
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
	{
		statistics stopSignResults = stopsign(numDirections, simulationLength, workLoad);
		//ALL RETURN VALUES ARE IN CLOCKS, NEED TO BE CONVERTED
		cout<<"Mean: "<<stopSignResults.mean/CLOCKS_PER_SEC<<endl;
		cout<<"Median: "<<stopSignResults.median/CLOCKS_PER_SEC<<endl;
		cout<<"Min: "<<stopSignResults.min/CLOCKS_PER_SEC<<endl;
		cout<<"Max: "<<stopSignResults.max/CLOCKS_PER_SEC<<endl;
	}

	else if(runmode==1)
	{
		statistics Results = WRAPPER(numDirections, simulationLength, workLoad);
		//ALL RETURN VALUES ARE IN CLOCKS, NEED TO BE CONVERTED
		cout<<"Mean: "<<Results.mean/CLOCKS_PER_SEC<<endl;
		cout<<"Median: "<<Results.median/CLOCKS_PER_SEC<<endl;
		cout<<"Min: "<<Results.min/CLOCKS_PER_SEC<<endl;
		cout<<"Max: "<<Results.max/CLOCKS_PER_SEC<<endl;
	}

	else //run both
	{
		statistics stopSignResults = stopsign(numDirections, simulationLength, workLoad);
		statistics Results = WRAPPER(numDirections, simulationLength, workLoad);
	}

	return 0;
}

statistics WRAPPER(int numDirections, double simulationLength, double** workLoad)
{
	//Reset all global variables//////////////////////////////////////////////////////////

			pthread_mutex_lock( &headLock );
			headOfTraffic.resize(numDirections);
			pthread_mutex_unlock( &headLock );

			pthread_mutex_lock( &sensorLock );
			//Empty existing queues, followed by resizing for appropriate length
			carQueues.empty(); carQueues.resize(numDirections);
			//For safety, remove anything within the size.
			for(int direction = 0; direction<numDirections;direction++)
			{
				while(!carQueues[direction].empty())
					carQueues[direction].pop();
			}
			pthread_mutex_unlock( &sensorLock );
			//Finish Reset///////////////////////////////////////////////////////////////////////

			//Determine the total load to expect/////////////////////////////////////////////////
			int DailyLoad = 0;
			for (int i=0; i<numDirections;i++)
			{
				for (int j=0; j<simulationLength*10;j++)
				{
					if(workLoad[i][j]!=-1)
						DailyLoad++;
					else
						break;
				}
			}
			//Finished Determining Load//////////////////////////////////////////////////////////

			//Begin Multithreading///////////////////////////////////////////////////////////////
			//Launch the TrafficLight thread
			//Example: future<statistics> signReturn = async(&Sign, DailyLoad);
			future<statistics> signalReturn = async(&TrafficLight, DailyLoad);

			//Create a Thread for each direction
			thread threads[numDirections];
			for (int direction = 0; direction<numDirections; direction++)
			{
				//create each thread's load
				argument load; load.size = simulationLength*10;
				load.direction=direction;
				vector<double>loadContents(load.size,-1);
				for (int j=0; j<load.size; j++)
				{
					double temp = workLoad[direction][j];
					loadContents[j]=temp;
				}
				load.contents=loadContents;

				//launch the thread with its load
				threads[direction] = thread(Sensor,load);
			}
			//Done Launching Threads////////////////////////////////////////////////////////////

			for (int direction = 0; direction<numDirections;direction++)
			{
				threads[direction].join();
			}

			return signalReturn.get();
}

statistics TrafficLight(int DailyLoad) //of TimeandDirection class
{
	printf("Expected Load: %d\n",DailyLoad);
	vector<long double> timeDifferences={};

	int carsThrough=0;
	//This function monitors the carQueues2, while it waits for the dailyLoad to be done.
	while(carsThrough<DailyLoad)
	{
		int anyoneWaiting=-1;
		pthread_mutex_lock( &headLock ); //Request Permission to access HeadOfTraffic
		for (int direction = 0; direction < headOfTraffic.size(); direction++)
		{
			if(headOfTraffic[direction]&&												//If there is anyone waiting in any lane
					(anyoneWaiting==-1 || 												//AND we have not Detected anyone OR
							headOfTraffic[direction]<headOfTraffic[anyoneWaiting]))		//This someone has higher (<current) priority
				anyoneWaiting=direction;												//Assign our direction of interest to them.
		}
		pthread_mutex_unlock( &headLock );
		if(anyoneWaiting==-1)
			{
				usleep(100);
				continue; //If we couldn't find anyone, try again.
			}

		//We now have the one person at the head of the line.
		//We pop the car in the lane with the lowest value, acquire lock, and decrement all values. If there is a car still in the lane, they get max value (size).

		//Acquire all relevant Locks. Note we do this in the same order that we do it in the other function.
		pthread_mutex_lock( &sensorLock );
		pthread_mutex_lock( &headLock );

		//We pop the car in the lane with the lowest value...
		//Recall that anyoneWaiting has the direction of the lowest value
		clock_t carTimeLoaded =
				carQueues[anyoneWaiting].front();
				carQueues[anyoneWaiting].pop();
		clock_t carTimeEnters = clock();

		//...decrement all values...:	HeadOfLine
		for(int direction = 0; direction<headOfTraffic.size();direction++)
		{
			if(direction==anyoneWaiting)//If this is the lane we popped from
			{
				//Check to see if there are more cars
				if(carQueues[direction].empty())
					//If not, then put headOfTraffic to 0.
					headOfTraffic[direction]=0;
				else
					//If so, then put headOfTraffic = max + 1 of all other directions
				{
					int max = 0; //We set max to be the value 1 above the maximum value. This tells us when it will be our turn to go.
					for(int j=0; j<headOfTraffic.size();j++)
					{
						if(headOfTraffic[j]>=max)
							max=headOfTraffic[j]+1;
					}
					headOfTraffic[direction]=max;
				}
			}
			else
			{
				if(headOfTraffic[direction])//If it has someone waiting at the head of the Line
					headOfTraffic[direction]--;//Move them towards the front of the line.
			}
		}
		pthread_mutex_unlock( &headLock );
		pthread_mutex_unlock( &sensorLock );

		//It takes 3 seconds for a car to pass through the intersection from a complete stop, which we have since we are simulating a stopsign.
		printf("Service time: %Lf \n",(long double)(carTimeEnters-carTimeLoaded));

		//sleep(3);//Functionally, we want to wait for 3 seconds, but sleep doesn't appear to effect the actual service time. Hence, we busy wait. Sadness.
		clock_t busyWait = clock();
		clock_t nowWait = clock();
		while((((long double)(nowWait-busyWait))/CLOCKS_PER_SEC)<3)
		{/*Busy Wait of extreme sadness*/
			//printf("Wait time: %Lf \n",((long double)(nowWait-busyWait))/CLOCKS_PER_SEC);
			nowWait = clock();

		}

		timeDifferences.push_back((long double)(carTimeEnters-carTimeLoaded));


		//On Pop, we increment the daily load counter
		carsThrough++;
	}

	//We need to perform statistics, such as max, min, mean, and median. Easier to do if sorted.
	sort(timeDifferences.begin(),timeDifferences.end());

	long double mean=0;//initialize

	long double median=0;
	if(timeDifferences.size()%2 == 0)//If 2|timeDifferences.size()
		median = (timeDifferences[timeDifferences.size()/2] + timeDifferences[timeDifferences.size()/2-1]) /2;
	else							 //If 2|timeDifferences.size()+1
		median = timeDifferences[timeDifferences.size()/2]; //Abuse of cast to some extent

	long double min=timeDifferences[0];
	long double max=timeDifferences[timeDifferences.size()-1];

	for(int current = 0; current<timeDifferences.size(); current++)
	{
		mean+=timeDifferences[current];
	}
	mean/=timeDifferences.size();

	return statistics(mean,median,min,max);
}

void *Sensor(argument Load)
{
	double checksum=0;
	for(int i=0; i<Load.size; i++)
	{
		if(Load.contents[i]==-1)
			break;
		checksum+=Load.contents[i];
	}
	printf("I've a load in my pocket, CheckSum = %G\n",checksum);

	//Retrieve the current time t.
	clock_t t; t=clock();//Measured in Clocks

	//Iterate through all cars.
	for (int i=0;  i<Load.size;i++)
	{

		//SAMPLE: printf("Wait time: %Lf \n",((long double)(nowWait-busyWait))/CLOCKS_PER_SEC);
		//At each car, get the current time, and wait for the current time + the car's double value
		clock_t nowTime = clock();
		while(nowTime<t+Load.contents[i]*CLOCKS_PER_SEC)//NOTE MEASURED IN CLOCKS: While current Time is less than the time to launch, wait.
		{nowTime = clock();}

		//When the car's time has come, push it to the appropriate CarQueues2[direction] with the current time
		//We push said current time in order to get the statistics for later.

		pthread_mutex_lock( &sensorLock );
		pthread_mutex_lock( &headLock );

		//On push, we need to check if(!headOfTraffic[direction]). If that is true, we need to assign it the next largest value of the values specified.
		if(!headOfTraffic[Load.direction]) //Head of Traffic is 0
		{
			int max = 0; //We set max to be the value 1 above the maximum value. This tells us when it will be our turn to go.
			for(int j=0; j<headOfTraffic.size();j++)
			{
				if(headOfTraffic[j]>=max)
					max=headOfTraffic[j]+1;
			}
			headOfTraffic[Load.direction]=max;
		}

		//Get an accurate time read
		nowTime = clock();
		carQueues[Load.direction].push(nowTime); //NOTE, WE ARE SPECIFICALLY PASSING CLOCKS

		pthread_mutex_unlock( &headLock );
		pthread_mutex_unlock( &sensorLock );
		printf("In you go, Direction: %d, time of arrival: %Lf! \n", Load.direction,(long double) nowTime/CLOCKS_PER_SEC);
		//Refresh the time t, so that the next car launches at the correct time.
		t=clock();
		//Once all cars have been pushed (signified by a -1) we break and call it a day for this function.
		if(i+1==Load.size||Load.contents[i+1]==-1)
			break;
	}
	return NULL;
}
