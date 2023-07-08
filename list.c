#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Definizione della struttura del nodo
struct Node {
    int type; // 0 per unsigned char[8], 1 per long double
    union {
        unsigned char data1[8];
        long double data2;
    } data;
    struct Node* prev;
    struct Node* next;
    pthread_mutex_t mutex; // Mutex per garantire l'accesso thread-safe al nodo
};

// Struttura della lista collegata
struct LinkedList {
    struct Node* head;
    struct Node* lastAccessed;
    pthread_mutex_t mutex; // Mutex per garantire l'accesso thread-safe alla lista
};

// Struttura per il thread-pool
typedef struct {
    pthread_t* threads;
    int thread_count;
    struct LinkedList* list;
    void (*mapper)(struct Node*);
} ThreadPool;

// Funzione per l'inserimento di un nuovo nodo in coda alla lista
struct Node* insert(struct LinkedList* list, int type, unsigned char data1[8], long double data2) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->type = type;
    if (type == 0) {
        memcpy(newNode->data.data1, data1, sizeof(unsigned char[8]));
    } else {
        newNode->data.data2 = data2;
    }
    newNode->next = NULL;
    pthread_mutex_init(&newNode->mutex, NULL); // Inizializza il mutex del nuovo nodo

    pthread_mutex_lock(&list->mutex); // Acquisisce il mutex della lista

    if (list->head == NULL) {
        newNode->prev = NULL;
        list->head = newNode;
        list->lastAccessed = newNode;

        pthread_mutex_unlock(&list->mutex); // Rilascia il mutex della lista
        return newNode;
    }

    struct Node* current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = newNode;
    newNode->prev = current;
    list->lastAccessed = newNode;

    pthread_mutex_unlock(&list->mutex);
    return newNode;
}


// Funzione per l'inserimento di un nuovo nodo in una posizione specifica
struct Node* insertAT(struct LinkedList* list, int type, unsigned char data1[8], long double data2, int position) {
    if (position < 0) {
        return NULL;
    }

    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->type = type;
    if (type == 0) {
        memcpy(newNode->data.data1, data1, sizeof(unsigned char[8]));
    } else {
        newNode->data.data2 = data2;
    }
    pthread_mutex_init(&newNode->mutex, NULL);

    pthread_mutex_lock(&list->mutex); 

    if (list->head == NULL) {
        newNode->prev = NULL;
        newNode->next = NULL;
        list->head = newNode;
        list->lastAccessed = newNode;

        pthread_mutex_unlock(&list->mutex); 
        return newNode;
    }

    if (position == 0) {
        newNode->prev = NULL;
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;

        pthread_mutex_unlock(&list->mutex); 
        return newNode;
    }

    struct Node* current = list->head;
    int count = 0;

    while (current->next != NULL && count < position - 1) {
        current = current->next;
        count++;
    }

    newNode->prev = current;
    newNode->next = current->next;

    if (current->next != NULL) {
        current->next->prev = newNode;
    }

    current->next = newNode;
    list->lastAccessed = newNode;

    pthread_mutex_unlock(&list->mutex); 
    return newNode;
}

// Funzione per rimuovere il nodo di testa dalla lista
struct Node* removeHead(struct LinkedList* list) {
    if (list->head == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&list->mutex); 
    struct Node* nodeToRemove = list->head;
    list->head = nodeToRemove->next;

    if (nodeToRemove->next != NULL) {
        nodeToRemove->next->prev = NULL;
    }

    pthread_mutex_unlock(&list->mutex); 

    free(nodeToRemove);

    // Aggiornamento del nodo lastAccessed
    if (list->lastAccessed != NULL) {
        pthread_mutex_lock(&list->lastAccessed->mutex); 
        if (list->lastAccessed == nodeToRemove) {
            list->lastAccessed = NULL;
        }
        pthread_mutex_unlock(&list->lastAccessed->mutex); 
    }

    return list->head;
}


// Funzione per rimuovere un nodo dalla lista usando getAT
struct Node* removeAT(struct LinkedList* list, struct Node* nodeToRemove) {
    if (list->head == NULL || nodeToRemove == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&list->mutex); 

    if (list->head == nodeToRemove) {
        list->head = nodeToRemove->next;
    }

    if (nodeToRemove->next != NULL) {
        nodeToRemove->next->prev = nodeToRemove->prev;
    }

    if (nodeToRemove->prev != NULL) {
        nodeToRemove->prev->next = nodeToRemove->next;
    }

    // Aggiornamento del nodo lastAccessed
    if (list->lastAccessed == nodeToRemove) {
        list->lastAccessed = NULL;
    }

