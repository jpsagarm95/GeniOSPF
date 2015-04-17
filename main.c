#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#include "helloPacketsHandler.h"
#include "lsaPacketsHandler.h"
#include "shortestPathFinder.h"

int identifier;
FILE* ifp;
FILE* ofp;
char outfile[1000];
int HELLO_INTERVAL;
int LSA_INTERVAL;
int SPF_INTERVAL;
int NUMBER_OF_ROUTERS;
int NUMBER_OF_EDGES;
int** linkDetails;
int** neighbor_link_details;
int NUMBER_OF_NEIGHBORS;
int* NEIGHBOR_IDS;
int sock;
struct sockaddr_in my_addr;
struct hostent *host;
int MAX_POSSIBLE_DIST;
int** actual_link_costs;
int** practise_costs;
int lsa_seq_num;
int*** every_node_lsa_details;
int* every_node_neighbors;
int* lsa_seq_num_det;

pthread_mutex_t lock;

void initializer(int argc, char *argv[]);

int main(int argc, char *argv[]){
	srand(time(NULL));
	initializer(argc, argv);
	struct sockaddr_in my_addr;
	struct hostent *host;
	char send_data[1024];

	if (pthread_mutex_init(&lock, NULL) != 0){
        printf("\n mutex init failed\n");
        return 1;
    }

	host = (struct hostent *) gethostbyname("localhost");

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(20039);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	if (bind(sock, (struct sockaddr *) &my_addr,
            sizeof (struct sockaddr)) == -1) {
        perror("Bind");
        exit(1);
    }
	pthread_t hello_packet_sender;
	// pthread_t lsa_packet_send;
    // pthread_t rec;
    // pthread_t spf_t;
    pthread_create(&hello_packet_sender, NULL, &sender, NULL);
    // pthread_create(&lsa_packet_send, NULL, &lsa_packet_sender, NULL);
    // pthread_create(&rec, NULL, &receiver, NULL);
    // pthread_create(&spf_t, NULL, &spf, NULL);

    while(1);
}

