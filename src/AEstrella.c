//MARC ROIG OLIVA 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <float.h>


//DEFINICIO DE CONSTANTS NECESSARIES

#define R 6371000 //Radi de la terra en metres
#define DEG2RAD (3.141592653589793 / 180)
#define MAXAR 20
#define MAX 1000000000

//STRUCTS

typedef struct Element
{
    int node;
    struct Element *seg;
} ElementCua;

typedef struct
{
    ElementCua *inici, *final;
} UnaCua;

typedef struct
{
    char carrer[12];
    int numnode;
    double llargada;
} infoaresta;

typedef struct
{
    long int id;
    double latitud, longitud;
    int numarst;
    double pes, dist_origen, dist_desti;
    int anterior;
    infoaresta *arestes;
} Node;

//PROTOTIPUS DE FUNCIONS

unsigned buscapunt(Node *, int, long int);

double distancia(Node, Node);

void encua(UnaCua *, unsigned);

int indexoptim(UnaCua *, Node *);

void desencua(UnaCua *, Node *);

int esAlaCua(UnaCua *, int);

void mostracami(Node * , int, long int, long int , int , int);


int main(int argc, char **argv){

    int nnodes, ncarrers, c;
    long int IDsortida, IDdesti;
    int IDXsortida, IDXdesti;

    FILE *dadesNodes, *dadesCarrers;
    ElementCua * actual;

    //VALIDACIO I EMMAGATZEMATGE DE NODES
    if (argc == 3) {
        sscanf(argv[1], "%ld", &IDsortida);           
        sscanf(argv[2], "%ld", &IDdesti); 
    } else {
        printf("El programa necessita 2 arguments i s'han introduit %d", argc-1);
        return -1;
    }

    //RESERVES DE MEMORIA
    if ((dadesNodes = fopen("Nodes.csv", "r")) == NULL)
    {
        printf("\nNo s'ha accedit al fitxer de dades Nodes.csv\n");
        return 1;
    }

    if ((actual = (ElementCua *)malloc(sizeof(ElementCua))) == NULL)
    {
        printf("\nNo es possible assignar la memòria necessària.\n\n");
        return 1;
    }

    //LECTURA DEL FITXER DE NODES
    nnodes = 0;
    while ((c = fgetc(dadesNodes)) != EOF){
        if (c == '\n') nnodes++;
    }

    Node * punts;

    if ((punts= (Node *)malloc(nnodes * sizeof(Node))) == NULL){
        printf("Error malloc nodes\n");
        return 2;
    }

    rewind(dadesNodes);
    
    
    for (int i = 0; i < nnodes; i++){
        
        punts[i].id = 0;
        punts[i].latitud = 0;
        punts[i].longitud = 0;
        punts[i].numarst = 0;
        
        if ((punts[i].arestes = (infoaresta *)malloc(MAXAR * sizeof(infoaresta))) == NULL){
            printf("Error malloc arestes\n");
            return 2;
        }
    }
    
    for (int i = 0; i < nnodes; i++){
        fscanf(dadesNodes, "%ld;", &punts[i].id);
        fscanf(dadesNodes, "%lf;", &punts[i].latitud);
        fscanf(dadesNodes, "%lf\n;", &punts[i].longitud);
    }
    fclose(dadesNodes);

    //LECTURA DEL FITXER DE CARRERS
    if ((dadesCarrers = fopen("Carrers.csv", "r")) == NULL){
        printf("No es possible obrir el fitxer de dades Carrers.csv\n");
        return 1;
    }

    ncarrers = 0;
    while ((c = fgetc(dadesCarrers)) != EOF)
    {
        if (c == '\n')
            ncarrers++;
    }

    rewind(dadesCarrers);
    char IDcarrer[12];
    long int nodeID;

    //no funciona el de la practica 9 cambiar
    while ((c=fgetc(dadesCarrers))!=EOF){
        fscanf(dadesCarrers,"d=%[0-9]", IDcarrer);
        fscanf(dadesCarrers,";%ld",&nodeID);
        int posant = buscapunt(punts, nnodes, nodeID);

        while (posant == MAX && fgetc(dadesCarrers) != '\n'){
            printf("%010ld no existeix\n", nodeID); //getchar();
            fscanf(dadesCarrers,"%ld",&nodeID); //Con el & funciona en Windows
            posant = buscapunt(punts, nnodes, nodeID);
        }

        while (fgetc(dadesCarrers)!='\n'){
            fscanf(dadesCarrers,"%ld",&nodeID);
            int pos = buscapunt(punts, nnodes, nodeID);
            while ((pos == MAX) && (fgetc(dadesCarrers) != '\n')){
                printf("%010ld no existeix\n", nodeID);
                fscanf(dadesCarrers,"%ld",&nodeID);
                pos = buscapunt(punts, nnodes, nodeID);
            }
            if (pos < MAX){
                strcpy(punts[pos].arestes[punts[pos].numarst].carrer, IDcarrer);
                punts[pos].arestes[punts[pos].numarst].numnode = posant;
                punts[pos].numarst++;
                strcpy(punts[posant].arestes[punts[posant].numarst].carrer, IDcarrer);
                punts[posant].arestes[punts[posant].numarst].numnode = pos;
                punts[posant].numarst++;
            }
            posant = pos;
        }
    }
    fclose(dadesCarrers);

    UnaCua nodesCua;
    int IDXactual;
    double distAux;

    //VALIDACIO DELS NODES DE SORTIDA I DESTI
    if (!(IDXsortida = buscapunt(punts, nnodes, IDsortida)))
    {
        printf("El node de sortida no és valid\n");
        return -1;
    }
    if (!(IDXdesti = buscapunt(punts, nnodes, IDdesti)))
    {
        printf("El node de desti no és valid\n");
        return -1;
    }

    //INICIALITZACIO DE VARIABLES I DE LA CUA
    actual->node = IDXsortida;
    actual->seg = NULL;
    nodesCua.inici = actual;
    nodesCua.final = actual;

    for (int i=0; i<nnodes; i++){
        punts[i].pes = MAX;
        punts[i].dist_origen = MAX;
        punts[i].dist_desti = distancia(punts[i], punts[buscapunt(punts, nnodes, IDXdesti)]);
        punts[i].anterior = -1;
    }

    punts[IDXsortida].dist_origen = 0;
    punts[IDXsortida].pes = 0;

    //CERCA DEL CAMI MES OPTIM AMB A*

    while (nodesCua.inici != NULL){
        
        IDXactual = indexoptim(&nodesCua, punts);
        if (punts[IDXactual].id == IDdesti){
            break;
        }
        desencua(&nodesCua, punts);
        int next;
        for (int i = 0; i < punts[IDXactual].numarst; i++){
            next = punts[IDXactual].arestes[i].numnode;
            distAux = punts[IDXactual].dist_origen + distancia(punts[IDXactual], punts[next]);
            if (distAux < punts[next].dist_origen){
                punts[next].anterior = IDXactual;
                punts[next].dist_origen = distAux;
                punts[next].pes = punts[next].dist_origen + punts[next].dist_desti;

                if (!esAlaCua(&nodesCua, next)) encua(&nodesCua, next);
            }
        }
    }

    mostracami(punts, IDXactual, IDsortida, IDdesti, IDXsortida, IDXdesti);
}