    pthread_mutex_unlock(&list->mutex); 
    pthread_mutex_lock(&nodeToRemove->mutex); 
    pthread_mutex_destroy(&nodeToRemove->mutex); // Dealloca il mutex del nodo
    pthread_mutex_unlock(&nodeToRemove->mutex); 

    free(nodeToRemove);

    return nodeToRemove;
}


// Funzione per stampare la lista
void printList(struct Node* head) {
    pthread_mutex_lock(&head->mutex); 
    struct Node* current = head;
    pthread_mutex_unlock(&head->mutex); 

    int cont = 0;
    while (current != NULL) {
        pthread_mutex_lock(&current->mutex); 

        printf("Nodo:%d ", cont);
        if (current->type == 0) {
            for (int i = 0; i < 8; i++) {
                printf("%02X ", current->data.data1[i]);
            }
            printf("\n");
            cont = cont + 1;
        } else {
            printf("%.15Lf\n", current->data.data2);
            cont = cont + 1;
        }

        struct Node* next = current->next;
        pthread_mutex_unlock(&current->mutex); 

        current = next;
    }
    printf("\n");
}

// Funzione per l'esecuzione dell'operazione di mappatura su un nodo
void* map_list(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    struct LinkedList* list = pool->list;

    pthread_mutex_lock(&list->mutex); 
    struct Node* current = list->head;
    pthread_mutex_unlock(&list->mutex); 

    while (current != NULL) {
        pthread_mutex_lock(&current->mutex); 

        pool->mapper(current);

        struct Node* next = current->next;
        pthread_mutex_unlock(&current->mutex); 

        current = next;
    }

    pthread_exit(NULL);
}

// Funzione per l'inizializzazione del thread-pool
void initialize_thread_pool(ThreadPool* pool, int thread_count, struct LinkedList* list, void (*mapper)(struct Node*)) {
    pool->threads = malloc(thread_count * sizeof(pthread_t));
    pool->thread_count = thread_count;
    pool->list = list;
    pool->mapper = mapper;

    int i;
for (i = 0; i < thread_count; i++) {
        pthread_create(&(pool->threads[i]), NULL, map_list, pool);
    }
}

