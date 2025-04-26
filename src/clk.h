#ifndef CLK_H
#define CLK_H

/*
 * This function is used to initialize the clock module.
 * It creates a shared memory segment and initializes the clock value to 0.
 */
void initClk();
/*
 * This function is used to run the clock module.
 * It increments the clock value every second.
 */
void runClk();
/*
 *This function is used to get the clock value from the shared memory
 */
int getClk();
/*
 * All process call this function at the beginning to establish communication between them and the
 * clock module. Again, remember that the clock is only emulation!
 */
void syncClk();
/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
 */
void destroyClk(short terminateAll);

#endif