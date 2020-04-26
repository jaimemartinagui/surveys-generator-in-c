#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <stdbool.h>
#include <setjmp.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// Genero una serie de variables globales: el tamano de los strings a emplear y las respuestas para la encuesta
#define TAM 300
char respuestaA[10] = "Muy bien";
char respuestaB[10] = "Bien";
char respuestaC[10] = "Mal";
char respuestaD[10] = "Muy mal";


// Estructura para cada pregunta de la encuesta
typedef struct pregunta{
    char texto[TAM];
    int tamano;
    int numPregunta;
    int respA, respB, respC, respD;
    int iteraciones;
} Pregunta;


struct Nodo{
    Pregunta p;
    struct Nodo *pSig; // Puntero al siguiente nodo de la lista
};


typedef struct Nodo *pNodo;


// Funciones
void menu();
void agregar_preguntas(char *ruta_completa_archivo);
pNodo generarListaCompleta(char *ruta_completa_archivo, pNodo *pCabeza);
void insertar_pregunta(pNodo *inicioLista, Pregunta *p);
void insertar_despues(pNodo pExistente, Pregunta *p);
short exists(char *ruta_completa_archivo);
pNodo generarListaEncuesta(char *ruta_completa_archivo, pNodo *pCabeza, int random_num_list[], int numPreguntas);
void ejecutar_encuesta(pNodo *inicioLista);
int contar_preguntas_totales(pNodo *pCabeza);


