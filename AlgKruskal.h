#include <iostream>
#include <string.h>
#include <limits.h>
#include <math.h>
#include<conio.h>

//FILE * f;                  /*  Входной файл, содержащий граф */

int number_of_vertices;    /* Количество вершин в графе */
int number_of_edges;       /* Количество ребер*/



struct edge_s {    /* структура, представляющая одно ребро*/
	int v;
	int u;
	int weight;
};


bool * rand_weight;
edge_s * rand_edges;
int rand_number_edge;

edge_s * edges;            /* Массив ребер, принадлежащих этому процессу*/
edge_s * msf_edges;  /* Массив ребер, который формирует MSF*/

int msf_edge_count;  /* Количество ребер в массиве ребер MSF*/


struct u_node {/* Система непересекающихся множеств*/
	u_node * parent;
	int depth;

	u_node(){
		parent = NULL;
		depth = 0;
	}
};

u_node * uf_set; /* Массив, указывающий, какая вершина какому множеству принадлежит. */

void uf_make(){
	int size = number_of_vertices;// + (number_of_vertices - 1);
	uf_set = new u_node[size];
	memset(uf_set, 0, size * sizeof(u_node));
}
u_node * uf_find(u_node * a){
	if (a->parent == NULL) return a;
	else return (a->parent = uf_find(a->parent));
}
void uf_union(u_node * a, u_node * b){
	if (a->depth > b->depth) {
		b->parent = a;
	}
	else if (a->depth<b->depth) {
		a->parent = b;
	}
	else {
		a->parent = b;
		a->depth += 1;
	}
}



void init(int argc, char** argv) {

	//f = fopen(argv[1], "rb");
	//fread(&number_of_vertices, sizeof(number_of_vertices), 1, f);
	cin >> number_of_vertices;

}


bool exist_Edge(int x, int y){
	for (int i = 0; i < rand_number_edge; i++){
		if ((rand_edges[i].u == x && rand_edges[i].v == y) || (rand_edges[i].u == y && rand_edges[i].v == x)){
			return true;
		}
	}
	return false;
}

void randAddEdges(){

		int size = number_of_vertices*(number_of_vertices - 1) / 2;
		int number_weight = 0;
		rand_number_edge = 0;
		rand_edges = new edge_s[size];
		rand_weight = new bool[size];

		int x = 0;
		int y = 0;
		int w = 0;
		int attempt = 0;
		int max_attempt = 0;

		if (number_of_vertices > 1 && number_of_vertices <= 10){
			max_attempt = 1;
		}
		else if (number_of_vertices > 10 && number_of_vertices <= 20){
			max_attempt = 2;
		}
		else if (number_of_vertices > 20 && number_of_vertices <= 30){
			max_attempt = 3;
		}
		else{
			max_attempt = 4;
		}

		for (int i = 0; i < number_of_vertices; i++){
			while (attempt < max_attempt){
				y = std::rand() % number_of_vertices;
				x = i;
				while ((x == y) || exist_Edge(x, y)){
					y = std::rand() % number_of_vertices;
				}

				w = std::rand() % size + 1;
				while (!rand_weight[w - 1]){
					w = std::rand() % size + 1;
				}
				rand_edges[rand_number_edge++] = { x, y, w };
				rand_weight[w - 1] = false;
				attempt++;
			}
			attempt = 0;
		}

		for (int i = 1; i < number_of_vertices; i++){
			if (exist_Edge(0, i)){
				continue;
			}
			w = std::rand() % size + 1;
			while (!rand_weight[w - 1]){
				w = std::rand() % size + 1;
			}
			rand_edges[rand_number_edge++] = { 0, i, w };
			rand_weight[w - 1] = false;
		}

}

void parse_input() {

	/* Начальный и конечный индекс вершины, принадлежащей этому процессу*/
	int rank_range_start = 0;
	int rank_range_end = number_of_vertices;

	number_of_edges = 0;

	
	int u, v, edge_weight;
	//int total_edges;
	//fread(&total_edges, sizeof(int), 1, f);

	/* Создание массивов для хранения реберных структур*/
	edges = new edge_s[number_of_vertices * number_of_vertices];
	msf_edges = new edge_s[number_of_vertices - 1];
	
	int i;
	int f_edge[3];
	for (i = 0; i < rand_number_edge; i++) {
		//fread(&f_edge, sizeof(int), 3, f);

		v = rand_edges[i].v; u = rand_edges[i].u; edge_weight = rand_edges[i].weight;
		edge_s e = { v, u, edge_weight };
		edges[number_of_edges++] = e;
	}

	//fclose(f);
	delete[]rand_edges;
	delete[]rand_weight;

}

void finalize() {

	delete[]uf_set;
	delete[]edges;
	delete[]msf_edges;

	MPI_Type_free(&mpi_edge);
	MPI_Finalize();

}

int compare_edges(const void *a, const void *b) {
	edge_s *ea = (edge_s *)a;
	edge_s *eb = (edge_s *)b;

	if (ea->weight < eb->weight) {
		return -1;
	}
	else if (ea->weight > eb->weight) {
		return 1;
	}

	//Никогда не происходит, если веса различны
	return 0;
}

void print_rand_edges() {
	if (rank != 0) return;
	printf("Proc 0 rand MSF edges:\n");
	for (int i = 0; i < rand_number_edge; i++) {
		printf("(%d,%d) = %d\n", rand_edges[i].v, rand_edges[i].u, rand_edges[i].weight);
	}
}
void print_msf_edges() {

	printf("Proc %d local MSF edges:\n", rank);
	for (int i = 0; i < msf_edge_count; i++) {
		printf("(%d,%d) = %d\n", msf_edges[i].v, msf_edges[i].u, msf_edges[i].weight);
	}
}
void find_msf() {

	msf_edge_count = 0;


	/*Сортировка ребер принадлежащих каждому процессу*/
	start_sort_measure();
	qsort(edges, number_of_edges, sizeof(edge_s), compare_edges);
	end_sort_measure();

	uf_make();

	int used_edge_index = 0;
	for (int i = 1; i <= number_of_edges; i++) {

		edge_s * min_edge = &edges[used_edge_index++];

		int v = (*min_edge).v;
		int u = (*min_edge).u;
		u_node * v_node = uf_find(uf_set + v);
		u_node * u_node = uf_find(uf_set + u);

		/* добавляем ребро к MSF, если оно не образует цикла*/
		if (v_node != u_node) {
			msf_edges[msf_edge_count++] = *min_edge;
			uf_union(v_node, u_node);
		}
	}

	print_msf_edges();
}



