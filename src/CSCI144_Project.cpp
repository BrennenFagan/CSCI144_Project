//============================================================================
// Name        : CSCI144_Project.cpp
// Author      : Brennen Fagan
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

using namespace std;


//Functions and Arguments
statistics TrafficLight(int DailyLoad);//Based on stopsign.cpp's Sign
void *Sensor(argument Load); //Based on Direction, but modifying different variables and locks
statistics WRAPPER(int numDirections, double simulationLength, double** workLoad);// To make implementation somewhat more modular.


//Global Variables
vector<int> headOfTraffic; //Stores the positions of the directions. If (0) then there are no cars. Otherwise, 1, 1st ... n, nth in line.
vector<queue<clock_t> > carQueues; //place to store ALL THE CARS, sorted by direction, and storing the cars' arrival times.
vector<long double> carsPastIntersection; //analogous to a more broadly used timeDifferences in stopsign.cpp


//Locks
pthread_mutex_t sensorLock = PTHREAD_MUTEX_INITIALIZER;//carQueues
mutex headLock;//headOfTraffic
pthread_mutex_t resultLock = PTHREAD_MUTEX_INITIALIZER; //carsPastIntersection

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
	//End Workload Creation//////////////////////////////////////////////////////////////////////////////////////////////////

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
		//Note that the stats from stopsign do not include the time to cross the intersection.
		//Hence, we add 3 to any stats we want to compare between the two.
		stopSignResults.mean+=3;
		stopSignResults.median+=3;
		stopSignResults.min+=3;
		stopSignResults.max+=3;

		statistics Results = WRAPPER(numDirections, simulationLength, workLoad);

		printf("|------------------------------------------------------------------------|\n");
		printf("| Metric |     Mean      |    Median     |      Min      |      Max      |\n");
		printf("| Sign   |%Lf|%Lf|%Lf|%Lf|\n",stopSignResults.mean,stopSignResults.median,stopSignResults.min,stopSignResults.max);
		printf("| Light  |%Lf|%Lf|%Lf|%Lf|\n",Results.mean,Results.median,Results.min,Results.max);
		printf("|------------------------------------------------------------------------|\n");
	}

	return 0;
}

statistics WRAPPER(int numDirections, double simulationLength, double** workLoad)
{
	//Reset all global variables//////////////////////////////////////////////////////////

			headLock.lock();
			headOfTraffic.resize(numDirections);
			headLock.unlock();

			pthread_mutex_lock( &sensorLock );
			//Empty existing queues, followed by resizing for appropriate length
			carQueues.resize(numDirections);
			//For safety, remove anything within the size.
			for(int direction = 0; direction<numDirections;direction++)
			{
				while(!carQueues[direction].empty())
					carQueues[direction].pop();
			}
			pthread_mutex_unlock( &sensorLock );

			pthread_mutex_lock( &resultLock );
			carsPastIntersection={};
			pthread_mutex_unlock( &resultLock );
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

			//Join and collect Results//////////////////////////////////////////////////////////
			for (int direction = 0; direction<numDirections;direction++)
			{
				threads[direction].join();
			}

			return signalReturn.get();
			//Returning results of Sim//////////////////////////////////////////////////////////
}

