#include <stdio.h>

int identifier = 1;
int NUMBER_OF_ROUTERS = 5;
int MAX_POSSIBLE_DIST = 100;


int allFixed(int* fixed){
	int i = 0;
	for(i = 0; i < NUMBER_OF_ROUTERS ; i++){
		if(fixed[i] == 0){
			return 0;
		}
	}
	return 1;
}

void dijkstras(int distMatrix[][NUMBER_OF_ROUTERS]);

int main(){
	NUMBER_OF_ROUTERS = 5;
	MAX_POSSIBLE_DIST = 100;
	int i = 0;
//	int** cost = (int *)malloc(sizeof(int *) * NUMBER_OF_ROUTERS);
//	for(i = 0 ; i < NUMBER_OF_ROUTERS ; i++){
//		cost[i] = (int)malloc(sizeof(int) * NUMBER_OF_ROUTERS);
//	}
	int dit[5][5] = {{50,2,4,7,50},
	      {2,50,50,6,3},
	      {4,50,50,50,50},
	      {7,6,50,50,50},
	      {50,3,50,50,50}};
	dijkstras(dit);

}
void dijkstras(int distMatrix[][NUMBER_OF_ROUTERS]){
	int present = identifier;
	int fixed[NUMBER_OF_ROUTERS];
	int i;
	int dist[NUMBER_OF_ROUTERS];
	int prev[NUMBER_OF_ROUTERS];
	for(i = 0 ; i < NUMBER_OF_ROUTERS; i++){
		fixed[i] = 0;
		dist[i] = -1;
		prev[i] = -1;
	}
	int update;
	fixed[identifier] = 1;
	dist[identifier] = 0;
	int min, next;
	while(!allFixed(fixed)){
		printf("%d\n", present);
		min = MAX_POSSIBLE_DIST;
		for(i = 0 ; i < NUMBER_OF_ROUTERS; i++){
			if(fixed[i] == 0){
				update = dist[present] + distMatrix[present][i];
				if(dist[i] == -1 || update < dist[i]){
					dist[i] = update;
					prev[i] = present;
				}
				if(min > dist[i]){
					min = dist[i];
					next = i;
				}
			}
		}
		present = next;
		fixed[present] = 1;
	}
	printf("Hi i am in\n");
	for(i = 0 ; i < NUMBER_OF_ROUTERS ; i++){
		printf(" %d ", dist[i]);
	}
	printf("\n");
}