void initializer(int argc, char *argv[]){
	int i;
	char buf[1000];
	HELLO_INTERVAL = 1;
	LSA_INTERVAL = 5;
	SPF_INTERVAL = 20;
	lsa_seq_num = 1;
	for(i = 1 ; i < argc ; i++){
		if(strcmp(argv[i],"-i") == 0){
			i++;
			identifier = atoi(argv[i]);
		}else if(strcmp(argv[i],"-f") == 0){
			i++;
			ifp = fopen(argv[i], "r");
//			printf("%s\n", argv[i]);
		}else if(strcmp(argv[i],"-o") == 0){
			i++;
			strcpy(outfile, argv[i]);
			ofp = fopen(argv[i], "w");
			fclose(ofp);
//			printf("%s\n", argv[i]);
		}else if(strcmp(argv[i],"-h") == 0){
			i++;
			HELLO_INTERVAL = atoi(argv[i]);
		}else if(strcmp(argv[i],"-a") == 0){
			i++;
			LSA_INTERVAL = atoi(argv[i]);
		}else if(strcmp(argv[i],"-s") == 0){
			i++;
			SPF_INTERVAL = atoi(argv[i]);
		}
	}
	//ifp = fopen("infile.txt", "r");
	//ofp = fopen("outfile.txt", "w");

	fgets(buf, 1000, ifp);
	char* data;
	data = strtok(buf, "\t\n ");
	NUMBER_OF_ROUTERS = atoi(data);
	data = strtok(NULL, "\t\n ");
	NUMBER_OF_EDGES = atoi(data);
	linkDetails = (int**)malloc(sizeof(int *) * NUMBER_OF_EDGES);
	int j;
	for(i = 0 ; i < NUMBER_OF_EDGES ; i++){
		linkDetails[i] = (int *)malloc(sizeof(int) * 2);
		fgets(buf, 1000, ifp);
		data = strtok(buf, "\t\n ");
		for(j = 0 ; j < 2 ; j++){
			linkDetails[i][j] = atoi(data);
			data = strtok(NULL, "\t\n, ");
		}
	}

	NUMBER_OF_NEIGHBORS = 0;
	for(i = 0 ; i < NUMBER_OF_EDGES ; i++){
		if(linkDetails[i][0] == identifier || linkDetails[i][1] == identifier){
			NUMBER_OF_NEIGHBORS++;
		}
	}
	neighbor_link_details = (int**)malloc(sizeof(int*) * NUMBER_OF_NEIGHBORS);
	actual_link_costs = (int**)malloc(sizeof(int*) * NUMBER_OF_NEIGHBORS);
	// practise_costs = (int**)malloc(sizeof(int*) * NUMBER_OF_NEIGHBORS);
	for(i = 0 ; i < NUMBER_OF_NEIGHBORS ; i++){
		neighbor_link_details[i] = (int *)malloc(sizeof(int) * 3);
		actual_link_costs[i] = (int *)malloc(sizeof(int)*2);
		// practise_costs[i] = (int *)malloc(sizeof(int)*2);
	}


	every_node_neighbors = (int*)malloc(sizeof(int) * NUMBER_OF_ROUTERS);
	every_node_lsa_details = (int***)malloc(sizeof(int**) * NUMBER_OF_ROUTERS);
	lsa_seq_num_det = (int*)malloc(sizeof(int) * NUMBER_OF_ROUTERS);
	for(i = 0 ; i < NUMBER_OF_ROUTERS ; i++){
		every_node_neighbors[i] = 0;
		every_node_lsa_details[i] = NULL;
		lsa_seq_num_det[i] = -1;
	}

	NEIGHBOR_IDS = (int*)malloc(sizeof(int) * NUMBER_OF_NEIGHBORS);
	int counter = 0, min, max, cost;
	MAX_POSSIBLE_DIST = 0;
	for(i = 0 ; i < NUMBER_OF_EDGES ; i++){
		if(linkDetails[i][0] == identifier){
			NEIGHBOR_IDS[counter] = linkDetails[i][1];
			neighbor_link_details[counter][0] = linkDetails[i][1];
			// neighbor_link_details[counter][1] = linkDetails[i][2];
			// neighbor_link_details[counter][2] = linkDetails[i][3];
			actual_link_costs[counter][0] = linkDetails[i][1];
			actual_link_costs[counter][1] = 99999;
			// min = neighbor_link_details[counter][1];
			// max = neighbor_link_details[counter][2];
			// if(min == max){
			// 	cost = min;
			// }else{
			// 	cost = (rand()%(max - min + 1)) + min;
			// }
			// practise_costs[counter][0] = linkDetails[i][1];
			// practise_costs[counter][1] = cost;
			counter++;
		}else if(linkDetails[i][1] == identifier){
			NEIGHBOR_IDS[counter] = linkDetails[i][0];
			neighbor_link_details[counter][0] = linkDetails[i][0];
			// neighbor_link_details[counter][1] = linkDetails[i][2];
			// neighbor_link_details[counter][2] = linkDetails[i][3];
			actual_link_costs[counter][0] = linkDetails[i][0];
			actual_link_costs[counter][1] = 99999;
			// min = neighbor_link_details[counter][1];
			// max = neighbor_link_details[counter][2];
			// if(min == max){
			// 	cost = min;
			// }else{
			// 	cost = (rand()%(max - min + 1)) + min;
			// }
			// practise_costs[counter][0] = linkDetails[i][0];
			// practise_costs[counter][1] = cost;
			counter++;
		}

		// if(MAX_POSSIBLE_DIST < linkDetails[i][3]){
		// 	MAX_POSSIBLE_DIST = linkDetails[i][3];
		// }
	}

	MAX_POSSIBLE_DIST = 999999;
	// for(i = 0 ; i < NUMBER_OF_NEIGHBORS ; i++){
	// 	printf("%d %d\n", practise_costs[i][0], practise_costs[i][1]);
	// }
}