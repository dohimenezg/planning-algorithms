/*
 Simulador de planificacion de CPU para un solo procesador.
 Derechos Reservados de Erwin Meza Vega <emezav@gmail.com>.
 
 Presentado por: 
 
 estudiante1 codigo correo
 estudiante1 codigo correo 
 
 IMPORTANTE
 Este código se proporciona como una guía general para implementar
 el simulador de planificación. El estudiante deberá verificar su 
 funcionamiento y adaptarlo a las necesidades del problema a solucionar.
 
 El profesor no se hace responsable por las omisiones, los errores o las 
 imprecisiones que se puedan encontrar en este código y los archivos relacionados.
 
 USO:
 
 ./nombre_ejecutable archivo_configuracion
 ./nombre_ejecutable < archivo_configuracion
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "split.h"
#include "list.h"

/** @brief Transforma los caracteres dentro de una cadena de minusculas a mayusculas. */
char * lcase(char *str);

/** @brief Macro para determinar si dos cadenas son iguales */
#define equals(x, y) (strcmp(x, y) == 0)

/* Tipos de enumeracion*/

/** @brief Estrategia de planificacion */
enum strategy {FIFO, SJF, SRT, RR};

/** @brief Estado de un proceso */
enum status { UNDEFINED, LOADED, READY, RUNNING, FINISHED};

/** Tipos de instruccion (solo CPU) */
enum type {CPU};

/** @brief Instruccion dentro de un programa */
typedef struct {
     enum type type;
     int time;
     int remaining;
} instruction; 

/** @brief Elemento de la secuencia para la simulacion */
typedef struct {
        char * name;
        int time;
}sequence_item;

/** @brief Definicion de un proceso */
typedef struct {
       char name[80];
       int priority;
       int arrival_time;      
       int waiting_time;
       int finished_time;
       enum status status;
       list * instructions;
       node_iterator thread;
} process;

/* Estructuras de las listas de procesos */
typedef struct {
       int quantum;
       enum strategy strategy;
       list *ready;
       list *arrival;
       list *finished;
} priority_queue;


/** @brief Crea una nueva lista de colas de prioridad
 * @param int Cantidad de colas de prioridad
*/
priority_queue * create_queues(int);

/** @brief Imprime la informacion de una cola de prioridad */
void print_queue(priority_queue *);

/** @brief Retorna el numero de procesos listos en una cola de prioridad
 * @param priority_queue * Apuntador a la lista de colas de prioridad
 * @param int Numero total de colas de prioridad
*/
int get_ready_count(priority_queue *, int);

/*@brief Retorna el tiempo en el cual se presenta la nueva llegada a la cola 
de listos de una cola de prioridad */
int get_next_arrival(priority_queue *, int);

/** @brief Procesa la llegada de procesos en el tiempo especificado */
int process_arrival(int, priority_queue *, int);

/** @brief Imprime la informacion de un proceso */
void print_process(process *);

/** @brief Crea un proceso a partir de la linea de configuracion */
process * create_process(char *);

/** @brief Fusiona instrucciones similares */
void merge_instructions(process *);

/** @brief Re-inicia un proceso*/
void restart_process(process *);

/** @brief Funcion que compara dos tiempos de llegada */
int compare_arrival(const void *, const void *);

/** @brief Funcion que permite planificar los procesos*/
void schedule(list*, priority_queue *, int );

/** @brief Prepara el entorno para la simulacion. */
void prepare(list *, priority_queue *, int );

/** @brief Imprime la ayuda. */
void usage(void);
void usage_en(void);


