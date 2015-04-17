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
// extern int sock;
extern struct sockaddr_in my_addr;
extern struct hostent *host;
extern int MAX_POSSIBLE_DIST;
extern int** neighbor_link_details;
extern int** actual_link_costs;
extern int** practise_costs;
extern int*** every_node_lsa_details;
extern int* every_node_neighbors;
extern int* lsa_seq_num_det;

extern pthread_mutex_t lock;

const char* exchange(int i){
 	switch(i){
 		case 0: return "node-0";
 		case 1: return "node-1";
 		case 2: return "node-2";
 		case 3: return "node-3";
 		case 4: return "node-4";
 		case 5: return "node-5";
 		case 6: return "node-6";
 		case 7: return "node-7";
 		case 8: return "node-8";
 		case 9: return "node-9";
 	}
 }

void* sender(void* param){
	int sock;
	int i = 0, peer_id;
	struct sockaddr_in peer_addr;
    char send_data[1024];

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

	while(1){
		sleep(HELLO_INTERVAL);
		for(i = 0 ; i < NUMBER_OF_NEIGHBORS ; i++){
			peer_id = NEIGHBOR_IDS[i];
			host = (struct hostent *) gethostbyname(exchange(peer_id));
			printf("%s\n", exchange(peer_id));
			peer_addr.sin_family = AF_INET;
		    peer_addr.sin_addr = *((struct in_addr *) host->h_addr);
		    peer_addr.sin_port = htons(20039);
		    // printf("%s\n", "In sender");
		    bzero(&(peer_addr.sin_zero), 8);
			// printf("Hello sent to %d\n", peer_id);
			strncpy(send_data, "HELLO", 5);
			strncpy(send_data + 5, (char *)&identifier, 4);
			// send_data[9] = '\0';
			// printf("%s\n",pee);
			sendto(sock, send_data, 9, 0, (struct sockaddr *) &peer_addr, sizeof (struct sockaddr));
		}
		// printf("%s\n", "Actual Link Costs:");
		// for(i = 0 ; i < NUMBER_OF_NEIGHBORS; i++){
		// 	printf("%d %d\n", actual_link_costs[i][0], actual_link_costs[i][1]);
		// }
	}
}

void* receiver(void* param){
	int sock;
	struct sockaddr_in peer_addr, my_addr;
	int addr_len = sizeof (struct sockaddr);
	char recv_data[1024];
	char send_data[1024];
	int bytes_read;
	int peer_id, i, cost, max, min, dummy, node_seq_num, node_id, num_of_entries, offset, j;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket");
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

	// printf("%s\n", "Lsa status:");
	// for(i = 0 ; i < NUMBER_OF_ROUTERS; i++){
	// 	printf("%d %d\n",i ,lsa_seq_num_det[i]);
	// }
	while(1){
		bytes_read = recvfrom(sock, recv_data, 1024, 0, (struct sockaddr *) &peer_addr, &addr_len);
		recv_data[bytes_read] = '\0';
		printf("%s\n", "I received something.");
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
					actual_link_costs[i][1] = dummy;
					// printf("%d\n",dummy );
					break;
				}
			}
			pthread_mutex_unlock(&lock);
		}else if(strncmp(recv_data, "HELLO", 5) == 0){
			strncpy((char *)&peer_id, recv_data + 5, 4);
			for(i = 0 ; i < NUMBER_OF_NEIGHBORS; i++){
				if(practise_costs[i][0] == peer_id){
					cost = practise_costs[i][1];
					break;
				}
			}
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