void mostracami(Node * punts, int IDXac, long int IDorigen, long int IDdesti, int IDXorigen, int IDXdesti){

    if (punts[IDXac].id != IDdesti){
        printf("No s'ha trobat cap camí possible des de %ld fins a %ld", IDorigen, IDdesti);
    } else {
        int n = 0, idx = IDXac;

        while (punts[IDXac].anterior != -1){
            n++;
            IDXac = punts[IDXac].anterior;
        }

        int * recorregut;
        if ((recorregut = (int *)malloc((n+1) * sizeof(int))) == NULL){
            printf("Error malloc recorregut");
            return;
        }
        int act = 0;
        while (punts[idx].anterior != -1){
            recorregut[act] = idx;
            act++;
            idx = punts[idx].anterior;
        }
        recorregut[0] = IDXorigen;
        int idxaux = IDXdesti;
        for (int i=0; i<n-1;i++){
            recorregut[i] = punts[idxaux].anterior;
            idxaux = punts[idxaux].anterior;
        }
        recorregut[n] = IDXorigen;
        printf("# La distància de %ld a %ld es de %f metres\n#Cami optim:\n", IDorigen, IDdesti, punts[IDXdesti].dist_origen);
        for (int i = n; i >= 0; i--){
            printf("Id=%010ld | %f | %f | Dist=%f\n", punts[recorregut[i]].id, punts[recorregut[i]].latitud, punts[recorregut[i]].longitud, punts[recorregut[i]].dist_origen);
        }
    }
}