int main(){
    
    srand(time(NULL));
    int opcion = 0;
    
    // Primer nodo de la lista con todas las preguntas del fichero
    pNodo pCabeza;
    // Primer nodo de la lista con las preguntas para la encuesta
    pNodo pCabezaEncuesta;
    
    /* Para el correcto funcionamiento de esta aplicacion se debe especificar
       la ruta en la que se encuentra el archivo "Preguntas_Encuesta.txt". Emplee
       para ello la variable denominada "dir_archivo". */
    char dir_archivo[100] = "/Users/jaime/Documents/Proyectos/survey_generator/Surveys-Generator-in-C/";
    char nombre_archivo[100] = "questions.txt";
    char ruta_completa_archivo[150];
    strcat(strcat(ruta_completa_archivo, dir_archivo), nombre_archivo);
    
    int ejecuciones_aplicacion = 0;
    
    while (opcion != 4){
        
        ejecuciones_aplicacion++;
        
        // Se muestra el menu y se pide al usuario elegir una opcion
        menu();
        setbuf(stdin, NULL);
        printf("\nEscoja una opción: ");
        scanf("%i", &opcion);
        
        // ================================= OPCION PARA REALIZAR LA ENCUESTA =================================
        if (opcion == 1){
            // Cargar las preguntas del fichero en una lista enlazada
            // Primero vaciamos las listas
            pCabeza = NULL;
            pCabezaEncuesta = NULL;
            
            // Generamos la lista enlazada con todas las preguntas del fichero
            pCabeza = generarListaCompleta(ruta_completa_archivo, pCabeza);
            pNodo pAux_contar;
            pAux_contar = pCabeza;
            
            // Preguntamos al usuario el numero de preguntas que formaran la encuesta definitiva
            int numPreguntas;
            int numPreguntasTotal;
            numPreguntasTotal = contar_preguntas_totales(&pAux_contar);
            
            setbuf(stdin, NULL);
            printf("\nDe un total de %i preguntas, ¿cuántas quiere incluir en la encuesta?: ", numPreguntasTotal);
            scanf("%i", &numPreguntas);
            printf("\n");
            
            // Muestro un error si se indican mas preguntas de las que hay en la lista
            while (numPreguntas > numPreguntasTotal){
                printf("\nError: Número de preguntadas seleccionado superior al número de preguntas totales indicado\n");
                printf("\nDe un total de %i preguntas, ¿cuántas quiere incluir en la encuesta?: ", numPreguntasTotal);
                scanf("%i", &numPreguntas);
                printf("\n");
            }
            
            /* Genero un vector de numeros aleatorios entre 1 y el numero de preguntas totales. Este vector tendra
               la longitud indicada por el usuario para la encuesta. Los elementos del vector son los indices de las
               preguntas de la lista enlazada de preguntas que se van a emplear en la encuesta (no pueden estar repetidos) */
            int i, j, random_num, repeated_value;
            int lista_indices_preguntas[numPreguntas];
            
            for (i=0; i<numPreguntas; i++){
                repeated_value = 0;
                random_num = rand() % (numPreguntasTotal) + 1;
                if (i == 0)
                    lista_indices_preguntas[i] = random_num;
                else {
                    for (j=0; j<i; j++){
                        if (random_num == lista_indices_preguntas[j]) 
                            repeated_value = 1;
                    }
                    if (repeated_value == 0)
                        lista_indices_preguntas[i] = random_num;
                    else
                        i--;
                }
            }
            
            // Generar nueva lista enlazada con las preguntas que contendra la encuesta, a partir del vector de numero aleatorios 
            pCabezaEncuesta = generarListaEncuesta(ruta_completa_archivo, pCabezaEncuesta, lista_indices_preguntas, numPreguntas);
            
            int numVecesEncuesta = 0;
            
            // Pregunto al usuario cuantas veces quiere realizar la encuesta
            int c;
            while ((c=getchar()) != '\n' && c != EOF){}
            printf("\n¿Cuántas veces quiere realizar la encuesta?: ");
            scanf("%i", &numVecesEncuesta);
            
            pNodo pAux; // Puntero para recorrer la lista y guardar las respuestas a cada pregunta
            for (i=0; i<numVecesEncuesta; i++){
                printf("\n----------------- Encuesta %i -----------------\n", i + 1);
                pAux = pCabezaEncuesta;
                ejecutar_encuesta(&pAux);
            }
            pNodo pRec2;
            pRec2 = pAux;
            printf("\n\nResultados de la encuesta\n");
            printf("-------------------------\n");
            
            FILE* fichero;
            fichero = fopen("resultado_encuesta.txt", "w");
            
            while (pRec2 != NULL){
                printf("\n%s", pRec2->p.texto);
                int resA = (pRec2->p.respA) * 100 / i;
                int resB = (pRec2->p.respB) * 100 / i;
                int resC = (pRec2->p.respC) * 100 / i;
                int resD = (pRec2->p.respD) * 100 / i;
                
                // Muestro los resultados
                printf("A): %i%%\n", resA); printf("B): %i%%\n", resB);
                printf("C): %i%%\n", resC); printf("D): %i%%\n", resD);
                
                // Convierto los valores a string para escribirlos en el fichero
                char resA_str[10]; char resB_str[10];
                char resC_str[10]; char resD_str[10];
                
                sprintf(resA_str, "A) %d%%", resA); sprintf(resB_str, "B) %d%%", resB);
                sprintf(resC_str, "C) %d%%", resC); sprintf(resD_str, "D) %d%%", resD);
                
                // Escribo el fichero con los resultados
                fputs("\n", fichero); fputs(pRec2->p.texto, fichero);
                fputs(resA_str, fichero); fputs("\n", fichero); fputs(resB_str, fichero); fputs("\n", fichero);
                fputs(resC_str, fichero); fputs("\n", fichero); fputs(resD_str, fichero); fputs("\n", fichero);
                
                pRec2 = pRec2->pSig;
            }
            
            fclose(fichero);
        }
        
        // ================================= OPCION PARA AGREGAR PREGUNTAS A LA ENCUESTA =================================
        if (opcion == 2)
            agregar_preguntas(ruta_completa_archivo);
        
        // ================================= OPCION PARA VISUALIZAR LOS ULTIMOS RESULTADOS =================================
        if (opcion == 3){
            if (ejecuciones_aplicacion == 1){
                printf("\nTodavía no hay resultados de ninguna encuesta.\n");
                ejecuciones_aplicacion--;
            } else {
                
                // Abro el fichero de resultados en modo lectura
                FILE *fp;
                fp = fopen("resultado_encuesta.txt", "r");
                
                char pregunta[200];
                
                // Recorro todas las preguntas del fichero
                while (fgets(pregunta, 200, (FILE*) fp)){
                    printf("%s", pregunta);
                }
                // Cierro el fichero
                fclose(fp);
            }
        }
    }
    
    return 0;
}