/* Programa principal*/
int main(int argc, char * argv[]) {
    char linea[80];
    FILE *fd;
    split_list * t;
    char ** aux;
    int i;
    int quantum;
    int simulated = 0;

    int nqueues;
    
  
    /* Referencia al arreglo de colas de prioridad */  
    priority_queue * queues;
    
    /* Lista de procesos */
    list * processes;

    /* Referencia a un proceso */
    process * p;

    printf("Scheduler start.\n");

    printf("Type 'help' for help.\n");
    
    /* Leer el archivo de configuracion, pasado como parametro al main
    o por redireccion de la entrada estandar */
    
    if (argc < 2) {
       fd = stdin;
    }else {
          fd = fopen(argv[1], "r");
          if (!fd) {fd = stdin;}
    }
    
    nqueues = 0;

    /* Lectura del archivo de configuracion */
    while (!feof(fd)) {
          
         strcpy(linea, "" );
         fgets(linea, 80, fd);

         //printf("Linea leida: %s", linea);

         if (strlen(linea) <= 1) {continue;}
         if (linea[0] == '#') {continue;}          
         
         //Convertir en minusculas
         lcase(linea);
    
          // Partir la linea en tokens
          t = split(linea, 0);
          
          //Ignora las lineas que no contienen tokens
          if (t->count == 0) { continue;}
          
          //Procesar cada linea, aux apunta al arreglo de tokens
          aux = t->parts;

          if (equals(aux[0], "exit")) {
             exit(EXIT_SUCCESS);
          }
          else if (equals(aux[0], "help")) {
             usage();
          }
          else if (equals(aux[0], "help_en")) {
             usage_en();
          }
          else if (equals(aux[0], "define") && t->count >= 3) {
                //Comando define queues n   
               if (equals(aux[1], "queues")) {                     
                  nqueues = atoi(aux[2]);
                  //Crear el arreglo de colas de prioridad y a lista de procesos
                  if (nqueues > 0) {
                     queues = create_queues(nqueues);

                     processes = create_list();
                  }
                  simulated = 0;
               }else if (equals(aux[1], "scheduling") && t->count >= 4){
                    //Comando scheduling n ESTATEGIA
                    //n = 1 ... # de colas de prioridad
                    
                    //i = n - 1, los arreglos comienzan en cero
                    i = atoi(aux[2]) - 1;
                    //printf("Defining scheduling to queue %d\n", i);
                    if (i < 0 || i >= nqueues) {continue;}
                    if (equals(aux[3], "rr")) {
                       queues[i].strategy = RR;
                    }else if (equals(aux[3], "sjf")) {
                       queues[i].strategy = SJF;
                    }
                    else if (equals(aux[3], "fifo")) {
                       queues[i].strategy = FIFO;
                    }
               }else if (equals(aux[1], "quantum") && t->count >= 4){
                    //Comando scheduling n QUANTUM
                    //n = 1 ... # de colas de prioridad
                    i = atoi(aux[2]) - 1;
                    //printf("Defining quantum to queue %d\n", i);
                    quantum = atoi(aux[3]);
                    if (i < 0 || i >= nqueues) {continue;}
                    queues[i].quantum = quantum;
               }
          }
          else if (equals(aux[0], "process")&& t->count >= 3){
                //Comando process FILENAME PRIORITY
                //printf("process %s\n", aux[1]);
                p = create_process(aux[1]);
                
                if (p != 0) {
                  i = -1;
                  i = atoi(aux[2]);
                  if (i <= 0 || i > nqueues) {
                     printf("Queue %d does not exist", i);
                     continue;
                  }
                  p->priority = i - 1;
                   //printf("Inserting process %s\n", p->name);
                   //Insertar el proceso en la lista general de procesos
                   insert_ordered(processes, p, compare_arrival);
                }
                //print_process(p);                
          }
          else if (equals(aux[0], "start")) {
                //Comando start
                //Comenzar la simulacion!!! 

                schedule(processes, queues, nqueues);
                
                simulated = 1;

                //system("pause");
          }                  
    }
}


/* Funcion para crear las colas de prioridad */
priority_queue * create_queues(int n) {
      priority_queue * ret;
      int i;
      
      ret = (priority_queue *)malloc(sizeof(priority_queue) * n);      
      
      for (i=0; i<n; i++) {
          ret[i].strategy = RR;   //Por defecto RR
          ret[i].quantum =  0;
          ret[i].arrival = create_list();
          ret[i].ready = create_list();
          ret[i].finished = create_list();
      }      

      return ret;
      
}

