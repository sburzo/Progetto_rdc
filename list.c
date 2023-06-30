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
        
        printf("Nodo: %d\n",cont);
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

int main() {
    struct Node* head = NULL;

    // Inserimento di nodi nella lista
    unsigned char data1_1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    insert(&head, 0, data1_1, 0.0L);

    printf("Lista:\n");
    printList(head);

    long double data2_1 = 1.23L;
    insert(&head, 1, NULL, data2_1);

    printf("Lista:\n");
    printList(head);

    unsigned char data1_2[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
    insert(&head, 0, data1_2, 0.0L);

    printf("Lista:\n");
    printList(head);

    long double data2_2 = 4.56L;
    insert(&head, 1, NULL, data2_2);

    // Stampa della lista
    printf("Lista:\n");
    printList(head);

    // Rimozione di un nodo dalla lista
    removeNode(&head, head);

    // Stampa della lista dopo la rimozione
    printf("\nLista dopo la rimozione di un nodo:\n");
    printList(head);

    // Liberazione della memoria occupata dalla lista
    struct Node* current = head;
    while (current != NULL) {
        struct Node* temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}
