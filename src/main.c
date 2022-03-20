#include <stdlib.h>
#include "genetic_algorithm.h"
#include <pthread.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

	// number of threads
	int num_threads = 0;

	void *status;


	pthread_barrier_t barrier;

	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, &num_threads, argc, argv)) {
		return 0;
	}

	pthread_t threads[num_threads];

	pthread_barrier_init(&barrier, NULL, num_threads);

	argument* arguments = (argument *)malloc(num_threads * sizeof(argument));
	individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(object_count, sizeof(individual));

	//intializare structuri
	for(int i = 0; i < num_threads; i++) {
		arguments[i].num_threads = num_threads;
		arguments[i].objects = objects;
		arguments[i].object_count = object_count;
		arguments[i].sack_capacity = sack_capacity;
		arguments[i].generations_count = generations_count;
		arguments[i].current_generation = current_generation;
		arguments[i].next_generation = next_generation;
		arguments[i].id = i;
		arguments[i].barrier = &barrier;
	}

	for(int i = 0; i < num_threads; i++) {
		pthread_create(&threads[i], NULL, run_genetic_algorithm, &arguments[i]);
	}

	for(int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], &status);
	}

	free(objects);	
	pthread_barrier_destroy(&barrier);
	return 0;
}
