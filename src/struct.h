

//structura pentru a trimite mai multe argumente la thread function
typedef struct arg_struct {
	sack_object *objects;
	int object_count;
	int sack_capacity;
	int generations_count;
	individual *current_generation;
	individual *next_generation;
	int id;
} argument;