// Funzione per attendere la terminazione dei thread nel thread-pool
void wait_for_threads(ThreadPool* pool) {
    int i;
    for (i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}



// Funzione di mapping per raddoppiare gli elementi
void doubleMapper(struct Node* node) {
    if (node->type == 1) {
        node->data.data2 *= 2;
    } else {
        for (int i = 0; i < 8; i++) {
            node->data.data1[i] *= 2;
        }
    }
}

// Funzione per l'applicazione della funzione di mapping a tutti gli elementi della lista tramite thread
void map(struct LinkedList* list, int thread_count, void (*mapper)(struct Node*)) {
    ThreadPool pool;
    initialize_thread_pool(&pool, thread_count, list, mapper);
    wait_for_threads(&pool);
}

// Funzione per l'applicazione della funzione di riduzione a tutti gli elementi della lista
long double reduce(struct LinkedList* list, long double (*reducer)(long double, struct Node*)) {
    if (list->head == NULL || reducer == NULL) {
        return 0.0L;
    }

    pthread_mutex_lock(&list->mutex); 
    struct Node* current = list->head;
    long double result = current->type == 1 ? current->data.data2 : 0.0L;
    pthread_mutex_unlock(&list->mutex); 

    current = current->next;

    while (current != NULL) {
        pthread_mutex_lock(&current->mutex);

        result = reducer(result, current);

        struct Node* next = current->next;
        pthread_mutex_unlock(&current->mutex);

        current = next;
    }

    return result;
}

// Funzione di riduzione  per sommare gli elementi
long double sumReducer(long double accumulator, struct Node* node) {
    if (node->type == 1) {
        return accumulator + node->data.data2;
    } else {
        int sum_array = 0;
        for (int i = 0; i < 8; i++) {
            if (node->data.data1[i] > 80) {  //confronto con 50
                sum_array += 1;
            }
        }
        return accumulator + sum_array;
    }
}

// Funzione che restituisce il puntatore all'elemento in una data posizione
struct Node* getAt(struct LinkedList* list, int position) {
    if (list->head == NULL || position < 0) {
        return NULL;
    }

    pthread_mutex_lock(&list->mutex); 
    struct Node* current = list->head;
    pthread_mutex_unlock(&list->mutex); 

    int count = 0;
    while (current != NULL && count < position) {
        pthread_mutex_lock(&current->mutex); 
        struct Node* next = current->next;
        pthread_mutex_unlock(&current->mutex); 

        current = next;
        count++;
    }

    return current;
}



int main() {
    struct LinkedList list1;
    list1.head = NULL;
    list1.lastAccessed = NULL;
    pthread_mutex_init(&list1.mutex, NULL);// Inizializza il mutex della lista
 

    // Inserimento di nodi nella lista
    unsigned char data1_1[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    insert(&list1, 0, data1_1, 0.0L);

   

    unsigned char data1_2[8] = {0x12,0x23,0x34,0x45,0x56,0x67,0x68,0x54};
    insert(&list1, 0, data1_2, 0.0L);
    
    

    unsigned char data1_3[8] = {0x23,0x65,0x23,0x67,0x93,0x56,0x78,0x60};
    insert(&list1, 0, data1_3, 0.0L);

    unsigned char data1_4[8] = {0x34,0x76,0x18,0x52,0x89,0x43,0x17,0x78};
    insertAT(&list1,0,data1_4,0.0L,2);

    // Stampa della lista
    printf("Lista originale unsigned char[8]:\n");
    printList(list1.head);

    // Rimozione di un nodo dalla lista
    printf("Rimozione del nodo in posizione 2:\n");
    struct Node* nodeToRemove = getAt(&list1, 2);
    removeAT(&list1, nodeToRemove);
    printList(list1.head);

    // Somma degli elementi della lista maggiori di 50 in ogni nodo
    long double sum = reduce(&list1, sumReducer);
    printf("Somma degli elementi della lista maggiori di 50 in ogni nodo(utilizzo reduce): %.2Lf\n", sum);

    // Applicazione della funzione doubleMapper
    printf("\nLista con gli elementi raddoppiati(utilizzo map):\n");
    map(&list1, 1, doubleMapper);
    printList(list1.head);

    // Rimozione del nodo in testa alla lista
    printf("Rimozione del nodo di testa:\n");
    
    removeHead(&list1);
    printList(list1.head);

    

    // Accesso a un elemento specifico
    int position = 1;
    struct Node* node = getAt(&list1, position);
    if (node != NULL) {
        printf("Elemento in posizione %d: ", position);
       
            for (int i = 0; i < 8; i++) {
                printf("%02X ", node->data.data1[i]);
            }
            printf("\n");
        
    } else {
        printf("Posizione %d non valida.\n", position);
    }

    // Liberazione della memoria occupata dalla lista
    struct Node* current = list1.head;
    while (current != NULL) {
        struct Node* temp = current;
        current = current->next;
        free(temp);
    }

    pthread_mutex_destroy(&list1.mutex); // Dealloca il mutex della lista

    printf("\n--------------------------------------------------------------\n");

    struct LinkedList list2;
    list2.head = NULL;
    list2.lastAccessed = NULL;
    pthread_mutex_init(&list2.mutex, NULL); // Inizializza il mutex della lista

     long double data2_1 = 1.23L;
    insert(&list2, 1, NULL, data2_1);
    long double data2_2 = 4.56L;
    insert(&list2, 1, NULL, data2_2);
    long double data2_3 = 13.33L;
    insert(&list2, 1, NULL, data2_3);
    long double data2_4 = 7.86L;
    insert(&list2, 1, NULL, data2_4);

    // Stampa della lista
    printf("Lista originale long double:\n");
    printList(list2.head);

    // Rimozione di un nodo dalla lista
    printf("Rimozione del nodo in posizione 1:\n");
    struct Node* nodeToRemove2 = getAt(&list2, 1);
    removeAT(&list2, nodeToRemove2);
    printList(list2.head);

    // Somma degli elementi della lista maggiori di 50 in ogni nodo
    long double sum2 = reduce(&list2, sumReducer);
    printf("Somma degli elementi della lista (utilizzo reduce): %.2Lf\n", sum2);

    // Applicazione della funzione doubleMapper
    printf("\nLista con gli elementi raddoppiati(utilizzo map):\n");
    map(&list2, 1, doubleMapper);
    printList(list2.head);

    // Rimozione del nodo in testa alla lista
    printf("Rimozione del nodo di testa:\n");
    
    removeHead(&list2);
    printList(list2.head);

    

    // Accesso a un elemento specifico
     position = 1;
    struct Node* node2 = getAt(&list2, position);
    if (node != NULL) {
        printf("Elemento in posizione %d: ", position);
       
            printf("%.15Lf\n", node2->data.data2);
            printf("\n");
        
    } else {
        printf("Posizione %d non valida.\n", position);
    }

    // Liberazione della memoria occupata dalla lista
    struct Node* current2 = list2.head;
    while (current != NULL) {
        struct Node* temp = current;
        current = current->next;
        free(temp);
    }

    pthread_mutex_destroy(&list2.mutex); // Dealloca il mutex della lista

    return 0;
}