// ============================================= FUNCIONES =============================================


int contar_preguntas_totales(pNodo *pCabeza){
    int numPreguntasTotal = 0;
    pNodo pRec;
    pRec = *pCabeza;
    while (pRec != NULL){
        numPreguntasTotal++;
        pRec = pRec->pSig;
    }
    
    return numPreguntasTotal;
}


void ejecutar_encuesta(pNodo *inicioLista){
    char caracter_A[] = "A"; char caracter_a[] = "a"; char caracter_B[] = "B"; char caracter_b[] = "b"; 
    char caracter_C[] = "C"; char caracter_c[] = "c"; char caracter_D[] = "D"; char caracter_d[] = "d";
    char respuesta[2];
    int c;
    pNodo pRec;
    pRec = *inicioLista;
    int conta = 0;
    while (pRec != NULL){
        conta++;
        printf("\nPregunta %i\n", conta);
        printf("-----------\n");
        printf("%s\n", pRec->p.texto);
        printf("A): %s\tB): %s\n", respuestaA, respuestaB);
        printf("C): %s\t\tD): %s\n", respuestaC, respuestaD);
        printf("\nIntroduzca su respuesta: ");
        while ((c=getchar()) != '\n' && c != EOF){}
        scanf("%s", &respuesta);
        while (strcmp(respuesta, caracter_A) != 0 && strcmp(respuesta, caracter_a) != 0 && strcmp(respuesta, caracter_B) != 0 && 
               strcmp(respuesta, caracter_b) != 0 && strcmp(respuesta, caracter_C) != 0 && strcmp(respuesta, caracter_c) != 0 && 
               strcmp(respuesta, caracter_D) != 0 && strcmp(respuesta, caracter_d) != 0){
            printf("\nError: respuesta no válida\n");
            printf("\nIntroduzca su respuesta: ");
            getc(stdin);
            scanf("%c", &respuesta[0]);
        }
        
        // Actualizo las respuestas para las estadisticas
        if (strcmp(respuesta, caracter_A) == 0 || strcmp(respuesta, caracter_a) == 0)
            pRec->p.respA++;
        if (strcmp(respuesta, caracter_B) == 0 || strcmp(respuesta, caracter_b) == 0)
            pRec->p.respB++;
        if (strcmp(respuesta, caracter_C) == 0 || strcmp(respuesta, caracter_c) == 0)
            pRec->p.respC++;
        if (strcmp(respuesta, caracter_D) == 0 || strcmp(respuesta, caracter_d) == 0)
            pRec->p.respD++;
        
        pRec = pRec->pSig;
    }
}


// Funcion para agregar preguntas al fichero
void agregar_preguntas(char *ruta_completa_archivo){
    
    // Pregunto al usuario el numero de preguntas a agregar
    int num_preguntas_nuevas = 0, i;
    printf("\n¿Cuántas preguntas desea añadir?: ");
    scanf("%i", &num_preguntas_nuevas);
    setbuf(stdin, NULL);
    
    // Abrimos el archivo en modo escribir sin borrar lo que contiene
    FILE *fp;
    fp = fopen(ruta_completa_archivo, "a+");
    
    for (i=0; i<num_preguntas_nuevas; i++){
        // Limpiamos el buffer del teclado
        setbuf(stdin, NULL);
        char pregunta_nueva[100];
        printf("Escriba la pregunta que desea añadir: ");
        fgets(pregunta_nueva, 100, stdin);
        fputs(pregunta_nueva, fp);
    }
    
    fclose(fp);
}


// Funcion para insertar un elemento a la lista
void insertar_despues(pNodo pExistente, Pregunta *p){
    pExistente->pSig = (pNodo)malloc(sizeof(struct Nodo)); // Se crea el nodo
    pExistente->pSig->p = *p;
    // Se ajustan los punteros
    pExistente->pSig->pSig = NULL;
}


