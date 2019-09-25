/*
  University of Victoria
  Fall 2018
  CSC 360 Assignment 2
  Jue Fu
  V00863998
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#define BUSY 1
#define IDLE 0
#define MAX_INPUT_LEN 100
#define MAX_INPUT_NUM 256
#define CLERK_NUM 4
#define MAX_CUSTOMER_NUM 256
#define DECISEC_TO_MICROSEC 100000

pthread_mutex_t clerk_locks[CLERK_NUM];
pthread_mutex_t enQ_lock;
pthread_mutex_t deQ_lock;
pthread_mutex_t queue_lock;
pthread_mutex_t customer_lock;

pthread_cond_t convars[CLERK_NUM];

struct timeval INITIAL_TIME;

typedef struct customer{
  int id;
  int q_id;
  int arrv_time;
  int serv_time;
}customer;

typedef struct queue{
  customer waitline[MAX_INPUT_NUM];
  int length;
  int status;
}queue;

int all_time;
int business_time;
int economy_time;
int CUSTOMER_NUM;
int customer_num_left;
queue* economy;
queue* business;
double wait_time_list[MAX_CUSTOMER_NUM];

/*
    Wrapped function to do mutex lock with error handling
*/
void mutex_lock(pthread_mutex_t lock){
  if (pthread_mutex_lock(&lock) != 0) {
		printf("Failed to lock mutex\n");
		exit(1);
	}
}

/*
    Wrapped function to do mutex unlock with error handling
*/
void mutex_unlock(pthread_mutex_t lock){
  if (pthread_mutex_unlock(&lock) != 0) {
		printf("Failed to unlock mutex\n");
		exit(1);
	}
}

/*
    Deal with time difference, get difference between start time and current time
*/
float getRelativeTime() {
	struct timeval curr_time;
	gettimeofday(&curr_time, NULL);
  long init_in_MS = (INITIAL_TIME.tv_sec * 10 * DECISEC_TO_MICROSEC) + INITIAL_TIME.tv_usec;
	long curr_in_MS = (curr_time.tv_sec * 10 * DECISEC_TO_MICROSEC) + curr_time.tv_usec;
	return (float)(curr_in_MS - init_in_MS) / (10 * DECISEC_TO_MICROSEC);
}

/*
    Functions to implement struct queues
*/
void enqueue(customer c, queue* q){
  mutex_lock(enQ_lock);
  q->waitline[q->length++] = c;
  wait_time_list[c.id] = getRelativeTime();
  printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n",c.q_id,q->length);
  mutex_unlock(enQ_lock);
}/*enqueue*/

customer dequeue(queue* q){
  mutex_lock(deQ_lock);
  if(q->length==0){printf("%s\n","Queue empty, cannot dequeue.");exit(1);}
  customer c = q->waitline[0];
  int i=0;
  for(i=0;i<q->length-1;i++){
    q->waitline[i]=q->waitline[i+1];
  }
  q->length--;
  customer_num_left--;
  return c;
  mutex_unlock(deQ_lock);
}/*dequeue*/

void initqueue( queue *q ) {
  q->length = 0;
  q->status = IDLE;
}

void printQueue(queue* q){
  int i;
  for(i=0;i<q->length;i++){
    printf("%d---\n",q->waitline[i].id);
  }
}

customer front(queue* q){
  if(q->length==0){
    printf("Queue empty, cannot get front.\n");
    exit(1);
  }
  return q->waitline[0];
}

/*
    Input a string, parse string into an integer list
*/
void lineToken(char* line,int tok_list[]){
  int c=0;
  for(c=0;c<MAX_INPUT_LEN;c++){
    line[c] = isdigit(line[c])? line[c]:' ';
  }
  char *token;
  token = strtok(line," ");
  int i=0;
  while( token != NULL ) {
    tok_list[i++]=atoi(token);
    token = strtok(NULL," ");
  }
}

/*
    Thread function, customers automatically enter the two queues
*/
void* executeCustomerThread(void* ptr){
  mutex_lock(customer_lock);
  customer * cptr = (customer *)ptr;
  usleep(cptr->arrv_time*100000);
  printf("A customer arrives: customer ID %2d. \n",cptr->id);

  queue* Q = (cptr->q_id == 1) ? business : economy;
  enqueue(*cptr,Q);

  int i;
  for(i = 0;i < CLERK_NUM; ++i ){
    if(pthread_cond_broadcast(&convars[i])){
      printf("Error on condition broadcast.\n");
      exit(1);
    }
    usleep(1);
  }
  mutex_unlock(customer_lock);
  return ((void *)0);
}


void* executeClerkThread(void* ptr){

  int clerk_id= *(int *)ptr;
  mutex_lock(clerk_locks[clerk_id]);
  while(customer_num_left>0){
    while(!(business->length||economy->length)){
      if(pthread_cond_wait(&convars[clerk_id], &clerk_locks[clerk_id])!=0){
        printf("Fail to wait for convar. Exit");
        exit(1);
      }
    }
    customer c;
    queue* Q=(business->length>0)?business:economy;
    mutex_lock(queue_lock);
    if(Q->status==BUSY)continue;
    Q->status=BUSY;
    c = dequeue(Q);
    Q->status=IDLE;
    double fin_time = getRelativeTime();

    wait_time_list[c.id] = fin_time - wait_time_list[c.id];

    mutex_unlock(queue_lock);

    printf("A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n",getRelativeTime(),c.id,clerk_id);
    usleep(c.serv_time*100000);
    printf("A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n",getRelativeTime(),c.id,clerk_id);
  }
  mutex_unlock(clerk_locks[clerk_id]);
  return (void*)0;
}

