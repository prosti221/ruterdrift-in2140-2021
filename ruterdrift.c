#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node{
    int nodeID;
    char flag;
    char length;
    char model[249];
    struct node **adj;
    int x; //Antall naboer
}NODE;

NODE *nodes, *cpy; 
int N = 0;

NODE *getNode(int ID); //Returnerer peker til node.
void freeAllAdj(); //Frigjør minne av alle nabo-noder
void freeAdj(int ID); //Frigjør minne av nabo-noder gitt ID
void printBin(char a); //Printer en Char i binaer form
int DFS(int ID,int R2_ID, int *visited);
int getIndex(int ID); //Returnerer indeks til node gitt ID
void run_command(char *line); //Kjører kommando fra linje
void remove_connection(int ID); //Fjerner alle koblinger til en gitt ruter

void print(int ID);
void sett_flagg(int ID, unsigned char flag, int verdi);
void sett_modell(int ID, char* name);
void legg_til_kobling(int ID1, int ID2);
void slett_ruter(int ID);
void finnes_rute(int R1_ID, int R2_ID);
void skriv_til_fil(FILE *fd);

int main(int argc, char *argv[]){
   if(argc != 3){
       printf("Feil antall argumenter.");
       exit(1);
   } 
    FILE *data, *command;
    char *data_buffer, *command_buffer;
    long datalen, commandlen;
    if( (data = fopen(argv[1], "rb")) == NULL || (command = fopen(argv[2], "r")) == NULL){
        printf("\nError reading file");
        exit(1);
    }
    //Finner antall linjer i kommandofil
    int ch;
    int line_count = 0;
    do{
        ch = fgetc(command);
        if(ch == '\n') line_count++;
    } while(ch != EOF);
    //Finner lengde til datafil og kommandofil
    fseek(data, 0, SEEK_END);
    datalen = ftell(data);
    
    fseek(command, 0, SEEK_END);
    commandlen = ftell(command);   

    rewind(data);
    rewind(command);
    //Allokerer minne til data-buffer og kommando-buffer
    if ( (data_buffer = (char*) malloc(datalen)) == NULL){
        printf("Error allocating memory");
        exit(1);
    }
    if ( (command_buffer = (char*) malloc(commandlen)) == NULL){
        printf("Error allocating memory");
        exit(1);
    }
    fread(data_buffer, datalen, 1, data);
    fread(command_buffer, commandlen, 1, command);
    //Allokerer minne til rutere
    memcpy(&N, data_buffer, 4);
    if ((nodes = malloc(sizeof(NODE) * N)) == NULL){
        printf("\nError allocating memory");
        exit(1);
    }
    //Fyller inn data
    int offset = 4; 
    for(int i = 0; i < N; i++){
        unsigned int nodeID = 0;
        char flag, length = 0;
        char name[249];

        memcpy(&nodeID, data_buffer + offset, sizeof(int));
        offset += sizeof(int);
        memcpy(&flag, data_buffer + offset, sizeof(char));
        offset += sizeof(char);
        memcpy(&length, data_buffer + offset, sizeof(char));
        offset += sizeof(char);
        memcpy(name, data_buffer + offset, (int)length + 1);
        offset += (int)length + 1;
         
        nodes[i] = (NODE){nodeID:nodeID, flag:flag, length:length, x:0};
        memcpy(nodes[i].model, name, strlen(name));
        //Allokerer minne til nabo-noder
        if ( (nodes[i].adj = malloc(sizeof(NODE*) * 10)) == NULL){
            printf("Error allocating memory");
            exit(1);
        }
    }
    //Setter sammen koblinger
    while(offset < datalen){
        NODE *startNode, *endNode;
        int R1, R2;
        memcpy(&R1, data_buffer + offset, sizeof(int));
        offset += sizeof(int);
        memcpy(&R2, data_buffer + offset, sizeof(int));
        offset += sizeof(int); 
        startNode = getNode(R1);
        endNode = getNode(R2);
        startNode->adj[startNode->x] = endNode; //Add R2 til R1 sin array
        startNode->x += 1; //nabo-node teller
    }
    //Kommando filen
    char *line;
    line = strtok(command_buffer, "\n");
    run_command(line);
    for(int i = 1; i < line_count; i++){
        if((line = strtok(NULL, "\n")) == NULL){
            break;
        }      
      run_command(line); //Kjoerer kommando fra linje
    }
    //Skriv til fil
    FILE *f_out;
    skriv_til_fil(f_out);
    //fclose
    fclose(data);
    fclose(command);
    //Frigjoer minne
    freeAllAdj();
    free(nodes);
    free(cpy);
    free(data_buffer);
    free(command_buffer);
    return 0;
}

