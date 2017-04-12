#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/inet.h>
dd
#include <time.h>


#define RCVSIZE 10
#define BUFFSIZE 1024
//TEST
int main (int argc, char *argv[]) {
    if(argc != 2) {
        printf("ERREUR nombre d'arguments invalides\n");
    }

    /*
    * Declaration des variables
    */
    int portPublic = atoi(argv[1]);
    int portPrive = portPublic + 1;
    char bufferACK[RCVSIZE];
    char bufferEnvoi[BUFFSIZE];
    int reutilisation = 1;
    char newPort[4];
    int fils;

    /*
    * Declaration des structures
    */
    struct sockaddr_in adresse, adresseClient;
    socklen_t alen= sizeof(adresseClient);




    /*
    * Creation du socket public
    */
    int descripteurSocketPublic = socket(AF_INET, SOCK_DGRAM, 0);
    if (descripteurSocketPublic < 0) {
		perror("ERREUR cannot create socket\n");
		return -1;
	}

    //Reutilisation du socket public
    setsockopt(descripteurSocketPublic, SOL_SOCKET, SO_REUSEADDR, &reutilisation, sizeof(int));

	adresse.sin_family= AF_INET;
	adresse.sin_port= htons(portPublic);
	adresse.sin_addr.s_addr= htonl(INADDR_ANY);

    if (bind(descripteurSocketPublic, (struct sockaddr*) &adresse, sizeof(adresse)) == -1) {
		perror("Bind fail\n");
		close(descripteurSocketPublic);
		return -1;
	}



    /*
    * Reception du SYN
    */
    while(1) {
        memset(bufferACK, 0, RCVSIZE);
        //Reception du SYN
        recvfrom(descripteurSocketPublic, bufferACK, RCVSIZE, 0, (struct sockaddr*)&adresseClient, &alen);
        if(strncmp(bufferACK,"SYN",3)==0){
            portPrive++;
            printf("On à reçu le SYN de la part du client ! On determine le nouveau port : %d\n", portPrive);
            memset(bufferEnvoi, 0, BUFFSIZE);
            strcpy(bufferEnvoi, "SYN-ACK");
            sprintf(newPort,"%d_",portPrive);
            memcpy(&bufferEnvoi[7], newPort, 4);
            printf("%s\n", bufferEnvoi);

            //Envoi du SYN-ACK avec le nouveau port
            sendto(descripteurSocketPublic, bufferEnvoi, sizeof(bufferEnvoi), 0, (struct sockaddr*)&adresseClient, alen);

            //printf("Adresse : %s\n", inet_ntoa(adresseClient.sin_addr));
            //printf("Port : %d\n", adresseClient.sin_port);


            //Reception de l'ACK
            memset(bufferACK, 0, RCVSIZE);
            recvfrom(descripteurSocketPublic, bufferACK, RCVSIZE, 0, (struct sockaddr*)&adresseClient, &alen);
            if(strncmp(bufferACK,"ACK",3)==0){
                printf("On a bien reçu l'ACK du client !\nOn va fork !\n");
                fils = fork();
                if(fils == 0) {
                    printf("Je suis le fils !\n");
                } else{
                }
            }
        }
    }


    /*
    * Envoie SYN-ACK + port
    */


    /*
    * Reception ACK
    */


    /*
    *
    */
    return 0;
}
