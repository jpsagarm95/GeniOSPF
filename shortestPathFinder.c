#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

extern int identifier;
extern int SPF_INTERVAL;
extern int NUMBER_OF_ROUTERS;
extern int MAX_POSSIBLE_DIST;
extern int*** every_node_lsa_details;
extern int* every_node_neighbors;
extern int NUMBER_OF_NEIGHBORS;
extern int** actual_link_costs;
extern FILE* ofp;
extern char outfile[1000];

extern pthread_mutex_t lock;

int allFixed(int* fixed){
	int i = 0;
	for(i = 0; i < NUMBER_OF_ROUTERS ; i++){
		if(fixed[i] == 0){
			return 0;
		}
	}
	return 1;
}

void dijkstras(int time, int** distMatrix){
	// printf("%s\n", "Just got in");
	int present = identifier;
	int i,j;
	int fixed[NUMBER_OF_ROUTERS];
	int dist[NUMBER_OF_ROUTERS];
	int prev[NUMBER_OF_ROUTERS];
	for(i = 0 ; i < NUMBER_OF_ROUTERS ; i++){
		fixed[i] = 0;
		dist[i] = 10000000;
		prev[i] -1;
	}

	int update;
	fixed[identifier] = 1;
	dist[identifier] = 0;
	int min, next;
	while(!allFixed(fixed)){
		// printf("%s\n", "Into dijkstras");
		// printf("%d\n", present);
		// printf("%s\n", "Fixed:");
		// for(i = 0 ; i < NUMBER_OF_ROUTERS ;i++){
		// 	printf("%d ",fixed[i]);
		// }
		// printf("\n");
		min = 10000000;
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
		// printf("%s\n", "Dist:");
		// for(i = 0 ; i < NUMBER_OF_ROUTERS ;i++){
		// 	printf("%d ",dist[i]);
		// }
		// printf("\n");
		present = next;
		fixed[present] = 1;
	}
	ofp = fopen(outfile, "a");
	fprintf(ofp,"Rounting table at time %d\n", time);
	fprintf(ofp,"Destination\tPath\tCost\n");
	int now, counter;
	int path[2 * NUMBER_OF_ROUTERS + 1];
	for(i = 0 ; i < NUMBER_OF_ROUTERS ; i++){
		counter = 0;
		if(i == identifier){
			continue;
		}
		fprintf(ofp,"%d ", i);
		now = i;
		path[counter++] = i;
		while(prev[now] != identifier){
			path[counter++] = prev[now];
			now = prev[now];
		}
		path[counter] = identifier;
		for(j = counter ; j > 0 ; j--){
			fprintf(ofp,"%d-", path[j]);
		}
		fprintf(ofp,"%d ", path[0]);
		fprintf(ofp,"%d\n", dist[i]);
	}
	fprintf(ofp, "\n" );
	fclose(ofp);
}

void* spf(void* param){
	int i = 1, j;
	int time = 0;
	int **distMatrix;
	while(1){
		sleep(SPF_INTERVAL);
		distMatrix = (int**)malloc(sizeof(int*) * NUMBER_OF_ROUTERS);
		for( i = 0 ; i < NUMBER_OF_ROUTERS ; i++){
			distMatrix[i] = (int*)malloc(sizeof(int) * NUMBER_OF_ROUTERS);
			for(j = 0 ; j < NUMBER_OF_ROUTERS ; j++){
				distMatrix[i][j] = 10000000;
			}
		}
		pthread_mutex_lock(&lock);
		for(i = 0 ; i < NUMBER_OF_ROUTERS ; i++){
			if(i == identifier)
				continue;
			for(j = 0 ; j < every_node_neighbors[i] ; j++){
				distMatrix[i][every_node_lsa_details[i][j][0]] = every_node_lsa_details[i][j][1];
			}
		}
		for(i = 0 ; i < NUMBER_OF_NEIGHBORS ; i++){
			distMatrix[identifier][actual_link_costs[i][0]] = actual_link_costs[i][1];
		}
		// for(i = 0 ; i < NUMBER_OF_ROUTERS; i++){
		// 	for(j = 0 ; j < NUMBER_OF_ROUTERS; j++){
		// 		printf("%d ", distMatrix[i][j]);
		// 	}
		// 	printf("\n");
		// }
		pthread_mutex_unlock(&lock);
		dijkstras(time, distMatrix);
		for(i = 0 ; i < NUMBER_OF_ROUTERS ; i++){
			free(distMatrix[i]);
		}
		free(distMatrix);
		time++;
	}
}
