Author: Brennen Fagan.
Class: CSCI 144: Operating Systems.
Instructor: Dr. Ming Li.
Date of version: 2015-12-10.
University: California State University, Fresno.
Hosting Service: GitLab.
Compiler: Eclipse Mars.
Language: C++11.

Files:	README.md (This file.)
	CSCI144_Report.doc (Details of creation and detailed walkthrough of program.)
	CSCI144_Project (Executes.)
	/src/CSCI144_Project.cpp (Main simulation and wrapping.)
	/src/stopsign.h (Some linking declarations and definitions, as well as includes.)
	/src/stopsign.cpp (Initial implementation under the Stop Sign rule.)

Run Details: While in the directory use the command
	./CSCI144_Project
	You will be prompted to provide 4 inputs on load:
	1. Maximum time to produce cars for.
	2. Number of directions that cars will be coming from.
	3. Whether you wish to run just the stop sign simulation, just the traffic light simulation, or both with a comparison at the end.
	4. The mean value for the exponential distribution employed.
	At any time during setup, enter a 0 in a field to exit. Alternatively, you may use Ctrl+C to force a quit at any time.

Known Issues:
	1. If we use a large number of lanes, the traffic light function busy spins. This occurs due to not all cars arriving in the finished vector: carsPastInterSection. It has been noted that the other threads have stopped at this time, so hypothetically all cars have been loaded into the traffic.
		Possibly Isolated it: the head of line is not accurately keeping track of if there are cars still.
	2. In the time it takes for the TrafficLight or Sign thread to reclaim the lock, it appears that the threads have managed to put all of their cars into the queues. I am unsure as to why this happens as consistently as it does and am half-inclined to believe that it is a result of my specific computer.
