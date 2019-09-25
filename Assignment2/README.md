--------------------------
* University of Victoria
* CSC360  Assignment2    
* Jue Fu                 
* V00863998              
--------------------------  

--------------------------------------------------------------------------

Build and run in linux system:

- Simply input 'make' in terminal to compile.
- To run the executable, input './ACS <test file name>' in the terminal.

--------------------------------------------------------------------------

Usage:

This program implements a task scheduler using the following concepts:

- thread
- mutex
- condition variable (convar)

The task scheduler simulates an airline check-in system, called ACS.

The check-in system includes 2 queues and 4 clerks.  One queue  for business class and the other

for economy class. The program reads data from the test files and give the corresponding messages about customers and serving conditions.
The program also calculates

- The average waiting time for all customers in the system
- The average waiting time for all business-class customers
- The average waiting time for all economy-class customers