// Funcion para insertar una pregunta a la lista enlazada. Esta funcion recibe
// como parametros un puntero al puntero del primer Nodo de la lista y un puntero a una
// estructura de tipo Pregunta
void insertar_pregunta(pNodo *inicioLista, Pregunta *p){
    pNodo pRec; // Puntero para recorrer la lista
    pRec = *inicioLista; // Inicio el puntero pRec en la Cabeza de lista
    if (pRec){ 
        // Si la lista no esta vacia recorro la lista hasta su ultimo elemento
        while (pRec->pSig != NULL){
            pRec = pRec->pSig;
        }
        // Cuando salgo del bucle while ya estoy situado en el ultimo elemento de la lista
        // Inserto un nodo despues del ultimo, es decir, en la cola
        insertar_despues(pRec, p);
    } else {
        // Si la lista esta vacia, el nuevo elemento se situa en la Cabeza
        // Asigno memoria al puntero de cabeza de lista
        *inicioLista = (pNodo)malloc(sizeof(struct Nodo));
        (*inicioLista)->p = *p;
        (*inicioLista)->pSig = NULL;
    }
}


// Funcion que genera y devulve la lista enlazada con todas las preguntas del fichero
pNodo generarListaCompleta(char *ruta_completa_archivo, pNodo *pCabeza){
    char pregunta[200];
    int num_preguntas = 0;
    Pregunta p;
    
    // Compruebo que el fichero se carga correctamente
    int open_file = exists(ruta_completa_archivo);
    switch (open_file){
        case 0:
            printf("\nArchivo abierto correctamente"); 
            printf("\n-----------------------------\n");  
            // Abro el fichero en modo lectura
            FILE *fp;
            fp = fopen(ruta_completa_archivo, "r");
            
            // Recorro todas las preguntas del fichero
            while (fgets(pregunta, 200, (FILE*) fp)){
                num_preguntas++;
                strcpy(p.texto, pregunta);
                p.tamano = strlen(pregunta);
                p.numPregunta = num_preguntas;
                insertar_pregunta(&pCabeza, &p);
            }
            // Cierro el fichero
            fclose(fp);
            break;
        case -1:
            printf("\nError: No se ha encontrado el fichero en la ruta indicada.\n");
            break;
        case -2:
            printf("\nError al abrir el archivo. Compruebe la ruta indicada al fichero.\n");
            break;
        default:
            break;
    }
    
    return pCabeza;
}


// Funcion que genera y devulve la lista enlazada con las preguntas que se mostraran en la encuesta,
// tomadas del fichero de preguntas mediante el vector aleatorio
pNodo generarListaEncuesta(char *ruta_completa_archivo, pNodo *pCabeza, int random_num_list[], int numPreguntas){
    char pregunta[200];
    int i, num_preguntas_fichero = 0, indice_pregunta = 0;
    Pregunta p;
    
    // Abro el fichero en modo lectura
    FILE *fp;
    fp = fopen(ruta_completa_archivo, "r");
    
    // Recorro todas las preguntas del fichero y me quedo con las que coinciden con los elementos del vector aleatorio
    while (fgets(pregunta, 200, (FILE*) fp)){
        
        num_preguntas_fichero++;
        int coincidencia = 0;
        for (i=0; i<numPreguntas; i++){
            if (num_preguntas_fichero == random_num_list[i])
                coincidencia++;
        }
        if (coincidencia != 0){
            indice_pregunta++;
            strcpy(p.texto, pregunta);
            p.tamano = strlen(pregunta);
            p.numPregunta = indice_pregunta;
            insertar_pregunta(&pCabeza, &p);
        }
    }
    // Cierro el fichero
    fclose(fp);
    
    return pCabeza;
}


// Funcion para comprobar si existe el archivo de preguntas y si se abre correctamente
short exists(char *ruta_completa_archivo){
    int fd = open(ruta_completa_archivo, O_RDONLY);
    if (fd < 0) /* error */
        return (errno==ENOENT)?-1:-2;
    // Si no hemos salido ya, cerramos
    close(fd);
    return 0;
}


void menu(){
    printf("\n");
    printf("============================================\n");
    printf("            GENERADOR DE ENCUESTAS          \n");
    printf("============================================\n");
    printf("1. Iniciar encuesta.                         \n");
    printf("2. Agregar preguntas al fichero de preguntas.\n");
    printf("3. Visualizar el fichero con los últimos resultados. \n");
    printf("4. Salir.                                    \n");
    printf("============================================\n");
}
