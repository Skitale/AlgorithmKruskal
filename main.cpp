#include "AlgKruskal.h"

int main(int argc, char** argv) {
	number_of_vertices = 8;

	if (argc < 2) {
		printf("usage: %s filename\n", argv[0]);
		return 1;
	}

	init(argc, argv);
	randAddEdges();

	if (number_of_vertices < 50){
		print_rand_edges();
	}

	parse_input();
	find_msf();

	return 0;
}