int main(int argc, char* argv[]){
  customer customer_list[MAX_INPUT_NUM];

  /* initialize queues */
  economy=malloc(sizeof(queue));
  business=malloc(sizeof(queue));
  initqueue(economy);
  initqueue(business);

  /* open file */
  if(argc!=2){
    printf("%s\n","Wrong number of arguments. Usage: ./ACS <filename>");
    exit(1);
  }
  FILE* fp = fopen(argv[1],"r");
  if(fp == NULL){printf("%s\n","Failed to open file. Exit.");exit(1);}



  char line[MAX_INPUT_LEN];
  int tok_list[4];

   /* read the number of customers */
  if(fgets(line,MAX_INPUT_LEN,fp)!=NULL){
    CUSTOMER_NUM = atoi(line);
    customer_num_left = CUSTOMER_NUM;
  }else{
    printf("%s\n","Failed to read the number of customers. Exit.");
    exit(1);
  }

  /* read and process every customer to queue */
  int i=0;
  customer c;

  while(fgets(line,MAX_INPUT_LEN,fp)!=NULL){
    lineToken(line,tok_list);
    c.id = tok_list[0];
    c.q_id = tok_list[1];
    c.arrv_time = tok_list[2];
    c.serv_time = tok_list[3];
    customer_list[i++] = c;
    if(c.id<0||c.q_id<0||c.arrv_time<0||c.serv_time<0){
      printf("%s\n","Illegal value of input. Exit");
      exit(1);
    }

  }
  fclose(fp);
  pthread_t customer_thread_list[CUSTOMER_NUM];
  pthread_t clerk_thread_list[CLERK_NUM];
  gettimeofday(&INITIAL_TIME, NULL);
  /* initialize convars mutexes and threads for clerks */
  for(i = 0; i < CLERK_NUM; ++i){
    if(pthread_cond_init(&convars[i], NULL)){
  		printf("Failed to initialize convar %d.\n",i);
  		exit(1);
  	}
    if(pthread_mutex_init(&clerk_locks[i], NULL)){
      printf("Failed to initialize clerk mutex.\n");
      exit(1);
    }
    int* iptr = (int*)malloc(sizeof(int));
    *iptr=i;
		if (pthread_create(&clerk_thread_list[i], NULL, executeClerkThread, iptr)!=0){
			printf("Failed to create clerk thread. Exit.\n");
			exit(1);
		}
  }

  /* initialize threads for customers */
  for (i = 0; i < CUSTOMER_NUM; ++i) {
		if (pthread_create(&customer_thread_list[i], NULL, executeCustomerThread, &customer_list[i])!=0){
			printf("Failed to create customer thread. Exit.\n");
			exit(1);
		}
	}
  /* join the customer threads */
  for (i = 0; i < CUSTOMER_NUM; ++i) {
		if (pthread_join(customer_thread_list[i], NULL) != 0) {
			printf("Failed to join customer thread. Exit.\n");
			exit(1);
		}
	}
  /* join the clerk threads */
  for (i = 0; i < CLERK_NUM; ++i) {
		if (pthread_join(clerk_thread_list[i], NULL) != 0) {
			printf("Failed to join clerk thread. Exit.\n");
			exit(1);
		}
	}
  /* destroy EVERYTHING */
	if (pthread_mutex_destroy(&enQ_lock) != 0) {
		printf("Failed to destroy mutex. Exit.\n");
		exit(1);
	}
	if (pthread_mutex_destroy(&deQ_lock) != 0) {
		printf("Failed to destroy mutex. Exit.\n");
		exit(1);
	}
	if (pthread_mutex_destroy(&queue_lock) != 0) {
		printf("Failed to destroy mutex. Exit.\n");
		exit(1);
	}
	if (pthread_mutex_destroy(&queue_lock) != 0) {
		printf("Failed to destroy mutex. Exit.\n");
		exit(1);
	}
  if (pthread_mutex_destroy(&customer_lock) != 0) {
		printf("Failed to destroy mutex. Exit.\n");
		exit(1);
	}
  for(i = 0; i< CLERK_NUM; ++i){
    if (pthread_mutex_destroy(&clerk_locks[i]) != 0) {
  		printf("Failed to destroy mutex. Exit.\n");
  		exit(1);
  	}
    if (pthread_cond_destroy(&convars[i]) != 0) {
      printf("Failed to destroy convar. Exit.\n");
      exit(1);
    }
  }

  /* do time stuff */
  float all_time;
  float business_time;
  float economy_time;
  int business_num=0;
  int economy_num=0;
  /* From wait_time_list get wait time */
  for(i = 1; i <= CUSTOMER_NUM; ++i){
    if(customer_list[i-1].q_id==0){
      economy_time += wait_time_list[i];
      economy_num++;
    }else{
      business_time += wait_time_list[i];
      business_num++;
    }
    all_time+=wait_time_list[i];
  }

  printf("The average waiting time for all customers in the system is: %.2f seconds. \n",all_time/CUSTOMER_NUM);
  printf("The average waiting time for all business-class customers is: %.2f seconds. \n",(float)business_time/business_num);
  printf("The average waiting time for all economy-class customers is: %.2f seconds. \n",(float)economy_time/economy_num);

  sleep(5);
  return 0;
}
