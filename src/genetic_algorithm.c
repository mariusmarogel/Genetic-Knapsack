#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "genetic_algorithm.h"
#include <pthread.h>

//min function

int min(int a, int b) {
	return (a < b) ? a : b;
}


int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *num_threads, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 4) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);
	*num_threads = (int) strtol(argv[3], NULL, 10); 
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, 
	int object_count, int sack_capacity, int id, int num_threads)
{
	int weight;
	int profit;

	int start = id * (double)object_count / num_threads;
	int end = min((id + 1) * (double) object_count / num_threads, object_count);

	for (int i = start; i < end; ++i) {
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}
		
		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;

	}
}

int cmpfunc(individual first, individual second) //am schimbat tipul argumentelor pentru a folosi functia in merge()
{
	int i;

	int res = second.fitness - first.fitness; // decreasing by fitness
	if (res == 0) {
		int first_count = 0, second_count = 0;

		for (i = 0; i < first.chromosome_length && i < second.chromosome_length; ++i) {
			first_count += first.chromosomes[i];
			second_count += second.chromosomes[i];
		}

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0) {
			return second.index - first.index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}


void merge(individual *arr, int left, int mid, int right) {
	int n1 = mid - left + 1;
	int n2 = right - mid;

	//vectorii folositi pentru a sorta *arr
	individual *Left = (individual *)calloc(n1, sizeof(individual));
	individual *Right = (individual *)calloc(n2, sizeof(individual));

	int i,j;

	for(i = 0; i < n1; i++) {
		Left[i] = arr[left + i];
	}
	for(i = 0; i < n2; i++) {
		Right[i] = arr[mid + 1 + i];
	}
	int k = left;
	i = 0;
	j = 0;
	while(i < n1 && j < n2) {
		if(cmpfunc(Left[i], Right[j]) < 0) {  //comparatia existenta si in schelet, adaptata
			arr[k] = Left[i];
			i++;
		} else {
			arr[k] = Right[j];
			j++;
		}
		k++;
	}
	while(i < n1) {
		arr[k] = Left[i];
		i++;
		k++;
	}
	while(j < n2) {
		arr[k] = Right[j];
		j++;
		k++;
	}
}

void mergeSort(individual *arr, int left, int right) {
	if(left >= right) {
		return;
	}
	int mid = (left + right)/2;
	mergeSort(arr, left, mid);
	mergeSort(arr, mid + 1, right);
	merge(arr, left, mid, right);
}

void *run_genetic_algorithm(void *var) 
{
	//retin datele luate din void *var

	/*Aceasta este instructiunea pentru a folosi o bariera:

	pthread_barrier_wait(((argument *)var) -> barrier); */
	
	argument *args = var;
	int object_count = args->object_count;
	sack_object *objects = args->objects;
	int sack_capacity = args->sack_capacity;
	int generations_count = args->generations_count;
	individual *tmp = NULL;
	int id = args->id;
	int num_threads = args->num_threads;
	int count, cursor;
	int start, end;
	int end1;
	int end_new;
	int mid_new;

	start = id * (double)object_count / num_threads;
	end = min((id + 1) * (double) object_count / num_threads, object_count);


	for(int i = start; i < end; i++) {
		args->current_generation[i].fitness = 0;
		args->current_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		args->current_generation[i].chromosomes[i] = 1;
		args->current_generation[i].index = i;
		args->current_generation[i].chromosome_length = object_count;

		args->next_generation[i].fitness = 0;
		args->next_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		args->next_generation[i].index = i;
		args->next_generation[i].chromosome_length = object_count;
	}

	pthread_barrier_wait(((argument *)var) -> barrier);

	for(int k = 0; k < generations_count; k++) {
		pthread_barrier_wait(((argument *)var) -> barrier); // unul din threaduri ajunge inaintea celorlaltor din cauza printarii
		cursor = 0;
		compute_fitness_function(objects, args->current_generation, object_count, sack_capacity, id, num_threads);
		pthread_barrier_wait(((argument *)var) -> barrier);

		start = id * (double)object_count / num_threads;
		end = min((id + 1) * (double) object_count / num_threads, object_count);


		//sortarea bucatilor din current_generation pe threaduri diferite
		mergeSort(args->current_generation, start, end - 1);
		pthread_barrier_wait(((argument *)var) -> barrier);

		//unirea bucatilor anterioare pe un singur thread
		if(id == 0) {
			end1 = min(2 * (double)object_count/num_threads, object_count);

			merge(args->current_generation, 0, end1 / 2, end1); //unirea primelor 2 bucati

			for(int i = 2; i < num_threads; i++) {
				end_new = min((i + 1) * (double)object_count/num_threads, object_count);
				mid_new = end_new / 2;
				merge(args->current_generation, 0, mid_new, end_new);
				end1 = end_new;
			}
		}
		pthread_barrier_wait(((argument *)var) -> barrier);

		//first 30% children
		count = object_count * 3 / 10;

		start = id * (double)count / num_threads;
		end = min((id + 1) * (double) count / num_threads, count);

		for (int i = start; i < end; ++i) {
			copy_individual(args->current_generation + i, args->next_generation + i);
		}
		pthread_barrier_wait(((argument *)var) -> barrier);

		cursor = count;

		//first 20% with first version of mutation
		count = object_count * 2 / 10;
	
		start = id * (double)count / num_threads;
		end = min((id + 1) * (double) count / num_threads, count);

		for (int i = start; i < end; ++i) {
			copy_individual(args->current_generation + i, args->next_generation + cursor + i);
			mutate_bit_string_1(args->next_generation + cursor + i, k);
		}


		pthread_barrier_wait(((argument *)var) -> barrier);
		cursor += count;

		//next 20% with second version mutation

		count = object_count * 2 / 10;
		

		start = id * (double)count / num_threads;
		end = min((id + 1) * (double) count / num_threads, count);
		pthread_barrier_wait(((argument *)var) -> barrier);
		for (int i = start; i < end; ++i) {
			copy_individual(args->current_generation + i + count, args->next_generation + cursor + i);
			mutate_bit_string_2(args->next_generation + cursor + i, k);
		}


		pthread_barrier_wait(((argument *)var) -> barrier);
		cursor += count;
		
		//crossover

		count = object_count * 3 / 10;


		if(count % 2 == 1 && id == 0) {
			copy_individual(args->current_generation + object_count - 1, args->next_generation + cursor + count - 1);
			count--;
		}

		pthread_barrier_wait(((argument *)var) -> barrier); 

		
		start = id * (double)count / num_threads;
		end = min((id + 1) * (double) count / num_threads, count);

		if(start % 2) {
			start++;
		}
 
		for(int i = start; i < count - 1 && i < end; i += 2) {
			crossover(args->current_generation + i, args->next_generation + cursor + i, k);
		}


		pthread_barrier_wait(((argument *)var) -> barrier); 

		//swap(curr, next)
			tmp = args->current_generation;
			args->current_generation = args->next_generation;
			args->next_generation = tmp;
		
		pthread_barrier_wait(((argument *)var) -> barrier);

		

		start = id * (double)object_count / num_threads;
		end = min((id + 1) * (double)object_count / num_threads, object_count);

		for (int i = start; i < end; ++i) {
			args->current_generation[i].index = i;
		}
		pthread_barrier_wait(((argument *)var) -> barrier);
		
		if (k % 5 == 0 && id == 0) {
			print_best_fitness(args->current_generation);
		}
	}
	pthread_barrier_wait(((argument *)var) -> barrier);
	compute_fitness_function(objects, args->current_generation, object_count, sack_capacity, id, num_threads);
	pthread_barrier_wait(((argument *)var) -> barrier);
	
		start = id * (double)object_count / num_threads;
		end = min((id + 1) * (double)object_count / num_threads, object_count);

		mergeSort(args->current_generation, start, end - 1);
		pthread_barrier_wait(((argument *)var) -> barrier);

		if(id == 0) {
			end1 = min(2 * (double)object_count/num_threads, object_count);

			merge(args->current_generation, 0, end1 / 2, end1);

			for(int i = 2; i < num_threads; i++) {
				end_new = min((i + 1) * (double)object_count/num_threads, object_count);
				mid_new = end_new / 2;
				merge(args->current_generation, 0, mid_new, end_new);
				end1 = end_new;
			}
		}
	pthread_barrier_wait(((argument *)var) -> barrier);

	if(id == 0) {
		print_best_fitness(args->current_generation);
	}
	pthread_barrier_wait(((argument *)var) -> barrier);
	
	
	pthread_exit(NULL);

}