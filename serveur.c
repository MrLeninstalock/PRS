#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/inet.h>

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
    FILE *file;
    char tmp;
    int numACK=0;
    char tabACK[6];


    /*
    * Declaration des structures
    */
    struct sockaddr_in adresse, adresseClient, adressePrive;
    socklen_t alen= sizeof(adresseClient);
    socklen_t alenPrive = sizeof(adressePrive);




    /*
    * Creation du socket public
    */
    int descripteurSocketPublic = socket(AF_INET, SOCK_DGRAM, 0);
    if (descripteurSocketPublic < 0) {
		perror("ERREUR cannot create socket\n");
		return -1;
	}

    /*
    * Creation du socket pour les données
    */
    int descripteurSocketDonnees = socket(AF_INET, SOCK_DGRAM, 0);
    if (descripteurSocketDonnees < 0) {
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

    //Reutilisation du socket données
    setsockopt(descripteurSocketDonnees, SOL_SOCKET, SO_REUSEADDR, &reutilisation, sizeof(int));

    adressePrive.sin_family= AF_INET;
    adressePrive.sin_port= htons(portPrive);
    adressePrive.sin_addr.s_addr= htonl(INADDR_ANY);

    if (bind(descripteurSocketDonnees, (struct sockaddr*) &adressePrive, sizeof(adressePrive)) == -1) {
        perror("Bind fail\n");
        close(descripteurSocketDonnees);
        return -1;
    }




    /*
    * Reception du SYN
    */
    int reception;
    int envoi;
    char bufferLecture[BUFFSIZE-6];
    while(1) {
        memset(bufferACK, 0, RCVSIZE);
        //Reception du SYN
        printf("Receiving SYN\n");
        reception = recvfrom(descripteurSocketPublic, bufferACK, RCVSIZE, 0, (struct sockaddr*)&adresseClient, &alen);
        printf("Recu : %d\n", reception);
        if(strncmp(bufferACK,"SYN",3)==0){
            printf("On à reçu le SYN de la part du client ! On determine le nouveau port : %d\n", portPrive);
            memset(bufferEnvoi, 0, BUFFSIZE);
            strcpy(bufferEnvoi, "SYN-ACK");
            sprintf(newPort,"%d_",portPrive);
            memcpy(&bufferEnvoi[7], newPort, 4);
            printf("%s\n", bufferEnvoi);

            //Envoi du SYN-ACK avec le nouveau port
            sendto(descripteurSocketPublic, bufferEnvoi, sizeof(bufferEnvoi), 0, (struct sockaddr*)&adresseClient, alen);
            //portPrive++;
            //printf("Adresse : %s\n", inet_ntoa(adresseClient.sin_addr));
            //printf("Port : %d\n", adresseClient.sin_port);


            //Reception de l'ACK
            memset(bufferACK, 0, RCVSIZE);
            printf("Receiving ACK\n");
            reception = recvfrom(descripteurSocketPublic, bufferACK, RCVSIZE, 0, (struct sockaddr*)&adresseClient, &alen);
            printf("Recu 2 : %d\n", reception);
            if(strncmp(bufferACK,"ACK",3)==0){
                printf("On a bien reçu l'ACK du client !\nOn va fork !\n");
                fils = fork();
                if(fils == 0) {
                    printf("On attend le nom du fichier ! \n");
                    memset(bufferACK, 0, RCVSIZE);
                    recvfrom(descripteurSocketDonnees, bufferACK, RCVSIZE, 0, (struct sockaddr*)&adressePrive, &alenPrive);
                    close(descripteurSocketPublic);
                    numACK =0;
                    int onContinue = 1;
                    int i;
                    printf("Fichier recherché : %s\n", bufferACK);
                    file = fopen(bufferACK,"r");
    				if(file==NULL){
    					printf("Erreur d'ouverture du fichier\n");
    					exit(1);
    				}
                    printf("Ouverture réussie !!\n");

                    //Lecture du fichier
                    while(onContinue == 1) {
                        numACK++;

                        memset(tabACK, 0, 6);
                        memset(bufferEnvoi, '0', BUFFSIZE);
                        memset(bufferLecture, 0, BUFFSIZE-6);
                        sprintf(tabACK, "%d", numACK);

                        if(numACK<10){
                            memcpy(&bufferEnvoi[5], tabACK, 1);
                        }else if(numACK<100){
                            memcpy(&bufferEnvoi[4], tabACK, 2);
                        }else if(numACK<1000){
                            memcpy(&bufferEnvoi[3], tabACK, 3);
                        }else if(numACK<10000){
                            memcpy(&bufferEnvoi[2], tabACK, 4);
                        }else if(numACK<100000){
                            memcpy(&bufferEnvoi[1], tabACK, 5);
                        }else if(numACK<1000000){
                            memcpy(&bufferEnvoi[0], tabACK, 6);
                        }
                        printf("Voyons voir : \n");
                        for(int i =0; i<6; i++) {
                            printf("%c",bufferEnvoi[i]);
                        }
                        printf("\n");
                        //printf("Test du tableau ACK : %s\n", bufferEnvoi);
                        for(i=0; i<BUFFSIZE-6; i++){
                            tmp = fgetc(file);
                            if(tmp != EOF && onContinue==1) {
                                bufferLecture[i] = tmp;
                            } else {
                                bufferLecture[i] = tmp;
                                printf("On est arrivés a la fin !\n");
                                onContinue = 0;
                                break;
                            }
                        }
                        //printf("%s\n", bufferLecture);
                        memcpy(&bufferEnvoi[6], bufferLecture, BUFFSIZE-6);

                        //printf("Test après ajout ACK : %s\n", bufferEnvoi);
                        envoi = sendto(descripteurSocketDonnees, bufferEnvoi, i+6, 0, (struct sockaddr*)&adressePrive, alenPrive);


                        printf("Envoie : %d\nSur le port :%d & l'adresse %s\n", envoi, htons(adressePrive.sin_port), inet_ntoa(adressePrive.sin_addr));
                    }
                    memset(bufferEnvoi, '0', BUFFSIZE);
                    bufferEnvoi[0] = 'F';
                    bufferEnvoi[1] = 'I';
                    bufferEnvoi[2] = 'N';
                    envoi = sendto(descripteurSocketDonnees, "FIN", sizeof("FIN"), 0, (struct sockaddr*)&adressePrive, alenPrive);

                    //printf("%s\n", bufferEnvoi);
                    exit(1);
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
