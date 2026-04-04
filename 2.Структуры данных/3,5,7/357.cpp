#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    unsigned long long data;
    struct Node* next;
} Node;

typedef struct {
    Node* begin;
    Node* end;
} Queue;

Node* createNode(unsigned long long data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

void createQueue(Queue* q) {
    q->begin = q->end = NULL;
}

int isEmpty(Queue* q) {
    return (q->begin == NULL);
}

void put(Queue* q, unsigned long long data) {
    Node* newNode = createNode(data);
    if (isEmpty(q)) {
        q->begin = q->end = newNode;
    }
    else {
        q->end->next = newNode;
        q->end = newNode;
    }
}

unsigned long long get(Queue* q) {
    Node* temp = q->begin;
    unsigned long long data = temp->data;
    q->begin = q->begin->next;
    free(temp);
    return data;
}

unsigned long long min(unsigned long long x, unsigned long long y, unsigned long long z) {
    if (x < y && x < z) return x;
    if (y <= x && y < z) return y;
    if (z <= y && z <= x) return z;
}


int main() {
    int n;
    scanf("%d", &n);
    Queue qx;
    createQueue(&qx);
    Queue qy;
    createQueue(&qy);
    Queue qz;
    createQueue(&qz);
    unsigned long long x = 1, y = 1, z = 1;
    unsigned long long m;
    for (int i = 1; i < n+1; i++) {
        m = min(x * 3, y * 5, z * 7);
        if (i == n) printf("%llu\n", m);
        put(&qx, m);
        put(&qy, m);
        put(&qz, m);
        if (m == x * 3) {
            x = get(&qx);
        }
        if (m == y * 5) {
            y = get(&qy);
        }
        if (m == z * 7) {
            z = get(&qz);
        }
    }
}