NODE* getNode(int ID){
    for(int i = 0; i < N; i++){
        if(nodes[i].nodeID == ID){
            NODE* temp = &nodes[i];
            return temp;
        }
    }
    printf("Noden %d finnes ikke", ID);
    return NULL;
}

void freeAllAdj(){
    for(int i = 0; i < N; i++){
        free(nodes[i].adj);
        nodes[i].x = 0;
    }
}

void freeAdj(int ID){
    for(int i = 0; i<N; i++){
        if(nodes[i].nodeID == ID){
            free(nodes[i].adj);
            nodes[i].x = 0;
        }
    }
}

void print(int ID){
    NODE *p = getNode(ID);
    printf("\nRuter: %d, Modell: %s, Flagg: ", ID, p->model); printBin(p->flag);
    printf("Koblinger:\n");
    for(int i = 0; i < p->x; i++){
        printf("%d ----> %d\n", ID, p->adj[i]->nodeID);
    }
}

void sett_flagg(int ID, unsigned char flag, int verdi){
    if(flag < 0 || flag > 4|| flag == 3 || verdi < 0 || verdi > 15){
        printf("\nugyldige parametere for flag\n");
        return;
    }
    NODE *p = getNode(ID);
    if(p == NULL)
        return;
    if(verdi == 0 && flag < 4){
        p->flag &= ~(1 << flag);
    }else if (verdi == 1 && flag < 4){
        p->flag |= (1 << flag);
    }else{
        p->flag &= ~0xf; //nuller ut siste 4 bits
        p->flag |= (char)verdi & 0xf; // OR'er siste 4 bits med verdi 
    }
}

void sett_modell(int ID, char* name){
    NODE *p = getNode(ID);
    if (p == NULL)
        return;
    memset(p->model, 0, strlen(p->model)); //Nullstiller minne 
    memcpy(p->model, name, strlen(name)); //kopierer name til dette minneomeraade
}

void legg_til_kobling(int ID1, int ID2){
    NODE *R1 = getNode(ID1);
    NODE *R2 = getNode(ID2);
    if(R1 == NULL || R2 == NULL)
        return;
    for(int i = 0; i < R1->x; i++){
        if(R1->adj[i]->nodeID == ID2){
            printf("Kobling %d --> %d finnes fra før", ID1, ID2);
            return; 
        } 
    }
    R1->adj[R1->x] = R2;
    R1->x += 1;
}

void remove_connection(int ID){
    for(int i = 0; i<N; i++){
        for(int j = 0; j < nodes[i].x; j++){
            if(nodes[i].adj[j]->nodeID == ID){
               for(int k = j; k < nodes[i].x - 1; k++){
                   nodes[i].adj[k] = nodes[i].adj[k + 1]; 
               }
              nodes[i].x--; 
            } 
        }
    }
}

void slett_ruter(int ID){
    remove_connection(ID); // Fjern alle koblinger til noden
    NODE *tmp = malloc(sizeof(NODE)*(N-1));
    if(tmp == NULL){
        printf("Error allocating memory");
        exit(-1);
    }
    int index = -1;
    for(int i = 0; i < N; i++){
        if(nodes[i].nodeID == ID){
            index = i;
            freeAdj(ID);
        }
    }
    if(index == -1){ //Hvis noden ikke eksisterer
        free(tmp);
        return;
    }
    if(index != 0)
        memcpy(tmp, nodes, index*sizeof(NODE)); //kopier alt før indeks
    if(index != (N-1)){
        memcpy(tmp + index, nodes+index+1, (N - index - 1) * sizeof(NODE)); //kopier alt etter indeks
    }
    N--;
    cpy = nodes; //Lar cpy peke på nodes slik at det kan frigjøres mot slutten, jeg får flere errors hvis jeg prøver å frigjøre det her.
    nodes = tmp; //Lar nodes peke til det oppdaterte arrayet 
}

