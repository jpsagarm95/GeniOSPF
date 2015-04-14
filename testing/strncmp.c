#include <stdio.h>
#include <string.h>
int main(){
	int i, j = 9;
	char recv_data[10];
	strncpy(recv_data,(char *)&j, 4);
	strncpy((char *)&i, recv_data, 4);
	printf("%d\n",i);
}