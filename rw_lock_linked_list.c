#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

#include "helpers/headers/util.h"
#include "helpers/headers/constants.h"

// struct operation operations[m] = { NULL };
int operations[m];
struct list_node_s* ll_head = NULL;
pthread_rwlock_t rw_lock;

void *doRandomOperations(void *rank);
double Iteration();

int main(){
    int i;
    double sum=0;
    double t[r_iterations];

    for(i = 0; i < r_iterations; i++)
    {
        t[i] = Iteration()*pow(10,6);
        sum += t[i];
    }

    double average = sum/r_iterations;
    double error_sum = 0;

    for ( i = 0; i < r_iterations; i++)
    {
       error_sum += pow(t[i]-average, 2);
    }
    
    double std = sqrt(error_sum/(r_iterations-1));
    double suff_n = pow( ((100*1.96*std)/(5*average)) ,2);

    printf("RWL - Average: %f, STD: %f n: %f\n", average, std, suff_n);

    return 0;
}

double Iteration(){
    init_linked_list(ll_head);
    generateRandomOperations(operations);

    long thread;
    pthread_t* thread_handles;

    if (pthread_rwlock_init(&rw_lock, NULL) != 0) {
        printf("Read/write lock initialization failed.");
        return 1;
    }


    // clock_t start, end;
    // double cpu_time_used;
    // start = clock();

    // struct timeval begin, end;
    // gettimeofday(&begin, 0);

    struct timespec start, finish;    
    clock_gettime(CLOCK_REALTIME, &start);


    thread_handles = malloc(thread_count * sizeof(pthread_t));
    for (thread = 0; thread < thread_count; thread++){
        pthread_create(&thread_handles[thread], NULL, doRandomOperations, (void*) thread);
    }

    for (thread = 0; thread < thread_count; thread++){
        pthread_join(thread_handles[thread], NULL);
    }

    free(thread_handles);
    pthread_rwlock_destroy(&rw_lock);

    // end = clock();
    // cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    // gettimeofday(&end, 0);
    // long seconds = end.tv_sec - begin.tv_sec;
    // long microseconds = end.tv_usec - begin.tv_usec;
    // double elapsed = seconds + microseconds*1e-6;

    clock_gettime(CLOCK_REALTIME, &finish);
    double time_spent = (finish.tv_sec - start.tv_sec) + ((finish.tv_nsec - start.tv_nsec) / BILLION);

    return time_spent;

}

void *doRandomOperations(void *rank) {
    long my_rank = (long) rank;

    int local_m = m/thread_count;
    int my_first_row = my_rank * local_m;
    int my_last_row = (my_rank + 1) * local_m - 1;

    int i;
    for (i = my_first_row; i <= my_last_row; i++){
        unsigned int rand_numb = (rand() % (upper - lower)) + lower;
        // if (*Member == (operations[i].function)){       
        if (operations[i] == 0){         
            pthread_rwlock_rdlock(&rw_lock);
            // (*(operations[i].function))(operations[i].value, &ll_head);
            Member(rand_numb, ll_head);
            pthread_rwlock_unlock(&rw_lock);
        }
        else if (operations[i] == 1) {
            pthread_rwlock_wrlock(&rw_lock);
            // (*(operations[i].function))(operations[i].value, &ll_head);
            Insert(rand_numb, &ll_head);
            pthread_rwlock_unlock(&rw_lock);
        } else {
            pthread_rwlock_wrlock(&rw_lock);
            // (*(operations[i].function))(operations[i].value, &ll_head);
            Delete(rand_numb, &ll_head);
            pthread_rwlock_unlock(&rw_lock);
        }
    }

    return NULL;
}