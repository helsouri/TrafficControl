#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "common.h"
#include <semaphore.h>
#define totalCarNumb 8          // Total number of cars we have
#define STRAIGHT false          // Definition for ease of use
#define LEFT true               // Definition for ease of use
#define oneHunderdK 100000
bool crossHolder[4];            // Flag for whether an entrance. exit is taken
int crossing[4]= {0,0,0,0};	    // Counter for how many cars are in one "node"

pthread_mutex_t turnLock[4];	// Lock for the directions( to prevent a car from traveling into a prohibited turn)
pthread_mutex_t updateLock[4];  // Lock for updating any locks that were acquired during  crossing

sem_t enterSem[4];				// To see if a car can enter into a certain direction
sem_t exitSem;					// To Ensure cars remain in order
sem_t printSem;                 // To ensure proper printing

int elapsedTime=0;			    // Time value (for incrementing by the wait function that also tracks elapsed time)
int actualTime=0;               // This is to ensure proper time value is printed when a car crosses will be updated only as needed

typedef struct _carInfo
// This is the data structure that contains information about the car
{
    char originalDir;
    char targetDir;
    int cid;
    int delay;
} carInfo;

carInfo newCar[8];

void delay1(int howLong)
// Main delay function used in main [similar to a primary timer]
{
    int i;
    for(i=0;i<howLong;i++)
    {
        usleep(oneHunderdK);
        elapsedTime+=1;
        actualTime=elapsedTime;
    }
}

void delay2(int howLong, int *cur)
// For cars turning
{
    int i;

    for(i=0;i<howLong;i++)
        usleep(oneHunderdK);
    *cur+=howLong;
}

int dirToInt (char tempDir)
// Function to convert character directions to integers since its more convenient to match numbers than character sets
{
    if(tempDir=='N')
        return 0;
    else if(tempDir=='S')
        return 2;
    else if(tempDir=='E')
        return 1;
    else
        return 3;
}

bool isRightTurn(int tempS, int tempD)
// Small function that determines if  car is turning right
{
    int dir=tempS-tempD;
    if(dir==3 || dir==-1)
        return true;
    else
        return false;
}

void arriveAtIntersection(carInfo *tempCar)
// Function called when a car arrives
{
    int source=dirToInt(tempCar->originalDir);       // Where car coming from
                                                    // Note here that the data provided is a bit confusing cars coming from south heading north
                                                    // are actually marked as coming from north and continue heading north so we have some opposition
                                                    // that we need to handle
    int destination=dirToInt(tempCar->targetDir);    // Where car is heading
    int val;                                        // local variable to help check locks and keep atomic behavior
    sem_wait(&printSem);
    printf("Time %d.%d: Car%d arriving from %c heading to %c \n",(int)actualTime/10, actualTime%10,tempCar->cid, tempCar->originalDir, tempCar->targetDir);
    sem_post(&printSem);
    sem_wait(&exitSem); //wait for previous car to cross and thus retaining order
    if(source==destination)
    // Car is going forward
    {
        pthread_mutex_lock(&updateLock[destination]); // acquire appropriate node for destination
        if(crossHolder[destination]==LEFT || crossing[destination]==0)
        {
            pthread_mutex_unlock(&updateLock[destination]);     // Release lock to avoid deadlock
            pthread_mutex_lock(&turnLock[destination]);         // Acquire lock for this path
            pthread_mutex_lock(&updateLock[destination]);       // Reclaim Lock
            crossHolder[destination]=STRAIGHT;                  // Set this flag
        }
        val=crossing[destination]++;                            // Increase car cont in that "node"
        pthread_mutex_unlock(&updateLock[destination]);         // don't need lock anymore since we updated all variables
        if(val<1)
        {
            sem_wait(&enterSem[destination]);
        }
    }
    else if(isRightTurn(source, destination))
    // Turning right
    {
        sem_wait(&enterSem[destination]);                       // Wait for no traffic then cross
        crossing[destination]++;                                // Increase traffic counter
    }
    else
    // Turning left
    {
        pthread_mutex_lock(&updateLock[(source+2)%4]);          // We need to set the source as turning left
                                                                // Blocking crossing cars from their exit
        if(crossHolder[(source+2)%4]==STRAIGHT || crossing[(source+2)%4]==0)
        // if "oppsite node" is marked
        {
            pthread_mutex_unlock(&updateLock[(source+2)%4]);    // Release lock while waiting
            pthread_mutex_lock(&turnLock[(source+2)%4]);        // Claim lock needed to turn
            pthread_mutex_lock(&updateLock[(source+2)%4]);      // Reclaim update lock to ensure atomic operation
            crossHolder[(source+2)%4]=LEFT;                     // Set this car to be turning left
        }
        val=crossing[(source+2)%4]++;                           // Increment crossing count ( remember sem is only signaled once all cars leave)
        if(val<1)
        {
            sem_wait(&enterSem[destination]);
        }
    }
    sem_post(&exitSem);                                         // Tell next car to search for chance to cross
    tempCar->delay=actualTime;                                   // update time to ensure correct value is printed
}

