/* schedule.c
 * This file contains the primary logic for the 
 * scheduler.
 */
#include "schedule.h"
#include "macros.h"
#include "privatestructs.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include "list.h"
#define ALPHA 0.5

/* Local Globals
 * rq - This is a pointer to the runqueue that the scheduler uses.
 * current - A pointer to the current running task.
 */
struct runqueue *rq;
struct task_struct *current;

/* External Globals
 * jiffies - A discrete unit of time used for scheduling.
 *			 There are HZ jiffies in a second, (HZ is 
 *			 declared in macros.h), and is usually
 *			 1 or 10 milliseconds.
 */
extern long long jiffies;
extern struct task_struct *idle;

/*-----------------Initilization/Shutdown Code-------------------*/
/* This code is not used by the scheduler, but by the virtual machine
 * to setup and destroy the scheduler cleanly.
 */
 
 /* initscheduler
  * Sets up and allocates memory for the scheduler, as well
  * as sets initial values. This function should also
  * set the initial effective priority for the "seed" task 
  * and enqueu it in the scheduler.
  * INPUT:
  * newrq - A pointer to an allocated rq to assign to your
  *			local rq.
  * seedTask - A pointer to a task to seed the scheduler and start
  * the simulation.
  */
void initschedule(struct runqueue *newrq, struct task_struct *seedTask) {

	seedTask->next = seedTask->prev = seedTask;
	newrq->head = seedTask;
	newrq->nr_running++;

}

/* killschedule
 * This function should free any memory that 
 * was allocated when setting up the runqueu.
 * It SHOULD NOT free the runqueue itself.
 */
void killschedule() {

	return;
}


void print_rq () {
	struct task_struct *curr;
	
	printf("Rq: \n");
	curr = rq->head;
	if (curr)
		printf("%s", curr->thread_info->processName);
	while(curr->next != rq->head) {
		curr = curr->next;
		printf(", %s", curr->thread_info->processName);
	};
	printf("\n");
}

/*-------------Scheduler Code Goes Below------------*/
/* This is the beginning of the actual scheduling logic */

/* schedule
 * Gets the next task in the queue
 */
