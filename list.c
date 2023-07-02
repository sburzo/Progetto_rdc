#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definizione della struttura del nodo
struct Node {
    int type; // 0 per unsigned char[8], 1 per long double
    union {
        unsigned char data1[8];
        long double data2;
    } data;
    struct Node* prev;
    struct Node* next;
};

// Funzione per l'inserimento di un nuovo nodo in coda alla lista
void insert(struct Node** head, int type, unsigned char data1[8], long double data2) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->type = type;
    if (type == 0) {
        memcpy(newNode->data.data1, data1, sizeof(unsigned char[8]));
    } else {
        newNode->data.data2 = data2;
    }
    newNode->next = NULL;

    if ((*head) == NULL) {
        newNode->prev = NULL;
        (*head) = newNode;
        return;
    }

    struct Node* current = (*head);
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = newNode;
    newNode->prev = current;
}
// Funzione per l'inserimento di un nuovo nodo in una posizione specifica
void insertAT(struct Node** head, int type, unsigned char data1[8], long double data2, int position) {
    if (position < 0) {
        return;
    }

    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->type = type;
    if (type == 0) {
        memcpy(newNode->data.data1, data1, sizeof(unsigned char[8]));
    } else {
        newNode->data.data2 = data2;
    }

    if ((*head) == NULL) {
        newNode->prev = NULL;
        newNode->next = NULL;
        (*head) = newNode;
        return;
    }

    if (position == 0) {
        newNode->prev = NULL;
        newNode->next = (*head);
        (*head)->prev = newNode;
        (*head) = newNode;
        return;
    }

    struct Node* current = (*head);
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
}


// Funzione per rimuovere un nodo dalla lista
void removeNode(struct Node** head, struct Node* nodeToRemove) {
    if ((*head) == NULL || nodeToRemove == NULL) {
        return;
    }

    if ((*head) == nodeToRemove) {
        (*head) = nodeToRemove->next;
    }

    if (nodeToRemove->next != NULL) {
        nodeToRemove->next->prev = nodeToRemove->prev;
    }

    if (nodeToRemove->prev != NULL) {
        nodeToRemove->prev->next = nodeToRemove->next;
    }

    free(nodeToRemove);
}

// Funzione per stampare la lista
void printList(struct Node* head) {
    struct Node* current = head;
    int cont=0;
    while (current != NULL) {
        
        printf("Nodo:%d ",cont);
        if (current->type == 0) {
            
            for (int i = 0; i < 8; i++) {
                printf("%02X ", current->data.data1[i]);
            }
            printf("\n");
            cont=cont+1;
        } else {
            printf("%.15Lf\n", current->data.data2);
            cont=cont+1;
        }
        current = current->next;
        
    }
    printf("\n");
}


// Funzione per applicare una funzione specifica a ogni elemento della lista
void map(struct Node* head, void (*mapper)(struct Node*)) {
    if (head == NULL || mapper == NULL) {
        return;
    }

    struct Node* current = head;

    while (current != NULL) {
        mapper(current);
        current = current->next;
    }
}

// Funzione di mapping personalizzata per raddoppiare gli elementi
void doubleMapper(struct Node* node) {
    if (node->type == 1) {
        node->data.data2 *= 2;
    } else {
        for(int i=0;i<8;i++){
            node->data.data1[i]*=2;
        }
        
    }
}

// Funzione per applicare una funzione specifica a tutti gli elementi della lista
long double reduce(struct Node* head, long double (*reducer)(long double, struct Node*)) {
    if (head == NULL || reducer == NULL) {
        return 0.0L;
    }

    struct Node* current = head;
    long double result = current->type == 1 ? current->data.data2 : 0.0L;
    current = current->next;

    while (current != NULL) {
        result = reducer(result, current);
        current = current->next;
    }

    return result;
}

// Funzione di riduzione personalizzata per sommare gli elementi
long double sumReducer(long double accumulator, struct Node* node) {
    if (node->type == 1) {
        return accumulator + node->data.data2;
    } else {
        int sum_array=0;
        for(int i=0;i<8;i++){
            if(node->data.data1[i]>80){  //confronto con 50
                sum_array+=1;
            }
            
        }
        return accumulator + sum_array;
    }
}


//Funzione che restituisce il puntatore di elemento in position
struct Node* getAT(struct Node* head, int position) {
    if (head == NULL || position < 0) {
        return NULL;
    }

    struct Node* current = head;
    int count = 0;

    while (current != NULL && count < position) {
        current = current->next;
        count++;
    }

    return current;
}

int main() {
    struct Node* head = NULL;

    // Inserimento di nodi nella lista
    unsigned char data1_1[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    insert(&head, 0, data1_1, 0.0L);

    long double data2_1 = 1.23L;
    insert(&head, 1, NULL, data2_1);

    unsigned char data1_2[8] = {0x12,0x23,0x34,0x45,0x56,0x67,0x68,0x54};
    insert(&head, 0, data1_2, 0.0L);
    
    long double data2_2 = 4.56L;
    insert(&head, 1, NULL, data2_2);

    unsigned char data1_3[8] = {0x23,0x65,0x23,0x67,0x93,0x56,0x78,0x60};
    insert(&head, 0, data1_3, 0.0L);

    unsigned char data1_4[8] = {0x34,0x76,0x18,0x52,0x89,0x43,0x17,0x78};
    insertAT(&head,0,data1_4,0.0L,2);


    // Stampa della lista
    printList(head);

    // Rimozione di un nodo dalla lista da capo 
    removeNode(&head, head);

    // Stampa della lista dopo la rimozione
    printf("\nRimosso nodo di testa\n");
    printList(head);

    printf("Lista dopo l'applicazione della funzione map (raddoppio degli elementi):\n");
    //map(head, doubleMapper);
    printList(head);

    removeNode(&head,getAT(head,2));
    printf("\nRimosso nodo in posizione 2 utilizzando getAT:\n");
    printList(head);

    long double sum=reduce(head,sumReducer);
    printf("Somma elementi: %Lf\n", sum);//somma dei long double + # elem>50 in ogni array 

    // Liberazione della memoria occupata dalla lista
    struct Node* current = head;
    while (current != NULL) {
        struct Node* temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}