void crossingIntersection( carInfo *tempCar)
// Function of cars that are crossing
{
    sem_wait(&printSem);
    printf("Time %d.%d: Car%d crossing from %c to %c",(int)actualTime/10, actualTime%10,tempCar->cid, tempCar->originalDir, tempCar->targetDir );
    int source=dirToInt(tempCar->originalDir);
    int destination=dirToInt(tempCar->targetDir);
    if(source==destination)
    // Print a car is traveling straight and wait for 2 seconds
    {
        printf(" straight\n");
        sem_post(&printSem);
        delay2(20, &tempCar->delay);
    }
    else if(isRightTurn(source,destination))
    // If car is travelling  right print and wait for 1s
    {
        printf(" right\n");
        sem_post(&printSem);
        delay2(10, &tempCar->delay);
    }
    else
    // For traveling lift print it and wait for 3s
    {
        printf(" left\n");
        sem_post(&printSem);
        delay2(30, &tempCar->delay);
    }
    actualTime=tempCar->delay;  // update actual time so that next car gets actual time delta
}

void exitIntersection(carInfo *tempCar)
// Function for cars that are exiting
{
    int source=dirToInt(tempCar->originalDir);
    int destination=dirToInt(tempCar->targetDir);
    printf("Time %d.%d: Car%d exited\n", (int)tempCar->delay/10, tempCar->delay%10, tempCar->cid);
    if(source==destination)
    // If a car is going straight
    {
        pthread_mutex_lock(&updateLock[destination]);           // Grab lock for changing
        crossing[destination]--;                                // Decrease Counter for traffic
        if(crossing[destination]==0)
        // If no cars remain
        {
            pthread_mutex_unlock(&turnLock[destination]);
            sem_post(&enterSem[destination]);
        }
        pthread_mutex_unlock(&updateLock[destination]);
    }
    else if(isRightTurn(source,destination))
    // Turning right
    {
        pthread_mutex_lock(&updateLock[destination]);           // Grab lock for changes
        crossing[destination]--;                                // Decrease counter for traffic
        if(crossing[destination]==0)
        {
            sem_post(&enterSem[destination]);
        }
        pthread_mutex_unlock(&updateLock[destination]);
    }
    else
    // turning left
    {
        pthread_mutex_lock(&updateLock[(source+2)%4]);               // Grab lock for atomic operations
        crossing[(source+2)%4]--;                                    // Decrease traffic
        if(crossing[(source+2)%4]==0)
        // no cars left
        {
            pthread_mutex_unlock(&turnLock[(source+2)%4]);           // Release lock for turning so cars going straight can pass
            sem_post(&enterSem[destination]);                        // release destination for next car
        }
        pthread_mutex_unlock(&updateLock[(source+2)%4]);             // Release lock for changing
    }
}

void *startCrossing(void *args)
//function being called by the threads to start crossing
{
    carInfo *tempCar = (carInfo *) args ;                       // Reconstructing data from the argument that was passed
    arriveAtIntersection(tempCar);                              // wait for intersection to be free
    crossingIntersection(tempCar);                              // cross with delays
    exitIntersection(tempCar);                                  // free intersection for next car
    pthread_exit(NULL);                                         // Exit thread (work is done)
}

int main()
{
    // Initiating Threads
    int delays[totalCarNumb]={11,9,13,2,7,2,13,2};                              // Delta delays for the wait between cars
    // consider that car 0 arrives at 1.1(1.1*10=11) and car1 arrives at 2[(2-1.1]*10=9] ....
    char originalDirData[totalCarNumb]={'N','N','N','S','S','N','E','W'};      // The data for original direction
    char targetDirData[totalCarNumb]={'N','N','W','S','E','N','N','N'};        // The data for target direction
    // Defining threads and attr
    pthread_t threads[totalCarNumb];                                           // An Array of 8 threads
    pthread_attr_t attr;                                                       // An attribute called attr
    // Initiating attr and setting threads with that attribute to be joinable
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int i;
    // Initializing mutexs and semaphores
    for(i=0;i<4;i++)
    {
        pthread_mutex_init(&updateLock[i],NULL);
		pthread_mutex_init(&turnLock[i],NULL);
		sem_init(&enterSem[i],0,1);
    }
    sem_init(&exitSem,0,1);
	sem_init(&printSem,0,1);
	for(i=0;i<totalCarNumb;i++)
    {
        delay1(delays[i]);
        newCar[i].delay=actualTime;
        newCar[i].originalDir=originalDirData[i];
        newCar[i].targetDir=targetDirData[i];
        newCar[i].cid=i;
        pthread_create(&threads[i], &attr, startCrossing, (void *) &newCar[i]);
    }
    for (int i=0; i<totalCarNumb;i++);
        pthread_join(threads[i], NULL);
    // Clean up [may not be needed since process will exit and do that automatically
    /*pthread_attr_destroy(&attr);
    for(i=0;i<4;i++)
    {
        pthread_mutex_destroy(&updateLock[i]);
        pthread_mutex_destroy(&turnLock[i]);
        sem_destroy(&enterSem[i]);
    }
    sem_destroy(&exitSem);
    sem_destroy(&printSem);*/
    pthread_exit(NULL);
    return 0;
}
