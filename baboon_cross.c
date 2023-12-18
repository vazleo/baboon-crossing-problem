//implementation of baboon crossing problem without semaphores

/*
                    BABOON CROSSING PROBLEM

This problem is adapted from Tanenbaum’s Operating Systems: Design and
Implementation [12]. There is a deep canyon somewhere in Kruger National
Park, South Africa, and a single rope that spans the canyon. Baboons can cross
the canyon by swinging hand-over-hand on the rope, but if two baboons going in
opposite directions meet in the middle, they will fight and drop to their deaths.
Furthermore, the rope is only strong enough to hold 5 baboons. If there are
more baboons on the rope at the same time, it will break.

Assuming that we can teach the baboons to use semaphores, we would like
to design a synchronization scheme with the following properties:

    • Once a baboon has begun to cross, it is guaranteed to get to the other
    side without running into a baboon going the other way.

    • There are never more than 5 baboons on the rope.

    • A continuing stream of baboons crossing in one direction should not bar
    baboons going the other way indefinitely (no starvation).

*/

//DISCLAIMER: implementing an algorithm without semaphores

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_BABOONS 5
#define MAX_CROSSING_TIME 3

//global variables
int num_baboons = 0; //number of baboons
int direction = 0; //0 for left, 1 for right
int baboons_on_rope = 0; //number of baboons crossing

//mutexes
pthread_mutex_t direction_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex for direction
pthread_mutex_t baboons_on_rope_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex for baboons_on_rope

//condition variables
pthread_cond_t left_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t right_cond = PTHREAD_COND_INITIALIZER;

//function prototypes
void *baboon(void *arg);

int main(int argc, char *argv[])
{
    srand(time(NULL)); //seed random number generator

    //check for correct number of arguments
    if(argc != 2)
    {
        printf("Usage: ./baboon_cross <number of baboons>\n");
        exit(1);
    }

    //get number of baboons from command line
    num_baboons = atoi(argv[1]);
    printf("Number of baboons: %d\n", num_baboons);

    //create threads
    pthread_t baboon_threads[num_baboons];
    for(int i = 0; i < num_baboons; i++)
    {
        int* baboon_id = malloc(sizeof(int));
        *baboon_id = i;
        pthread_create(&baboon_threads[i], NULL, baboon, baboon_id);
    }

    //join threads
    for(int i = 0; i < num_baboons; i++)
    {
        pthread_join(baboon_threads[i], NULL);
    }

    printf("All baboons have crossed the canyon.\n"); 

    return 0;  //exit program
}

void *baboon(void *arg)
{
    //get baboon id
    int baboon_id = *(int *)arg;

    //get direction
    int baboon_direction = rand() % 2; //get random number between 0 and 1 - 0 for left, 1 for right

    printf("Baboon %d wants to cross the canyon in direction %s\n", baboon_id, baboon_direction ? "RIGHT" : "LEFT");

    //get on rope
    pthread_mutex_lock(&baboons_on_rope_mutex); 
    while(baboons_on_rope >= MAX_BABOONS || baboon_direction != direction) 
    {
        if(baboon_direction == 0)           //if baboon is going left, wait for left_cond
            pthread_cond_wait(&left_cond, &baboons_on_rope_mutex);
        else                                //if baboon is going right, wait for right_cond
            pthread_cond_wait(&right_cond, &baboons_on_rope_mutex);
    }
    baboons_on_rope++;
    pthread_mutex_unlock(&baboons_on_rope_mutex);

    //cross rope
    printf("Baboon %d is crossing the rope in direction %d\n", baboon_id, baboon_direction);
    sleep(rand() % MAX_CROSSING_TIME);
    printf("Baboon %d has crossed the rope in direction %d\n", baboon_id, baboon_direction);
    //get off rope
    pthread_mutex_lock(&baboons_on_rope_mutex);
    baboons_on_rope--;

    if(baboons_on_rope == 0) //if no baboons are crossing signal (broadcast) baboons from opposite direction
    //obs: i could have implemented a timeout system, but just waiting for rope to be empty is simpler and seems to work
    {
        if(direction == 0)
        {
            pthread_mutex_lock(&direction_mutex);   // (?) do i need to lock this mutex? considering that i am already inside a mutex
            direction = 1;
            //pthread_cond_signal(&right_cond);         (?) do i need to verify if there is only one baboon waiting? to use this instead of broadcast
            pthread_cond_broadcast(&right_cond);
            pthread_mutex_unlock(&direction_mutex);
        }
        else
        {
            pthread_mutex_lock(&direction_mutex);
            direction = 0;
            //pthread_cond_signal(&left_cond);     
            pthread_cond_broadcast(&left_cond);
            pthread_mutex_unlock(&direction_mutex);
        }
    }
    
    pthread_mutex_unlock(&baboons_on_rope_mutex); // (?) could i put this before the if statement? 

    pthread_exit(NULL);
}


