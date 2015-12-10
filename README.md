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
	1. Sometimes the traffic light function busy spins. This occurs due to not all cars arriving in the finished vector: carsPastInterSection. Cause still undetermined. It has been noted that the other threads appear to have stopped at this time.
	2. The computer scheduler schedules the TrafficLight and Sign functions last, despite initializing them first. As such, all cars end up loaded before these functions really begin running. Solutions welcome!