unsigned buscapunt(Node * llistaPunts, int nnodes, long int ident)
{
    int minim = 0, maxim = nnodes, arrel;
    do
    {
        arrel = (minim + maxim) / 2;
        if (llistaPunts[arrel].id < ident)
        {
            minim = arrel + 1;
        }
        else if (llistaPunts[arrel].id > ident)
        {
            maxim = arrel;
        }
    } while (llistaPunts[arrel].id != ident && minim != maxim);
    return arrel;
}

double distancia(Node node1, Node node2)
{
    double x1, x2, y1, y2, z1, z2;

    x1 = R * cos(node1.longitud * DEG2RAD) * cos(node1.latitud * DEG2RAD);
    y1 = R * sin(node1.longitud * DEG2RAD) * cos(node1.latitud * DEG2RAD);
    z1 = R * sin(node1.latitud * DEG2RAD);

    x2 = R * cos(node2.longitud * DEG2RAD) * cos(node2.latitud * DEG2RAD);
    y2 = R * sin(node2.longitud * DEG2RAD) * cos(node2.latitud * DEG2RAD);
    z2 = R * sin(node2.latitud * DEG2RAD);

    return sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));
}

void encua(UnaCua *cua, unsigned n)
{
    ElementCua *nou;
    if ((nou = (ElementCua *)malloc(sizeof(ElementCua))) == NULL)
    {
        printf("\nNo es possible assignar la memòria necessòria.\n\n");
        return;
    }

    nou->node = n;
    nou->seg = cua->inici;
    cua->inici = nou;
}

int indexoptim(UnaCua *cua, Node *punts)
{
    ElementCua *actual = cua->inici;
    int index;
    double pes = MAX;
    while (actual != NULL)
    {
        if (punts[actual->node].pes < pes)
        {
            index = actual->node;
        }
        actual = actual->seg;
    }
    return index;
}

void desencua(UnaCua *cua, Node *punts)
{

    int iminf;
    iminf = indexoptim(cua, punts);
    ElementCua *temp = cua->inici, *anterior;

    if (temp != NULL && punts[iminf].id == punts[temp->node].id)
    {
        cua->inici = temp->seg;
        free(temp);
        return;
    }

    while (temp != NULL && punts[iminf].id != punts[temp->node].id)
    {
        anterior = temp;
        temp = temp->seg;
    }

    if (temp == NULL)
    {
        return;
    }

    anterior->seg = temp->seg;
    free(temp);
}

int esAlaCua(UnaCua *cua, int ivei)
{

    if (cua->inici == NULL)
    {
        return 0;
    }
    ElementCua *actual = cua->inici;
    while (actual->seg != NULL)
    {
        if (actual->node == ivei)
        {
            return 1;
        }
        actual = actual->seg;
    }
    return 0;
}
