 --------------------------
 * University of Victoria *
 * CSC360  Assignment1    *
 * Jue Fu                 *
 * V00863998              *
 --------------------------

--------------------------------------------------------------------------

Build and run in linux system:

1. Simply input 'make' in terminal to compile.
2. To run the executable, input './PMan' in the terminal.

--------------------------------------------------------------------------

Usage:

This prompt supports 6 commands:

1. bg <cmd> <argument1> <argument2> ... : starts <cmd> running in the background
2. bgkill <pid>: terminates the process using SIGTERM
3. bgstop <pid>: stops the process using SIGSTOP
4. bgstart <pid>: RESUME the process using SIGCONT
5. bglist: lists all processes
6. pstat <pid>: lists comm, state, utime, stime, rss, voluntary_ctxt_switches and nonvoluntary_ctxt_switches for the process with process id <pid>
