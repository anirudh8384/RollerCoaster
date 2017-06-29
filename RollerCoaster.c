#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<unistd.h>
#include <semaphore.h>

#define NUM_PASS 200
#define SEATS    4
#define TRIPS     3
#define Car_NumbeR     3
typedef sem_t Semaphore;

typedef struct
{
  int boarders, unboarders;
  Semaphore *mutex1, *mutex2;                               /*mutex protects passengers, which counts the number of passengers that have invoked boardCar or unboardcar.*/
  Semaphore *boardQueue, *unboardQueue;       		/*Passengers wait on boardQueue before boarding and unboardQueue before unboarding..*/
  Semaphore *allAboard, *allAshore;			         /* allAboard indicates that the car is full, allAshore indicates that car is empty.*/
  Semaphore *loadingarea, *unloadingarea;
} Shared;

unsigned int id_counter;

void error (char *msg)
{
  perror (msg);
  exit (1);
}


void *check_malloc (int size)
{
  void *p = malloc (size);
  if (p == NULL)
    error ("malloc failed\n");
  return p;
}


Semaphore *new_semaphore (int n)
{
  Semaphore *sem = check_malloc (sizeof (Semaphore));
  int r = sem_init (sem, 0, n);
  if (r == -1)
    error ("sem_init failed\n");
  return sem;
}


void join_thread (pthread_t thread)
{
  int r = pthread_join (thread, NULL);
  if (r == -1)
    error ("pthread_join failed\n");
}


pthread_t new_thread (void *(entry)(void *), Shared *shared)
{
  int ret;
  pthread_t thread;

  ret = pthread_create (&thread, NULL, entry, (void *) shared);
  if (ret != 0)
    error ("pthread_create failed\n");
  return thread;
}



Shared * new_shared ()
{
  Shared *p = check_malloc (sizeof (Shared));
  p->mutex1 = new_semaphore (1);
  p->mutex2 = new_semaphore (1);
  p->boardQueue = new_semaphore (0);
  p->unboardQueue = new_semaphore (0);
  p->allAboard = new_semaphore (0);
  p->allAshore    = new_semaphore (0);
  p->loadingarea = new_semaphore (1);
  p->unloadingarea = new_semaphore (1);  
  p->boarders =0;
  p->unboarders = 0;
}

/*Passengers wait for the car before boarding, naturally, and wait for the car
to stop before leaving. The last passenger to board signals the car  and resets
the passenger id-counter.*/


void *passenger (void *argument)
{
  int id = id_counter++;
  Shared *shared = (Shared *) argument;

  sem_wait (shared->boardQueue);
  printf ("Statement Passenger: Passenger %d is boarding\n", id);

  sem_wait (shared->mutex1);
  shared->boarders++;
  if (shared->boarders == SEATS)
    {
      sem_post (shared->allAboard);
      shared->boarders = 0;
    }
  sem_post (shared->mutex1);
  
  sem_wait (shared->unboardQueue);
  printf ("Statement Passenger: Passenger %d is unboarding\n", id);
  
  sem_wait (shared->mutex2);
  shared->unboarders++;
  if (shared->unboarders == SEATS)
    {
      sem_post (shared->allAshore);
      shared->unboarders = 0;
    }
  sem_post (shared->mutex2);
	 
  pthread_exit (NULL);
}

/*When the car arrives, it signals C passengers, then waits for the last one to
signal allAboard. After it departs, it allows C passengers to disembark, then
waits for allAshore.*/

void * rollercoaster (void *argument)
{
  int id = id_counter++;
  int i, trips = 1;
  int car_number=1;
  Shared *shared = (Shared *) argument;
  for (trips = 1; trips <= TRIPS; trips++){
	printf ("\nStatement RollerCoaster Car: This is TRIP number %d\n", trips);
	for( car_number = 1 ; car_number <= Car_NumbeR ; car_number++ ){
	printf ("\nStatement RollerCoaster Car: This is Car number %d\n", car_number );
   		sem_wait(shared->loadingarea);
      
      printf ("Statement RollerCoaster Car: Loading passengers...\n");


      for (i = 0; i < SEATS; i++)
	sem_post (shared->boardQueue);
      sem_wait (shared->allAboard);
      sem_post(shared->loadingarea);
      sleep(2);
      printf ("Statement RollerCoaster Car: Roller coaster car %d Loaded\n",car_number);
      
      }
      printf ("Statement RollerCoaster Car: Roller coaster on Track\n");
      
      sleep(8);
      printf ("Statement RollerCoaster Car: Roller coaster stopping\n");
      for(car_number = 1;car_number <= Car_NumbeR; car_number++){
      
      printf ("Statement RollerCoaster Car: Roller Coaster car %d is now Unloading passengers\n",car_number);
         		sem_wait(shared->unloadingarea);

      

      sleep(3);

      for (i = 0; i < SEATS; i++)
	sem_post (shared->unboardQueue);
      sem_wait (shared->allAshore);
       sem_post(shared->unloadingarea);
    }
}
  printf ("\nStatement RollerCoaster Car: Ride is closed!! Come Back Tomorrow\n");
  pthread_exit (NULL);
}


int main ()
{
  int i;
  pthread_t passengers[NUM_PASS];
  pthread_t roller_coaster;
  
  id_counter     = 0;
  Shared *shared = new_shared ();

  roller_coaster = new_thread (rollercoaster, shared);

  for (i = 0; i < NUM_PASS; i++)
    new_thread (passenger, shared);

  join_thread (roller_coaster);

  return 0;
}