void finnes_rute(int R1_ID, int R2_ID){
    int visited[N];
    for(int i = 0; i < N; i++){
        visited[i] = 0;
    }
   if(DFS(R1_ID, R2_ID, visited) == 0){
       printf("\nFant en sti mellom %d og %d\n", R1_ID, R2_ID);
   }else{
       printf("\nFant ingen sti mellom %d og %d\n", R1_ID, R2_ID);
   }
}

int DFS(int ID, int R2_ID, int *visited){
    if(ID == R2_ID){
        return 0;
    }
    NODE *R1 = getNode(ID);
    if(R1 == NULL)
        return 1;
    memset(visited + getIndex(R1->nodeID), 1, sizeof(int)); 
    for(int i = 0; i < R1->x; i++){
        int index = getIndex(R1->adj[i]->nodeID);
        if(*(visited+index) == 0){
            int out = DFS(R1->adj[i]->nodeID, R2_ID, visited);
            if(out == 0){
                return 0;
            }
        }
    }
    return -1;
}

void printBin(char a){ //printer char i binary form
    int i;
    for(i = 0; i < 8; i++){
        printf("%d", !!((a << i) & 0x80));
    }
    printf("\n");

}

int getIndex(int ID){ //returnerer indeks til gitt nodeID i arrayet
for(int i = 0; i < N; i++){
    if(nodes[i].nodeID == ID){
        return i;
    }
}
printf("Noden %d finnes ikke", ID);
return -1;
}

void run_command(char *line){
    char func[40];
    sscanf(line, "%s", func);
    if(strcmp(func, "print") == 0){
        int param1;
        sscanf(line, "%*s%d", &param1);
        print(param1);
    }else if((strcmp(func, "sett_modell") == 0)){
        int param1; 
        char temp[249];
        char *param2 = temp; //Bruker pekeren for aa fjerne space forran param2 ved bruk av ++t
        sscanf(line, "%*s%d", &param1);
        sscanf(line, "%*s%*d%[^\n]", temp);
        sett_modell(param1, ++param2);
    }else if((strcmp(func, "sett_flagg") == 0)){
        int param1, param2, param3;
        sscanf(line, "%*s%d", &param1);
        sscanf(line, "%*s%*d%d", &param2);
        sscanf(line, "%*s%*d%*d%d", &param3);
        sett_flagg(param1, param2, param3);
    }else if((strcmp(func, "finnes_rute") == 0)){
        int param1, param2;
        sscanf(line, "%*s%d", &param1);
        sscanf(line, "%*s%*d%d", &param2);
        finnes_rute(param1, param2);
    }else if((strcmp(func, "legg_til_kobling") == 0)){
         int param1, param2;
         sscanf(line, "%*s%d", &param1);
         sscanf(line, "%*s%*d%d", &param2);
         legg_til_kobling(param1, param2);
    }else if((strcmp(func, "slett_ruter") == 0)){
         int param1; 
         sscanf(line, "%*s%d", &param1);
         slett_ruter(param1); 
    }
}

void skriv_til_fil(FILE *fd){
    if(N == 0){
        printf("Ingen rutere å skrive til");
    }
    fd = fopen("new-topology.dat", "wb");
    fwrite(&N, sizeof(int), 1, fd);
    for(int i = 0; i < N; i++){
        fwrite(&(nodes[i].nodeID), sizeof(int), 1, fd);
        fwrite(&(nodes[i].flag), 1, 1, fd);
        fwrite(&(nodes[i].length), 1, 1, fd);
        fwrite(&(nodes[i].model), 1, nodes[i].length, fd);
        fputc(0, fd);
    }
    for(int i = 0; i < N; i++){
        for(int j = 0; j < nodes[i].x; j++){
            fwrite(&(nodes[i].nodeID), sizeof(int), 1, fd); 
            fwrite(&(nodes[i].adj[j]->nodeID), sizeof(int), 1, fd); 
        }
    }
    fclose(fd);
}
