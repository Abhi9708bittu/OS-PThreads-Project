#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>


typedef void *init_semaphore;

init_semaphore Create(int value);

void _wait(init_semaphore semaphore);
void  _signal(init_semaphore semaphore);


struct semaphore_state
{
	pthread_mutex_t mutex;
	int value;
	pthread_cond_t blocked_queue;
};

init_semaphore Create(int value)
{
	struct semaphore_state *s = (struct semaphore_state *)malloc(sizeof(struct semaphore_state));
	int return_value= pthread_mutex_init(&s->mutex, NULL);
	s->value = value;
	return_value= pthread_cond_init(&s->blocked_queue, NULL);
	return (init_semaphore)s;
}

void _wait(init_semaphore semaphore)
{
	struct semaphore_state *s = (struct semaphore_state *)semaphore;
	int return_value= pthread_mutex_lock(&s->mutex);
	s->value--;
	if (s->value < 0)
	{
		return_value= pthread_cond_wait(&s->blocked_queue, &s->mutex);
	}
	return_value= pthread_mutex_unlock(&s->mutex);
}

void _signal(init_semaphore semaphore)
{
	struct semaphore_state *s = (struct semaphore_state *)semaphore;
	int return_value= pthread_mutex_lock(&s->mutex);
	s->value++;
	if (s->value <= 0)
	{
		return_value= pthread_cond_signal(&s->blocked_queue);
	}
	return_value= pthread_mutex_unlock(&s->mutex);
}

pthread_t *ThreadCreation(void *(*function)(void *), void *argument)
{
	pthread_t *pointer = malloc(sizeof(pthread_t));
	int return_value = pthread_create(pointer, NULL, function, argument);
	return pointer;
}

static const int NUMBER_ELVES = 6;
static const int NUMBER_REINDEER = 9;

static init_semaphore Sem_santa;
static init_semaphore Sem_reindeer;
static init_semaphore Sem_elve;
static init_semaphore mutex;
static int elves;
static int reindeer;

void *SantaClaus(void *argument)
{
	printf("Santa is here\n");
	while (true)
	{
		_wait(Sem_santa);
		_wait(mutex);
		if (reindeer == 9)
		{
			printf("Santa is preparing sleigh\n");
			for (int r = 0; r < 9; r++)
				_signal(Sem_reindeer);
			printf("Santa is going to deliver the toys\n");
			reindeer = 0;
			printf("Santa has returned from delivery\n");
		}
		else if (elves == 3)
		{
			printf("Santa is helping elves\n");
		}
		_signal(mutex);
	}
	return argument;
}

void *Elf(void *argument)
{
	int identity = (int)argument;
	printf("I am elf %d\n", identity);
	while (true)
	{
		bool ask_for_help = rand() % 100 < NUMBER_ELVES;
		if (ask_for_help)
		{
			_wait(Sem_elve);
			_wait(mutex);
			elves++;
			if (elves == 3)
				_signal(Sem_santa);
			else
				_signal(Sem_elve);
			_signal(mutex);
			printf("Elf %d will ask for help\n", identity);
			sleep(5);
			_signal(mutex);
			elves--;
			if (elves == 0)
				_signal(Sem_elve);
			_signal(mutex);
		}

		printf("Elf %d is working\n", identity);
		sleep(5);
	}
	return argument;
}

void *Reindeer(void *argument)
{
	int identity = (int)argument;
	printf("I am reindeer %d\n", identity);
	while (true)
	{
		_wait(mutex);
		reindeer++;
		if (reindeer == 9)
			_signal(Sem_santa);
		_signal(mutex);
		_wait(Sem_reindeer);
		printf("Reindeer %d has hitched\n", identity);
		sleep(5);
	}
	return argument;
}


int main()
{
	elves = 0;
	reindeer = 0;
	Sem_reindeer = Create(0);
	Sem_santa = Create(0);
	mutex = Create(1);
	Sem_elve = Create(1);

	pthread_t *santa_claus = ThreadCreation(SantaClaus, 0);

	pthread_t *elves[NUMBER_ELVES];
	for (int elf= 0; elf< NUMBER_ELVES; elf++)
		elves[elf] = ThreadCreation(Elf, (void *)elf + 1);

	pthread_t *reindeers[NUMBER_REINDEER];
	for (int reindeer = 0; reindeer < NUMBER_REINDEER; reindeer++)
		reindeers[reindeer] = ThreadCreation(Reindeer, (void *)reindeer + 1);

	int return_value= pthread_join(*santa_claus, NULL);
}