/*Rutina para imprimir una cola de prioridad */
void print_queue(priority_queue *queue) {
  int i;
  node_iterator  ptr;

  printf("%s q=", 
      (queue->strategy == RR)?"RR":((queue->strategy == FIFO)?"FIFO":((queue->strategy == SJF)?"SJF":"UNKNOWN")));
  printf("%d ", queue->quantum);

  printf("ready (%d): { ", queue->ready->count);

  for (ptr = head(queue->ready); ptr != 0; ptr= next(ptr)) {
    print_process((process*)ptr->data);
  }

  printf("} ");

  printf("arrival (%d): { ", queue->arrival->count);

  for (ptr = head(queue->arrival); ptr != 0; ptr= next(ptr)) {
    print_process((process*)ptr->data);
  }

  printf("} ");

  printf("finished (%d): { ", queue->finished->count);

  for (ptr = head(queue->finished); ptr != 0; ptr= next(ptr)) {
    print_process((process*)ptr->data);
  }
  printf("}\n");
}

/* Comparar dos procesos por tiempo de llegada */
int compare_arrival(const void *a, const void *b) {
    process *p1;
    process *p2;

    p1 = (process *) a;
    p2 = (process *) b;

    //printf("Comparing %s to %s : %d %d\n", p1->name, p2->name, p1->arrival_time, p2->arrival_time);

    return p2->arrival_time - p1->arrival_time;
}

/* Rutina para leer la informacion de un proceso */
process * create_process(char *filename) {
        FILE *fd;
        char linea[80];
        split_list *t;
        char ** aux;

        instruction *ins;
        
        process * ret;

        ret = 0;

        fd = fopen(filename, "r");
        if (!fd) {
           printf("file %s not found\n", filename);
           return ret;
        }

           ret = (process *) malloc(sizeof(process));
           strcpy(ret->name, filename);
           ret->arrival_time = -1;
           ret->priority = -1;
           ret->waiting_time = -1;
           ret->finished_time = -1;
           ret->status = UNDEFINED;
           ret->instructions = create_list();
           ret->thread = 0;

           while (!feof(fd)) {
                 fgets(linea, 80, fd);
                 lcase(linea);
                 if (strlen(linea) <= 1) {continue;}
                 if (linea[0] == '#') {continue;}
                 t = split(linea, 0);
                 if (t->count <= 0) {continue;} 
                 aux = t->parts;

                 if (equals(aux[0], "begin")&& t->count >= 2) {
                    ret->arrival_time  = atoi(aux[1]);
                 }else if(equals(aux[0], "cpu")) {
                       ins = (instruction *)malloc(sizeof(instruction));
                       ins->type = CPU;                       
                       ins->time = atoi(aux[1]);
                       push_back(ret->instructions, ins);                       
                       //printf("cpu %s\n", aux[1]);
                 }else if(equals(aux[0], "end")) {
                       //printf("End!\n");
                       break;
                 }
           }
           merge_instructions(ret);
           fclose(fd);
        //print_process(ret);

        return ret;       

}

void merge_instructions(process *p) {
     node_iterator ant;
     node_iterator aux;

     instruction *ins;
     instruction *ins_next;

     ant = 0;
     aux = head(p->instructions);
     while (aux != 0 && next(aux) != 0) {
           ins = (instruction*)aux->data;
           ins_next = (instruction*)next(aux)->data;
           //printf("\tComparando %d %d con %d %d\n", ins->type, ins->time, ins_next->type, ins_next->time);
           if (ins->type == ins_next->type) { //Nodos iguales, fusionar y restar 1 al conteo de nodos
              //printf("\t\t%d %d y %d %d son iguales\n", ins->type, ins->time, ins_next->type, ins_next->time);
              ins->time = ins->time + ins_next->time;
              aux->next = next(aux)->next;
              if (aux->next != 0) {
                 aux->next->previous = aux;
              }              
              p->instructions->count = p->instructions->count - 1;
              if (aux->next == p->instructions->tail) {
                 p->instructions->tail = aux;
              }
           }else {
               aux = next(aux);
           }
     }
}

/* Rutina para re-iniciar un proceso */
void restart_process(process *p) {
     
     node_iterator n;
     instruction *it;
     
     p->waiting_time = -1;
     p->finished_time = -1;
     p->status = LOADED;
     p->thread = head(p->instructions);

     for (n = head(p->instructions); n!= 0; n = next(n)) {
         it = (instruction*)n->data;
         it->remaining = it->time;
     }     
}

