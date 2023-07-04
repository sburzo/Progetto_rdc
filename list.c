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

    pthread_mutex_unlock(&list->mutex); // Rilascia il mutex della lista
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
    pthread_mutex_init(&newNode->mutex, NULL); // Inizializza il mutex del nuovo nodo

    pthread_mutex_lock(&list->mutex); // Acquisisce il mutex della lista

    if (list->head == NULL) {
        newNode->prev = NULL;
        newNode->next = NULL;
        list->head = newNode;
        list->lastAccessed = newNode;

        pthread_mutex_unlock(&list->mutex); // Rilascia il mutex della lista
        return newNode;
    }

    if (position == 0) {
        newNode->prev = NULL;
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;

        pthread_mutex_unlock(&list->mutex); // Rilascia il mutex della lista
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

    pthread_mutex_unlock(&list->mutex); // Rilascia il mutex della lista
    return newNode;
}


// Funzione per rimuovere un nodo dalla lista
struct Node* removeAT(struct LinkedList* list, struct Node* nodeToRemove) {
    if (list->head == NULL || nodeToRemove == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&list->mutex); // Acquisisce il mutex della lista

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

    pthread_mutex_unlock(&list->mutex); // Rilascia il mutex della lista

    pthread_mutex_lock(&nodeToRemove->mutex); // Acquisisce il mutex del nodo
    pthread_mutex_destroy(&nodeToRemove->mutex); // Dealloca il mutex del nodo
    pthread_mutex_unlock(&nodeToRemove->mutex); // Rilascia il mutex del nodo

    free(nodeToRemove);

    return nodeToRemove;
}


// Funzione per stampare la lista
void printList(struct Node* head) {
    pthread_mutex_lock(&head->mutex); // Acquisisce il mutex del nodo
    struct Node* current = head;
    pthread_mutex_unlock(&head->mutex); // Rilascia il mutex del nodo

    int cont = 0;
    while (current != NULL) {
        pthread_mutex_lock(&current->mutex); // Acquisisce il mutex del nodo

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
        pthread_mutex_unlock(&current->mutex); // Rilascia il mutex del nodo

        current = next;
    }
    printf("\n");
}

// Funzione per applicare una funzione specifica a ogni elemento della lista
void map(struct Node* head, void (*mapper)(struct Node*)) {
    if (head == NULL || mapper == NULL) {
        return;
    }

    pthread_mutex_lock(&head->mutex); // Acquisisce il mutex del nodo
    struct Node* current = head;
    pthread_mutex_unlock(&head->mutex); // Rilascia il mutex del nodo

    while (current != NULL) {
        pthread_mutex_lock(&current->mutex); // Acquisisce il mutex del nodo

        mapper(current);

        struct Node* next = current->next;
        pthread_mutex_unlock(&current->mutex); // Rilascia il mutex del nodo

        current = next;
    }
}
// Funzione di mapping personalizzata per raddoppiare gli elementi
void doubleMapper(struct Node* node) {
    if (node->type == 1) {
        node->data.data2 *= 2;
    } else {
        for (int i = 0; i < 8; i++) {
            node->data.data1[i] *= 2;
        }
    }
}
// Funzione per applicare una funzione specifica a tutti gli elementi della lista
long double reduce(struct Node* head, long double (*reducer)(long double, struct Node*)) {
    if (head == NULL || reducer == NULL) {
        return 0.0L;
    }

    pthread_mutex_lock(&head->mutex); // Acquisisce il mutex del nodo
    struct Node* current = head;
    long double result = current->type == 1 ? current->data.data2 : 0.0L;
    pthread_mutex_unlock(&head->mutex); // Rilascia il mutex del nodo

    current = current->next;

    while (current != NULL) {
        pthread_mutex_lock(&current->mutex); // Acquisisce il mutex del nodo

        result = reducer(result, current);

        struct Node* next = current->next;
        pthread_mutex_unlock(&current->mutex); // Rilascia il mutex del nodo

        current = next;
    }

    return result;
}
// Funzione di riduzione personalizzata per sommare gli elementi
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

    pthread_mutex_lock(&list->mutex); // Acquisisce il mutex della lista

    int currentIndex = 0;
    struct Node* currentNode = list->head;

    while (currentNode != NULL && currentIndex != position) {
        pthread_mutex_lock(&currentNode->mutex); // Acquisisce il mutex del nodo

        struct Node* next = currentNode->next;
        pthread_mutex_unlock(&currentNode->mutex); // Rilascia il mutex del nodo

        currentIndex++;
        currentNode = next;
    }

    // Memorizza l'ultimo elemento utilizzato nella cache (lastAccessed)
    //pthread_mutex_lock(&list->lastAccessed->mutex); // Acquisisce il mutex del nodo
    //list->lastAccessed = currentNode;
    //pthread_mutex_unlock(&list->lastAccessed->mutex); // Rilascia il mutex del nodo

    pthread_mutex_unlock(&list->mutex); // Rilascia il mutex della lista

    return currentNode;
}

