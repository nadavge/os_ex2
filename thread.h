#define ERROR -1

struct Thread {
	int tid;
	Priority priority;
	address_t entry;
	int quantums;
	char stack[STACK_SIZE];
};
