#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

extern int HELLO_INTERVAL;
extern int identifier;
extern int HELLO_INTERVAL;
extern int NUMBER_OF_ROUTERS;
extern int NUMBER_OF_EDGES;
extern int NUMBER_OF_NEIGHBORS;
extern int* NEIGHBOR_IDS;
extern int sock;
extern struct sockaddr_in my_addr;
extern int MAX_POSSIBLE_DIST;
extern int** neighbor_link_details;
extern int** actual_link_costs;
extern int** practise_costs;
extern int*** every_node_lsa_details;
extern int* every_node_neighbors;
extern int* lsa_seq_num_det;
int helloTime;


extern pthread_mutex_t lock;

char exchange(int i){
	switch(i){
		case 0: return '0';
		case 1: return '1';
		case 2: return '2';
		case 3: return '3';
		case 4: return '4';
		case 5: return '5';
		case 6: return '6';
		case 7: return '7';
		case 8: return '8';
		case 9: return '9';
	}
}

void* sender(void* param){
	int i = 0, peer_id;
	char addr[10] = "node-";
	addr[6] = '\0';
	struct hostent *host;
	struct sockaddr_in peer_addr;
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(20039);
    // printf("%s\n", "In sender");
    bzero(&(peer_addr.sin_zero), 8);
    char send_data[1024];
	while(1){
		sleep(HELLO_INTERVAL);
		for(i = 0 ; i < NUMBER_OF_NEIGHBORS ; i++){
			peer_id = NEIGHBOR_IDS[i];
			addr[5] = exchange(peer_id);
			printf("%s\n", addr);
			host = (struct hostent *) gethostbyname(addr);
			peer_addr.sin_addr = *((struct in_addr *) host->h_addr);
			// printf("Hello sent to %d\n", peer_id);
			strncpy(send_data, "HELLO", 5);
			strncpy(send_data + 5, (char *)&identifier, 4);
			// send_data[9] = '\0';
			// printf("%s\n",pee);
			sendto(sock, send_data, 9, 0, (struct sockaddr *) &peer_addr, sizeof (struct sockaddr));
			struct timeval timenow;
			gettimeofday(&timenow, NULL);
			helloTime = ((timenow.tv_sec  % 1000000)* 1000000 + (timenow.tv_usec));
			printf("%d\n", helloTime);
		}
		printf("%s\n", "Actual Link Costs:");
		for(i = 0 ; i < NUMBER_OF_NEIGHBORS; i++){
			printf("%d %d\n", actual_link_costs[i][0], actual_link_costs[i][1]);
		}
	}
}

