#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>

struct process_data{
    int pid;
    char fileName[255];
};

typedef struct
{
	pthread_mutex_t mutex;
	pthread_mutex_t mutex2;
	pthread_cond_t cond;
	long long int value;
	long long int progress;
	long long int pending_tasks;

}
Semaphore;

// block

void _block(Semaphore* sem, long long int i)
{
	pthread_mutex_lock(&sem->mutex2);
	while(sem->progress + i < 0) pthread_cond_wait(&sem->cond, &sem->mutex2);
	pthread_mutex_unlock(&sem->mutex2);
}

// sem_lock

void sem_lock(Semaphore* sem)
{
	pthread_mutex_lock(&sem->mutex);

	sem->value--;
	if (sem->pending_tasks == 0)
	{
		pthread_mutex_lock(&sem->mutex2);
		sem->progress = 0;
		pthread_mutex_unlock(&sem->mutex2);
	}
	sem->pending_tasks++;
	int sem_value = sem->value, sem_progress = sem->progress;

	pthread_mutex_unlock(&sem->mutex);
	if(sem_value<0)
	{
		_block(sem, sem_value - sem_progress);
	}
}

// sem_release

void sem_release(Semaphore* sem)
{
	pthread_mutex_lock(&sem->mutex);
	pthread_mutex_lock(&sem->mutex2);

	sem->value++;
	sem->progress++;
	sem->pending_tasks--;
	pthread_cond_broadcast(&sem->cond);

	pthread_mutex_unlock(&sem->mutex2);
	pthread_mutex_unlock(&sem->mutex);
}

// sem_init

void sem_init(Semaphore* sem, long long int val)
{
	pthread_mutex_init(&sem->mutex, NULL);
	pthread_mutex_init(&sem->mutex2, NULL);
	pthread_cond_init(&sem->cond, NULL);
	sem->value = val;
	sem->pending_tasks = 0;
}


Semaphore mutex;
pthread_t processThreads[25];
int in;
int R[100];
char *spoolerDirectory[100];
struct process_data process_data_array[100];

void *process(void *processarg){
    struct process_data *data;
    data = (struct process_data *) processarg;
    int i = data->pid;
    char *fileName = data->fileName;

    printf("Process %d arrives\n", i+1);

    sem_lock(&mutex);
    sleep(5);
    printf("CRITICAL SECTION ...\n");
    R[i] = in;
    spoolerDirectory[R[i]] = fileName;
    R[i]++;
    in = R[i];
    sleep(5);
    printf("Contents of the Spooler Directory :\n");
    for(int i=0; i<in; i++){
        printf("%s ", spoolerDirectory[i]);
    }
    printf("\n");
    sem_release(&mutex);

    sleep(5);
    pthread_exit(0);
}

int main(){
    int n;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    sem_init(&mutex, 1);

    for(int i=0; i<n; i++){
        process_data_array[i].pid = i;
        sprintf(process_data_array[i].fileName, "f%d.doc", i+1);
        pthread_create(&processThreads[i], NULL, process, (void *) &process_data_array[i]);
    }

    for(int i=0; i<n; i++){
        pthread_join(processThreads[i], NULL);
    }

    return 0;
}













