#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#ifndef asysob_semaphore
#define asysob_semaphore


// ***** SEMAPHORE *******


typedef void *semaphore_t;

semaphore_t create(int value);

void sem_wait(semaphore_t semaphore);
void sem_signal(semaphore_t semaphore);

#endif

struct semaphore_state
{
    pthread_mutex_t mutex;
    int value;
    pthread_cond_t blocked_queue;
};

semaphore_t create(int value)
{
    struct semaphore_state *s = (struct semaphore_state *)malloc(sizeof(struct semaphore_state));
    int return_value = pthread_mutex_init(&s->mutex, NULL);
    s->value = value;
    return_value = pthread_cond_init(&s->blocked_queue, NULL);
    return (semaphore_t)s;
}

void sem_wait(semaphore_t semaphore)
{
    struct semaphore_state *s = (struct semaphore_state *)semaphore;
    int return_value = pthread_mutex_lock(&s->mutex);
    s->value--;
    if (s->value < 0)
    {
        return_value = pthread_cond_wait(&s->blocked_queue, &s->mutex);
    }
    return_value = pthread_mutex_unlock(&s->mutex);
}

void sem_signal(semaphore_t semaphore)
{
    struct semaphore_state *s = (struct semaphore_state *)semaphore;
    int return_value = pthread_mutex_lock(&s->mutex);
    s->value++;
    if (s->value <= 0)
    {
        return_value = pthread_cond_signal(&s->blocked_queue);
    }
    return_value = pthread_mutex_unlock(&s->mutex);
}

// ****** PTHREAD CREATION **********

pthread_t *createThread(void *(*function)(void *), void *argument)
{
    pthread_t *pointer = malloc(sizeof(pthread_t));
    int return_value = pthread_create(pointer, NULL, function, argument);
    return pointer;
}

// ********* VARIABLES *********

static semaphore_t full;
static semaphore_t mutex;
int count = 0;
int prod_no = 20;
int cons_no = 10;


// ******* BUFFER FUNCTION ************

struct node
{
    int data;
    struct node *next;
};
struct node *head;
struct node *tail;

void buff_insert(int x)
{
    struct node *n;
    n = (struct node *)malloc(sizeof(struct node));
    n->data = x;
    n->next = NULL;
    if (head == NULL || tail == NULL)
    {
        head = n;
        tail = n;
    }
    else
    {
        head->next = n;
        head = head->next;
    }
}

int buff_remove()
{
    int x;
    x = tail->data;
    tail = tail->next;
    return x;
}


// ********* PRODUCER AND CONSUMER FUNCTION *********

void *producer(void *arg)
{

    int item;
    printf("Producer no %d arrived \n", *((int *)arg));
    item = rand()%100;
    sem_wait(mutex);
    buff_insert(item);
    count++;
    printf("Producer no %d: Inserting  item %d , No of items = %d \n", *((int *)arg), item, count);
    sem_signal(mutex);
    sem_signal(full);
    return NULL;
}

void *consumer(void *arg)
{

    int item;
    printf("Consumer no %d arrived \n", *((int *)arg));
    sem_wait(full);
    sem_wait(mutex);
    item = buff_remove();
    count--;
    printf("Consumer no %d: Removing item %d , No of items = %d \n", *((int *)arg), item, count);
    sem_signal(mutex);
    return NULL;
}



int main()
{
    pthread_t *producer_threads[prod_no], *consumer_threads[cons_no];
    full = create(0);
    mutex = create(1);
    int arr[prod_no];
    for (int i = 0; i < prod_no; i++)
    {
        arr[i] = i + 1;
    }
    for (int i = 0; i < cons_no; i++)
    {
        consumer_threads[i] = createThread(consumer, (void *)&arr[i]);
    }
    for (int i = 0; i < prod_no; i++)
    {
        producer_threads[i] = createThread(producer, (void *)&arr[i]);
    }
    for (int i = 0; i < prod_no; i++)
    {
        pthread_join(*producer_threads[i], NULL);
    }
    for (int i = 0; i < cons_no; i++)
    {
        pthread_join(*consumer_threads[i], NULL);
    }
    return 0;
}