/* Rutina para imprimir un proceso */
void print_process(process *p) {
  if (p == 0) {return;}
  node_iterator ptr;
  instruction *ins;
  printf("(%s arrival:%d finished:%d waiting:%d ", 
      p->name, p->arrival_time, p->finished_time, p->waiting_time);
  //UNDEFINED, LOADED, READY, RUNNING, FINISHED
  printf("%s ", (p->status == READY)?"ready":(p->status==LOADED)?"loaded":(p->status == FINISHED)?"finished":"unknown");
  printf("%d:[ ", p->instructions->count);    

  for ( ptr = head(p->instructions); ptr !=0; ptr = next(ptr)) {
    ins = (instruction *)ptr->data;
    printf("%s %d ",(ins->type == CPU)?"CPU":"LOCK", ins->time);
  }
  printf("]) ");
}
/* Rutina para convertir una cadena en minusculas */
char * lcase(char * s) {
     char * aux;

     aux = s;
     while (*aux != '\0' ) {
           if (isalpha(*aux) && isupper(*aux)) {
              *aux = tolower(*aux);
           }
           aux++;
     }
     return s;
}

void prepare(list * processes, priority_queue *queues, int nqueues) {
   int i;
   process *p;
   node_iterator it;

   /* Limpiar las colas de prioridad */

   for (i=0; i<nqueues; i++) {
       //printf("Clearing queue %d\n", i);
       if (queues[i].ready != 0) {
          clear_list(queues[i].ready);
          queues[i].ready = create_list();
       }
       if (queues[i].arrival != 0) {
          clear_list(queues[i].arrival);
          queues[i].arrival = create_list();
       }
       if (queues[i].finished != 0) {
          clear_list(queues[i].finished);
          queues[i].finished = create_list();
       }
   }

   /* Inicializar la informacion de los procesos en la lista de procesos */  
   
   for (it = head(processes); it != 0; it = next(it)) {
       p = (process *)it->data;
       restart_process(p);

       insert_ordered(queues[p->priority].arrival, p, compare_arrival);                    
   }

   printf("Prepared queues:\n");
   for (i=0; i<nqueues; i++) {
        print_queue(&queues[i]);
   }

   system("pause");
      
}

/* Procesa la llegada de procesos  a una cola de prioridad */
int process_arrival(int now, priority_queue *queues, int nqueues) {
     int i;
     process *p;
     int finished;
     int total;

    //printf("Process arrival at %d\n", now);
     
     total = 0;
     for (i=0; i<nqueues; i++) {
         //printf("Queue %d\n", i);
         finished = 0;
         
         while (finished == 0) {
             p = front(queues[i].arrival); 
             if (p == 0) {
                finished = 1;
                continue;
             } //Cola vacia, pasar a la siguiente cola             
             if (p->status == LOADED) { //Es un proceso nuevo?
                if (p->arrival_time <= now) { //Es hora de llevarlo a ready?
                   p->status = READY;
                   p->waiting_time = now - p->arrival_time;
                   push_back(queues[i].ready, p);
                   pop_front(queues[i].arrival);
                }else { //Terminar de procesar esta cola
                      finished = 1;
                }   
             }else { //??
                   finished = 1;
             }
         }                 
     }       

     //Retorna el numero de procesos que se encuetran en estado de
     //listo en todas las colas de prioridad
     return get_ready_count(queues, nqueues);
}

/* Retorna el tiempo en el cual se presenta la nueva llegada a la cola 
de listos de una cola de prioridad */
int get_next_arrival(priority_queue * queues, int nqueues) {
    int ret;
    process *p;
    int i;

    ret = INT_MAX;

    for (i=0; i<nqueues; i++) {         
         //Revisar el primer proceso en la lista arrival
         //este tiene el menor tiempo de llegada.
             p = front(queues[i].arrival); 
             if (p != 0) {
                if (p->status == LOADED && p->arrival_time < ret) {
                   ret = p->arrival_time;
                }
             }
     }

     //printf("Next arrival : %d\n", ret);

     if (ret == INT_MAX) {
        ret = -1;
     }
    
    return ret;
}