void* receiver(void* param){
	struct sockaddr_in peer_addr;
	int addr_len = sizeof (struct sockaddr);
	char recv_data[1024];
	char send_data[1024];
	int bytes_read;
	struct hostent *host;
	int peer_id, i, cost, max, min, dummy, node_seq_num, node_id, num_of_entries, offset, j;
	// printf("%s\n", "Lsa status:");
	// for(i = 0 ; i < NUMBER_OF_ROUTERS; i++){
	// 	printf("%d %d\n",i ,lsa_seq_num_det[i]);
	// }
	while(1){
		bytes_read = recvfrom(sock, recv_data, 1024, 0, (struct sockaddr *) &peer_addr, &addr_len);
		recv_data[bytes_read] = '\0';
		printf("%s\n", "I received something atleast.");
		// printf("\n(%s , %d) said : ", inet_ntoa(client_addr.sin_addr),
		//         ntohs(client_addr.sin_port));
		// printf("%s", recv_data);
		// fflush(stdout);
		if(strncmp(recv_data, "HELLOREPLY", 10) == 0){
			strncpy((char *)&peer_id, recv_data + 10, 4);
			pthread_mutex_lock(&lock);
			for(i = 0; i < NUMBER_OF_NEIGHBORS; i++){
				if(actual_link_costs[i][0] == peer_id){
					strncpy((char*)&dummy, recv_data + 18, 4);
					struct timeval timenow;
					gettimeofday(&timenow, NULL);
					actual_link_costs[i][1] = ((timenow.tv_sec  % 1000000)* 1000000 + (timenow.tv_usec)) - helloTime;
					// printf("%d\n",dummy );
					break;
				}
			}
			pthread_mutex_unlock(&lock);
		}else if(strncmp(recv_data, "HELLO", 5) == 0){
			printf("%s\n", "I just received a hello packet.");
			strncpy((char *)&peer_id, recv_data + 5, 4);
			// for(i = 0 ; i < NUMBER_OF_NEIGHBORS; i++){
			// 	if(practise_costs[i][0] == peer_id){
			// 		cost = practise_costs[i][1];
			// 		break;
			// 	}
			// }
			cost = 0;
			strncpy(send_data, "HELLOREPLY", 10);
			strncpy(send_data + 10, (char *)&identifier, 4);
			strncpy(send_data + 14, (char *)&peer_id, 4);
			strncpy(send_data + 18, (char *)&cost, 4);
			send_data[22] = '\0';
			// printf("%d\n", cost);
			sendto(sock, send_data, 22, 0, (struct sockaddr *) &peer_addr, sizeof (struct sockaddr));
			// printf("\nTransaction done\n");
		}else if(strncmp(recv_data, "LSA", 3) == 0){
			strncpy((char*)&node_id, recv_data + 3, 4);
			strncpy((char*)&node_seq_num, recv_data + 7, 4);
			if(node_id != identifier){
				if(lsa_seq_num_det[node_id] < node_seq_num){
					// printf("Received LSA packet from %d\n", node_id);
					lsa_seq_num_det[node_id] = node_seq_num;
					peer_addr.sin_family = AF_INET;
	    			peer_addr.sin_addr = *((struct in_addr *) host->h_addr);
	    			bzero(&(peer_addr.sin_zero), 8);
					for(i = 0 ; i < NUMBER_OF_NEIGHBORS ; i++){
						peer_addr.sin_port = htons(20000 + NEIGHBOR_IDS[i]);
						sendto(sock, recv_data, bytes_read, 0, (struct sockaddr *) &peer_addr, sizeof (struct sockaddr));
						// printf("Resent to %d\n", NEIGHBOR_IDS[i]);
					}
					// printf("%d ", node_id);
					// printf("%d ", node_seq_num);
					strncpy((char*)&num_of_entries, recv_data + 11, 4);
					// printf("%d ", num_of_entries);
					offset = 15;
					pthread_mutex_lock(&lock);
					every_node_neighbors[node_id] = num_of_entries;
					if(every_node_lsa_details[node_id] == NULL){
						every_node_lsa_details[node_id] = (int**)malloc(sizeof(int*) * num_of_entries);
						for(j = 0 ; j < num_of_entries; j++){
							every_node_lsa_details[node_id][j] = (int*)malloc(sizeof(int) * 2);
						}
					}
					for(i = 0 ; i < num_of_entries ; i++){
						strncpy((char*)&every_node_lsa_details[node_id][i][0], recv_data + offset, 4);
						strncpy((char*)&every_node_lsa_details[node_id][i][1], recv_data + offset + 4, 4);
						// printf("%d ", every_node_lsa_details[node_id][i][0]);
						// printf("%d ", every_node_lsa_details[node_id][i][1]);
						offset += 8;
					}
					// printf("\n");
					// printf("%s\n", "Lsa status:");
					// int k;
					// for(i = 0 ; i < NUMBER_OF_ROUTERS; i++){
					// 	printf("%dth router:\n",i);
					// 	for(j = 0 ; j < every_node_neighbors[i]; j++){
					// 		printf("%d %d\n",every_node_lsa_details[i][j][0], every_node_lsa_details[i][j][1] );
					// 	}
					// }
					// for(i = 0 ; i < NUMBER_OF_NEIGHBORS ; i++){
					// 	printf();
					// }
					pthread_mutex_unlock(&lock);
				}
			}
		}
	}

}