statistics TrafficLight(int DailyLoad) //of TimeandDirection class
{
	bool allCarsThrough = false;
		//This function monitors the carQueues, while it waits for the dailyLoad to be done.
		while(!allCarsThrough)
		{
			//Determine if Cars waiting at the intersection////////////////////////////////////////////////////////////
			int anyoneWaiting=-1;
			headLock.lock(); //Request Permission to access HeadOfTraffic
			for (int direction = 0; direction < headOfTraffic.size(); direction++)
			{
				if(headOfTraffic[direction]&&											//If there is anyone waiting in any lane
						(anyoneWaiting==-1 || 											//AND we have not Detected anyone OR
								headOfTraffic[direction]<headOfTraffic[anyoneWaiting]))	//This someone has higher (<current) priority
					anyoneWaiting=direction;											//Assign our direction of interest to them.
			}
			headLock.unlock();
			if(anyoneWaiting==-1)
				{
					//Note, may be inefficient to acquire locks, but it is as safe or safer than not doing so.
					pthread_mutex_lock( &resultLock );
					//If noone is waiting, we need to check to see if we are done.
					//All Cars are Through if the number of cars through is greater than or equal to the number of cars expected
					int numPastIntersection=carsPastIntersection.size();
					allCarsThrough=(numPastIntersection>=DailyLoad);
					pthread_mutex_unlock( &resultLock );

					if(allCarsThrough) break;
					else continue; //If we couldn't find anyone, try again.
				}
			//Done Determining if Cars waiting//////////////////////////////////////////////////////////////////////////

			//Time to Load Cars through the intersection////////////////////////////////////////////////////////////////
			//We now know that someone is waiting.
			//We acquire locks to remove people form the lanes and get them through the intersection.
			//Acquire in the same order as sensor
			pthread_mutex_lock( &sensorLock );
			headLock.lock();

			//I think the timing logic is right, but I am unsure.

			//We wish to release all cars who could make it through the intersection.
			//By time, that means that they have less than
			clock_t closeTime = clock();
			//Plus three seconds to get through the intersection:
			closeTime+=3*CLOCKS_PER_SEC;
			//Plus the time to get the last car through the intersection:
			closeTime+=(50/27+40.5/27)*CLOCKS_PER_SEC;
			//So that the car in front is guaranteed to get through as well as a car just arriving to trigger the sensor.
			//Any cars in between also get serviced.
			//This is the time that we will maintain the locks for.

			//Now we need to transfer any cars
			for(int cars = 0; cars<carQueues[anyoneWaiting].size(); cars++)
			{
				//We check the time of the cars and if they would get through in time.
				//Recall that values in the Queues of carQueues are in Clocks.
				if(carQueues[anyoneWaiting].front()<closeTime)
				{
					clock_t carTimeLoaded =
							carQueues[anyoneWaiting].front();
							carQueues[anyoneWaiting].pop();

							//Acquire the Results lock and push the closingTime-TimeLoaded
							pthread_mutex_lock( &resultLock );
							carsPastIntersection.push_back((long double)(closeTime-carTimeLoaded));
							pthread_mutex_unlock( &resultLock );
							printf("Traffic Light: Release time: %Lf!\n",(long double) closeTime/CLOCKS_PER_SEC);
				}
			}

			//If there is an opposite direction, i.e. 2|numDirections i.e. there are an even number of directions
			int oppositeDirection = -1;
			if(carQueues.size()%2==0)
			{
				//We check the opposite direction
				//If anyoneWaiting>half of size, we subtract,
				//If anyoneWaiting<half of size, we add
				//In a four way, 0 and 2, 1 and 3
				//In a six way, 0 and 3, 1 and 4, 2 and 5
				if(anyoneWaiting>=carQueues.size()/2)
					oppositeDirection = anyoneWaiting-carQueues.size()/2;
				else
					oppositeDirection = anyoneWaiting+carQueues.size()/2;
				for(int cars = 0; cars<carQueues[oppositeDirection].size(); cars++)
				{
					//We check the time of the cars and if they would get through in time.
					//Recall that values in the Queues of carQueues are in Clocks.
					if(carQueues[oppositeDirection].front()<closeTime)
					{
						clock_t carTimeLoaded =
								carQueues[oppositeDirection].front();
								carQueues[oppositeDirection].pop();

								//Acquire the Results lock and push the closingTime-TimeLoaded
								pthread_mutex_lock( &resultLock );
								carsPastIntersection.push_back((long double)(closeTime-carTimeLoaded));
								pthread_mutex_unlock( &resultLock );
								printf("Traffic Light: Release time: %Lf!\n",(long double) closeTime/CLOCKS_PER_SEC);
					}
				}
			}
			//End loading cars through the intersection///////////////////////////////////////////////////////////////

			//Update all Lanes////////////////////////////////////////////////////////////////////////////////////////
			//Almost direct copy from stopsign.cpp's Sign function.
			for(int direction = 0; direction<headOfTraffic.size();direction++)
					{
						if(direction==anyoneWaiting||direction==oppositeDirection)//If this is the lane we popped from
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
			//All Lanes should be updated/////////////////////////////////////////////////////////////////////////////

			//Finish the simulation portion of the timing/////////////////////////////////////////////////////////////
			//Now we need to ensure that the other threads wait for the cars to actually get through the intersection.
			//We don't need to wait for the time that we have already waited, so we only check to go the remainder.
			clock_t nowWait = clock();
			while( ( (long double) (closeTime-nowWait) ) >0)//If the time left to wait is greater than the current time, continue to wait.
			{/*Busy Wait of extreme sadness*/
				nowWait = clock();
			}

			headLock.unlock();
			pthread_mutex_unlock( &sensorLock );
		}

	//Statistics and Results Section////////////////////////////////////////////////////////////
		//We need to perform statistics, such as max, min, mean, and median. Easier to do if sorted.

		pthread_mutex_lock( &resultLock );
		sort(carsPastIntersection.begin(),carsPastIntersection.end());

		long double mean=0;//initialize

		long double median=0;
		if(carsPastIntersection.size()%2 == 0)//If 2|carsPastIntersection.size()
			median = (carsPastIntersection[carsPastIntersection.size()/2] + carsPastIntersection[carsPastIntersection.size()/2-1]) /2;
		else							 //If 2|carsPastIntersection.size()+1
			median = carsPastIntersection[carsPastIntersection.size()/2]; //Abuse of cast to some extent

		long double min=carsPastIntersection[0];
		long double max=carsPastIntersection[carsPastIntersection.size()-1];

		for(int current = 0; current<carsPastIntersection.size(); current++)
		{
			mean+=carsPastIntersection[current];
			long double car =
					carsPastIntersection.back();
			printf("Car: Response time: %Lf \n", car);
		}
		mean/=carsPastIntersection.size();
		pthread_mutex_unlock( &resultLock );
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
	//printf("I've a load in my pocket, CheckSum = %G\n",checksum);

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

		//When the car's time has come, push it to the appropriate CarQueues[direction] with the current time
		//We push said current time in order to get the statistics for later.

		//On push, we need to check if(!headOfTraffic[direction]). If that is true, we need to assign it the next largest value of the values specified.
		int max = 0; //We set max to be the value 1 above the maximum value. This tells us when it will be our turn to go.

		if(headLock.try_lock())	//If the headOfTraffic is locked, someone is in the intersection and we need to add ourselves to the list of people to wait.
		{						//If the headOfTraffic is not locked, there is noone in the intersection and we need to check if we need to wait our turn.
			if(!headOfTraffic[Load.direction]) //Head of Traffic is 0, meaning noone is waiting in our direction
					{
						//Check the other directions
						for(int j=0; j<headOfTraffic.size();j++)
						{
							if (headOfTraffic[j]==0)
								;
							//if someone is waiting there, we now need to wait until after they go.
							else if(headOfTraffic[j]>=1&&headOfTraffic[j]>=max)
								max=headOfTraffic[j]+1;
						}
						//If we're the first in line and another line has people waiting in it, then we add ourselves behind them.
						if(max)
							headOfTraffic[Load.direction]=max;
					}
			//If noone is waiting elsewhere
			if(max==0)
					{
						printf("Traffic Sensor: Direction: %d, time of GO: %Lf! \n", Load.direction,(long double) nowTime/CLOCKS_PER_SEC);
						//BusyWait for the Car to go through the intersection AT SPEED. 40.5/27 = 1.5
						clock_t nowWait = clock();
						while( (long double) (nowWait<1.5*CLOCKS_PER_SEC+nowTime) )
						{/*Busy Wait of extreme sadness*/
							//printf("Wait time: %Lf \n",((long double)(nowWait-busyWait))/CLOCKS_PER_SEC);
							nowWait = clock();
						}

						//And Load into the results. Remember that this specifically holds the time leaving the intersection - the time of beginning wait.
						pthread_mutex_lock( &resultLock );
						carsPastIntersection.push_back((long double)(nowWait-nowTime));
						pthread_mutex_unlock( &resultLock );

					}
					else
					{
						//Get an accurate time read and pass to the TrafficLight to handle
						nowTime = clock();

						pthread_mutex_lock( &sensorLock );
						carQueues[Load.direction].push(nowTime); //NOTE, WE ARE SPECIFICALLY PASSING CLOCKS
						pthread_mutex_unlock( &sensorLock );

						printf("Traffic Sensor: Direction: %d, time of WAIT: %Lf! \n", Load.direction,(long double) nowTime/CLOCKS_PER_SEC);
					}
			headLock.unlock();
		}
		else
		{
			//Since we know we are waiting, our max is at least 1
			max=1;
			//We wait to retrieve the lock, and add our car to the waitlist.
			if(!headOfTraffic[Load.direction]) //Head of Traffic is 0
					{
						for(int j=0; j<headOfTraffic.size();j++)
						{
							if (headOfTraffic[j]==0)
								;
							else if(headOfTraffic[j]>=max)
								max=headOfTraffic[j]+1;
						}

						headLock.lock();
						headOfTraffic[Load.direction]=max;
						headLock.unlock();
					}

			//Get an accurate time read and pass to the TrafficLight to handle
			nowTime = clock();
			pthread_mutex_lock( &sensorLock );
			carQueues[Load.direction].push(nowTime); //NOTE, WE ARE SPECIFICALLY PASSING CLOCKS
			pthread_mutex_unlock( &sensorLock );

			printf("Traffic Sensor: Direction: %d, time of WAIT: %Lf! \n", Load.direction,(long double) nowTime/CLOCKS_PER_SEC);
		}


		//printf("In you go, Direction: %d, time of arrival: %Lf! \n", Load.direction,(long double) nowTime/CLOCKS_PER_SEC);
		//Refresh the time t, so that the next car launches at the correct time.
		t=clock();
		//Once all cars have been pushed (signified by a -1) we break and call it a day for this function.
		if(i+1==Load.size||Load.contents[i+1]==-1)
			break;
	}
	return NULL;
}