/* Retorna el numero de procesos listos en una cola de prioridad */
int get_ready_count(priority_queue *queues, int nqueues){
    int ret;
    int i;

    ret = 0;

    for (i=0; i<nqueues; i++) {   
        ret = ret + queues[i].ready->count;             
     }

    return ret;
}

/* Rutina para la planificacion.*/
void schedule(list * processes, priority_queue *queues, int nqueues) {
    
    
    int finished;
    int nprocesses;
    
    
    //Preparar para una nueva simulacion
    //Inicializar las colas de prioridad con la informacion de la lista
    //de procesos leidos
    prepare(processes, queues, nqueues);
    
    
    //Numero de procesos que falta por ejecutar     
    nprocesses = processes->count;
    
     printf("TODO: Implementar la planificacion!!\n");
     
      
      print_queue(&queues[0]);
    
    /*
     while (nprocesses > 0) {    
        // TODO: Implementar la planificaci�n
         
        //Cuando un proceso termina, decrementar nprocesses.
        //El ciclo termina cuando todos los procesos han terminado,
        //es decir nprocesses = 0 
     }
     */    
    
    //Imprimir la salida del programa
    
   
}


void usage(void) {
   printf("Simulador de planificacion.\n");
   printf("Este programa simula el comportamiento de los algoritmos: FIFO, SJF, SRT, RR.\n");
   printf("Los parametros para el simulador se pueden ingresar por entrada estandar o un archivo de configuracion\n");
   printf("El formato del archivo de configuracion especificado por linea de comandos o de forma interactiva por entrada estandar es:\n");
   printf("   DEFINE\n");
   printf("         Define un parametro de simulacion. Debe ser seguido por una de las siguientes opciones:\n");
   printf("         QUEUES N                             : Numero de colas de prioridad >= 1\n");
   printf("         SCHEDULING i FIFO|SJF|SRT|RR         : Algoritmo de planificacion para la cola i\n");
   printf("         QUANTUM i t                          : Unidades de tiempo (quantum) asignado a la cola i\n");
   printf("         PROCESS filename k                   : Proceso a simular en la cola k (vea a continuacion para el formato del archivo\n");
   printf("   START\n");
   printf("         Realiza la simulacion.\n");
   printf("   HELP\n");
   printf("         Imprime esta ayuda.\n");
   printf("   HELP_EN\n");
   printf("         Imprime esta ayuda en Ingles.\n");
   printf("   EXIT\n");
   printf("         Termina el simulador.\n");

   printf("   FORMATO DE LOS ARCIVOS DE PROCESOS.\n");
   printf("         Un programa es un archivo de texto con el siguiente formato:\n");
   printf("         BEGIN t                              : Tiempo de llegada del proceso\n");
   printf("         CPU t                                : Instruccion para usar la CPU. Pueden especificarse varias instrucciones CPU seguidas.\n");
   printf("         END                                  : Fin del programa.\n");

}

void usage_en(void) {
   printf("Scheduling simulator.\n");
   printf("This program simulates the most basic scheduling algorithms: FIFO, SJF, SRT, RR.\n");
   printf("The parameters for the simulator can be entered via standard input, or via a configuration file.\n");
   printf("The format of the configuration file or the parameters specified via stdin are:\n");
   printf("   DEFINE\n");
   printf("         Defines a simulation parameter. Must be followed by one of the following options:\n");
   printf("         QUEUES N                             : Number of priority queues >= 1\n");
   printf("         SCHEDULING i FIFO|SJF|SRT|RR         : Scheduling algorithm for queue i\n");
   printf("         QUANTUM i t                          : Time units (t) of the quantum for queue i\n");
   printf("         PROCESS filename k                   : Process to simulate on priority queue k (see below for the file format)\n");
   printf("   START\n");
   printf("         Starts the simulation.\n");
   printf("   HELP_EN\n");
   printf("         Prints this help.\n");
   printf("   HELP\n");
   printf("         Prints this help in Spanish.\n");
   printf("   EXIT\n");
   printf("         Exits the simulator.\n");

   printf("   PROCESS FILE FORMAT.\n");
   printf("         A program is a text file with the following format:\n");
   printf("         BEGIN t                              : Process arrival time\n");
   printf("         CPU t                                : Execution time instruction. Can be followed by other CPU instructions.\n");
   printf("         END                                  : End of program.\n");

}