// Funzione per rimuovere il nodo di testa dalla lista
struct Node* removeHead(struct LinkedList* list) {
    if (list->head == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&list->mutex); // Acquisisce il mutex della lista

    struct Node* nodeToRemove = list->head;
    list->head = nodeToRemove->next;

    if (nodeToRemove->next != NULL) {
        nodeToRemove->next->prev = NULL;
    }

    pthread_mutex_unlock(&list->mutex); // Rilascia il mutex della lista

    free(nodeToRemove);

    // Aggiornamento del nodo lastAccessed
    if (list->lastAccessed != NULL) {
        pthread_mutex_lock(&list->lastAccessed->mutex); // Acquisisce il mutex del nodo
        if (list->lastAccessed == nodeToRemove) {
            list->lastAccessed = NULL;
        }
        pthread_mutex_unlock(&list->lastAccessed->mutex); // Rilascia il mutex del nodo
    }

    return list->head;
}


int main() {
    struct LinkedList list;
    list.head = NULL;
    list.lastAccessed = NULL;
    pthread_mutex_init(&list.mutex, NULL); // Inizializza il mutex della lista

    // Inserimento di nodi nella lista
    unsigned char data1_1[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    insert(&list, 0, data1_1, 0.0L);

    long double data2_1 = 1.23L;
    insert(&list, 1, NULL, data2_1);

    unsigned char data1_2[8] = {0x12,0x23,0x34,0x45,0x56,0x67,0x68,0x54};
    insert(&list, 0, data1_2, 0.0L);
    
    long double data2_2 = 4.56L;
    insert(&list, 1, NULL, data2_2);

    unsigned char data1_3[8] = {0x23,0x65,0x23,0x67,0x93,0x56,0x78,0x60};
    insert(&list, 0, data1_3, 0.0L);

    unsigned char data1_4[8] = {0x34,0x76,0x18,0x52,0x89,0x43,0x17,0x78};
    insertAT(&list,0,data1_4,0.0L,2);

    // Stampa della lista
    printf("Lista originale:\n");
    printList(list.head);

    // Rimozione di un nodo dalla lista
    printf("Rimozione del nodo in posizione 2:\n");
    struct Node* nodeToRemove = getAt(&list, 2);
    removeAT(&list, nodeToRemove);
    printList(list.head);

    // Applicazione della funzione doubleMapper
    printf("Lista con gli elementi raddoppiati:\n");
    map(list.head, doubleMapper);
    printList(list.head);

    // Rimozione del nodo in testa alla lista
    printf("Rimozione del nodo di testa:\n");
    
    removeHead(&list);
    printList(list.head);

    // Somma degli elementi della lista
    long double sum = reduce(list.head, sumReducer);
    printf("Somma degli elementi(long double): %.2Lf\n", sum);

    // Accesso a un elemento specifico
    int position = 2;
    struct Node* node = getAt(&list, position);
    if (node != NULL) {
        printf("Elemento in posizione %d: ", position);
        if (node->type == 0) {
            for (int i = 0; i < 8; i++) {
                printf("%02X ", node->data.data1[i]);
            }
            printf("\n");
        } else {
            printf("%.15Lf\n", node->data.data2);
        }
    } else {
        printf("Posizione %d non valida.\n", position);
    }

    // Liberazione della memoria occupata dalla lista
    struct Node* current = list.head;
    while (current != NULL) {
        struct Node* temp = current;
        current = current->next;
        free(temp);
    }

    pthread_mutex_destroy(&list.mutex); // Dealloca il mutex della lista

    return 0;
}
