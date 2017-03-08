#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>





int main(){
    struct sockaddr_in myaddr;
    int s,j,s2,sz,sz1,x,st,ready, ready1;
    int maxclients=10;
    int clients[10];
    char buf[140];
    char* nom[10];
    char user[20];
    socklen_t myaddr_size;


    /* On créé la socket et tous les paramètres */
    myaddr. sin_family = AF_INET;
    myaddr.sin_port =  htons(5678);
    inet_aton("127.0.0.1", &(myaddr.sin_addr));
    s=socket(PF_INET, SOCK_STREAM,0);
    
    /* Afin d'éviter l'erreur "adresse already in use"*/
    x=1;
    st = setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x));
    if( st ==-1) {
        perror("setsockopt SO_REUSEADDR");
        return 2;
    }
    bind(s,(struct sockaddr * ) &myaddr, sizeof(myaddr));
    listen(s,10);
    
    /*On créé un fdset afin de pouvoir surveiller des files descriptors*/
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    
    /*on ajoute des files descriptors, on commance par "s" afin de savoir si l'on a une tentative de connexion*/
    int maxfd;
    FD_SET(s,&rfds);
    if (s>maxfd) maxfd=s;
    
    
    /*ensuite on surveille ces descripteurs*/
    int nombreclients =0;
    int i=1;
    ready1=1;
    tv.tv_sec = 10; /* On a alors une réponse de la fonction sélect toutes les dix secondes*/
    tv.tv_usec = 0;
    printf("on accepte au maximum %d clients \n", maxclients);
    
    while ( nombreclients<maxclients){
        /*On surveille au maximum 11 files descriptors*/
        ready=select(11, &rfds, NULL, NULL, &tv);/*la fonction sélect renvoie -1 si erreur, 1 si il y a eut des modifications sur les files descriptors surveillés, 0 si rien ne s'est passé*/
        int I;
        for (I=0; I<7; I++) printf("%s\n", nom[I]);
        
        if (ready == -1){
            perror("problème de sélect");
            nombreclients=maxclients;
        }
        else if (ready==1){
            /* Des files desciptors ont été modifiés*/
            printf("De nouvelles données sont disponibles. \n");
            
            /*soit c'est "s" qui a été modifié, dans ce cas l'on a une tentative de connexion*/
            if(FD_ISSET(s,&rfds)){
                clients[nombreclients]=accept(s, (struct sockaddr *) &myaddr, &myaddr_size); /*on créé la connexion */
                sendto(clients[nombreclients], "Quel est vôtre pseudo? \n", 26 ,0 ,(struct sockaddr *) &myaddr, myaddr_size); /* On demande son pseudo */
                sz=recv(clients[nombreclients],user,sizeof(user),0); /* On reçoit le pseudo et on le place dans la chaîne de caractère user*/
                strcpy (nom, user);
                printf(" on a un nouveau client, %s\n",nom[nombreclients]);
                nombreclients=1+nombreclients;
            }
            
            else{
                /*Soit on a reçu un nouveau message */
                ready1=1;
                i=0;
                
                while(i<nombreclients && ready1==1){
                    /* Dans un premier temps on cherche qui a envoyé le message */
                    
                    if(FD_ISSET(clients[i],&rfds)){ /* FD_ISSET renvoie la valeur booléenne TRUE si client[i] est présent dans rfds,si le client[i] a reçu un message */
                        ready1=0;
                        /* On reçoit le message */
                        sz=recv(clients[i],buf,sizeof(buf),0);
                        buf[sz]=0;
                        printf("On a recu le message: %s provenant de %s \n",buf, nom[i]);
                        sz1=0;
                        
                        /*On le redistribue à tous ceux qui sont connectés à notre serveur */
                        while(sz1<nombreclients){
                            if (sz1 != i){ /* afin de ne pas renvoyer le message à celui qui l'a envoyé (pour éviter les doublons) */
                                sendto(clients[sz1],"@", 1 ,0 ,(struct sockaddr *) &myaddr, myaddr_size);
                                sendto(clients[sz1],nom[i], sizeof(nom[i]) ,0 ,(struct sockaddr *) &myaddr, myaddr_size);
                                sendto(clients[sz1], buf, sz ,0 ,(struct sockaddr *) &myaddr, myaddr_size);
                            }
                            sz1=sz1+1;
                        }
                    }
                    else {
                        i=i+1;
                    }
                }
            }
            
            
            /*le rfds a été changé par la fonction sélect or on veut surveiller tous les clients et les demande de connexion */
            FD_ZERO(&rfds);
            FD_SET(s,&rfds);
            i=0;
            while (i<nombreclients){
                FD_SET(clients[i],&rfds);
                i=i+1;
            }
        }
        else {
            printf (" Pas de nouvelles données. \n");
            FD_ZERO(&rfds);
            FD_SET(s,&rfds);
            i=0;
            while (i<nombreclients){
                FD_SET(clients[i],&rfds);
                i=i+1;
            }
        }
        
    }
    
    close(s);
    close(s2);
    return 0;
}