void schedule()
{
	struct task_struct *curr;
	
	current->need_reschedule = 0; /* Always make sure to reset that, in case *
								   * we entered the scheduler because current*
								   * had requested so by setting this flag   */
		//SJF implementation 
		switch (rq->nr_running) {
			case 1: {
				//the only process able to run is equal to the init process.
				curr = rq->head;
				break;
			}

			case 2: {
				
				//In case we have only two processes ready to run (nr_running == 2),
				//when the second process will get the CPU from the INIT process, update
				//the data from the second process this very minute.
				if (current != rq->head->next){
					//calculate burst prev and total wait time for the process 
					rq->head->next->burst_prev = sched_clock();
					rq->head->next->total_wait_time += sched_clock() - rq->head->next->rq_wait_time;
				}
				
				curr = rq->head->next;
				break;
			}

			default : {

				double temp_burst;
				double temp_exp_burst;
				
				print_rq();

				if(current != idle){
					//calculate the expected burst of the current process 
					temp_burst = current->burst_prev;
					current->burst_prev = sched_clock() - current->burst_prev;
					temp_exp_burst = current->exp_burst;
					current->exp_burst = (current->burst_prev + ALPHA*current->exp_burst)/(1+ALPHA);
				}

				else	//Visualizing idle process activity 
					printf("Process: %s\n", current->thread_info->processName);

				struct task_struct *temp;
				struct task_struct *temp_min;
				double min;
				double max;
				double goodness, min_goodness = DBL_MAX;
				

				temp_min = rq->head->next;
				//searches the running queue for the minimun expected burst 
				for (temp = rq->head->next; temp != rq->head; temp = temp->next) {
					
					if (temp->exp_burst < temp_min->exp_burst)
						temp_min = temp;

				}

				min = temp_min->exp_burst;

				//searches the running queue for the maximum waiting time in the queue
				for(temp = rq->head->next; temp->next != rq->head; temp=temp->next){
					
					//there is no need to take the current process under consideration 
					if(temp == current)
						continue;
					
					else{	//we actually need the smallest rq_wait_time value among all the
						//processes. By doing so the term: ( sched_clock() - smallest rq_wait_time value )
						//will get the biggest value
						if(temp->rq_wait_time > temp->next->rq_wait_time)
							max = temp->next->rq_wait_time;
					}
				} 
				
				//calculates the goodness for each process and finds the minimun 
				for (temp = rq->head->next; temp != rq->head; temp = temp->next) {
					
					goodness = ((1+temp->exp_burst)/(min + 1)) *
						    ((1 + (sched_clock() - max))/(1+ (sched_clock() - temp->rq_wait_time)));


					if(temp == current){
						//if current process is equal to temp process, wait_time_rq = 0
						goodness = ((1+temp->exp_burst)/(min + 1))
							      *(1 + (sched_clock() - max));
					}
						
					if(goodness < min_goodness){
						curr = temp;
						min_goodness = goodness;
					}
				}

				
				//curr = temp_min; //choose the next process to run according to the minimun expected burst 
				curr->burst_prev = sched_clock();

				//if the process to run next remains the same, don't change its data 
				if (curr == current) {
					curr->burst_prev = temp_burst;
					curr->exp_burst = temp_exp_burst;
				}

				else{
					//set enqueue time for current process before change
					if(current != idle)
						current->rq_wait_time = sched_clock();
					
					//update total rq_wait_time for curr before leaving the run queue
					curr->total_wait_time += sched_clock() - curr->rq_wait_time;
				
				}
				
				//curr will get the CPU, set wait time equal to 0
				curr->rq_wait_time = 0;
				
				break;
			}
		}
		
		context_switch(curr);

	}


/* sched_fork
 * Sets up schedule info for a newly forked task
 */
void sched_fork(struct task_struct *p) {
	
	p->time_slice = 100;
	p->exp_burst = 0;
	p->burst_prev = 0;
	p->rq_wait_time = 0;
	p->total_wait_time = 0;
}

/* scheduler_tick
 * Updates information and priority
 * for the task that is currently running.
 */
void scheduler_tick(struct task_struct *p)
{

	schedule();

}

/* wake_up_new_task
 * Prepares information for a task
 * that is waking up for the first time
 * (being created).
 */
void wake_up_new_task(struct task_struct *p)
{	
	p->next = rq->head->next;
	p->prev = rq->head;
	p->next->prev = p;
	p->prev->next = p;
	p->spawned_time = sched_clock();
	p->rq_wait_time = sched_clock();
	rq->nr_running++;
}

/* activate_task
 * Activates a task that is being woken-up
 * from sleeping.
 */
void activate_task(struct task_struct *p)
{
	p->next = rq->head->next;
	p->prev = rq->head;
	p->next->prev = p;
	p->prev->next = p;
	p->rq_wait_time = sched_clock();
	rq->nr_running++;
}

/* deactivate_task
 * Removes a running task from the scheduler to
 * put it to sleep.
 */
void deactivate_task(struct task_struct *p)
{
	p->prev->next = p->next;
	p->next->prev = p->prev;
	rq->nr_running--;
	p->next = p->prev = NULL; /* Make sure to set them to NULL *
					   * next is checked in cpu.c      */
	
	//for the cases where nr_running == 2 (INIT, some other process) or nr_running == 1 (INIT process), 
	//when an interrupt occurs we have to update the data of the current process before going to sleep   
	if(rq->nr_running <= 2) {
		p->burst_prev = sched_clock() - p->burst_prev;
		p->exp_burst = (p->burst_prev + ALPHA*p->exp_burst)/(1+ALPHA);
	}
	
}
