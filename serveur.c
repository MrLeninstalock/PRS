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
        printf("ERREUR nombre d'arguments invalides: ./serveur port\n");
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
    int sizeBuffer;
    int tailleFenetre=50;
    int TabACKrecu[50]={0};
    int TabIndiceACK[50]={0};


    /*
    * Declaration des structures
    */
    struct sockaddr_in adresse, adresseClient, adressePrive;
    socklen_t alen= sizeof(adresseClient);
    socklen_t alenPrive = sizeof(adressePrive);
    fd_set Ldesc;
    struct timeval tv;
    tv.tv_sec=1;




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
                    numACK =1;
                    int onContinue = 1;
                    int i, j, k;
                    int flightSize=0;
                    int cwnd=1;
                    char ACKrecu[6] = "AAAAAA";
                    int resend=1;
                    int lastACK;
                    printf("Fichier recherché : %s\n", bufferACK);
                    file = fopen(bufferACK,"r");
            				if(file==NULL){
            					printf("Erreur d'ouverture du fichier\n");
            					exit(1);
            				}
                    printf("Ouverture réussie !!\n");

                    //Lecture du fichier
                    while(onContinue == 1 || resend==1) {

                        resend=0;
                        memset(TabACKrecu,0,tailleFenetre);
                        memset(TabIndiceACK,0,tailleFenetre);
                        for(i=0;i<tailleFenetre;i++){


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
                          printf("ACK envoyé: \n");
                          for(int j =0; j<6; j++) {
                              printf("%c",bufferEnvoi[j]);
                          }
                          printf("\n");
                          usleep(5000);
                          sizeBuffer=fread(bufferLecture, 1, BUFFSIZE-6, file);
                          //printf("Lecture : %s\n", bufferLecture );
                          memcpy(&bufferEnvoi[6], bufferLecture, BUFFSIZE-6);
                          usleep(100);
                          sendto(descripteurSocketDonnees, bufferEnvoi, sizeBuffer+6, 0, (struct sockaddr*)&adressePrive, alenPrive);
                          TabACKrecu[i]=0;
                          TabIndiceACK[i]=numACK;
                          /*printf("trame 0 :\n%s\n",bufferEnvoi);
                          sleep(2);
                          fseek(file,numACK*sizeBuffer, SEEK_SET);
                          sizeBuffer=fread(bufferLecture, 1, BUFFSIZE-6, file);
                          printf("trame test :\n%s\n",bufferLecture);
                          sleep(10);*/

                          numACK++;

                          if(feof(file)){
                              onContinue=0;
                              break;
                          }

                        }





                        //printf("Test après ajout ACK : %s\n", bufferEnvoi);
                        flightSize++;
                        printf("On attend confirmation de l'ACK\n");
                        for(i=0;i<tailleFenetre;i++) {

                            FD_ZERO(&Ldesc);
                            FD_SET(descripteurSocketDonnees,&Ldesc);
                            tv.tv_sec=0;
                            tv.tv_usec =15000;
                            printf("On va faire le select\n");
                    	      int a = select(descripteurSocketDonnees+1,&Ldesc,NULL,NULL, &tv);
                            printf("Select : %d\n", a);
                            if(FD_ISSET(descripteurSocketDonnees,&Ldesc)==1) {
                                recvfrom(descripteurSocketDonnees, bufferACK, RCVSIZE, 0, (struct sockaddr*)&adressePrive, &alenPrive);
                                for(int j = 0; j<6; j++) {
                                    ACKrecu[j] = bufferACK[j+3];
                                }
                                printf("%d\n", atoi(ACKrecu));

                                for(j=0;j<tailleFenetre;j++){
                                  if(atoi(ACKrecu)>=TabIndiceACK[j] && TabACKrecu[j]==0){
                                    TabACKrecu[j]=1;
                                    //break;
                                  }
                                }


                            } else {
                                break;
                                //sendto(descripteurSocketDonnees, bufferEnvoi, sizeBuffer+6, 0, (struct sockaddr*)&adressePrive, alenPrive);

                            }
                        }

                        for(i=0;i<tailleFenetre;i++){
                          printf("%d; ",TabACKrecu[i] );
                        }
                        printf("\n" );

                        for(i=0;i<tailleFenetre;i++){
                          if(TabACKrecu[i]==0){
                            numACK=TabIndiceACK[i];
                            fseek(file,numACK*(BUFFSIZE-6), SEEK_SET);
                            resend=1;
                            printf("On va renvoyer l ack %d d'indice %d\n", numACK, i);
                            break;
                          }
                        }

                        flightSize--;
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
