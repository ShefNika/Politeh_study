#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Node {
    char* element;
    unsigned long long elementSize;
    char key;
    unsigned long long counter;
    struct Node** segments;
    unsigned long long segmentsNumber;
    unsigned long long segmentsSize;
} Node;

Node* createNode(long long segmentsSize) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->element = NULL;
    newNode->elementSize = 0;
    newNode->key = -1;
    newNode->counter = 0;
    newNode->segmentsNumber = 0;
    newNode->segmentsSize = segmentsSize;
    newNode->segments = (Node**)malloc(segmentsSize * sizeof(Node*));
    for (long long i = 0; i < segmentsSize; i++)
        newNode->segments[i] = NULL;
    return newNode;
}

void deleteNode(Node* node) {
    for (long long i = 0; i < node->segmentsSize; i++) {
        if (node->segments[i] != NULL)
            deleteNode(node->segments[i]);
    }
    free(node->segments);
    if (node->element != NULL)
        free(node->element);
    free(node);
}

void setElement(Node* node, char* charArr, unsigned long long size) {
    node->elementSize = size;
    node->element = (char*)malloc(node->elementSize * sizeof(char));
    for (unsigned long long i = 0; i < node->elementSize; i++)
        node->element[i] = charArr[i];
}

void setKey(Node* node, char key) {
    node->key = key;
}

char getKey(Node* node) {
    return node->key;
}

void setCounter(Node* node, unsigned long long counter) {
    node->counter = counter;
}

unsigned long long getCounter(Node* node) {
    return node->counter;
}

void insertKey(Node* node, char key) {
    if (node->segmentsNumber + 1 > node->segmentsSize) {
        node->segmentsSize = node->segmentsSize * 2;
        node->segments = (Node**)realloc(node->segments, node->segmentsSize * sizeof(Node*));
        for (long long i = node->segmentsNumber; i < node->segmentsSize; i++)
            node->segments[i] = NULL;
    }
    node->segments[node->segmentsNumber] = createNode(16);
    setKey(node->segments[node->segmentsNumber], key);
    setCounter(node->segments[node->segmentsNumber], 0);
    node->segmentsNumber++;
}

long long containsOn(Node* node, char key) {
    for (long long i = 0; i < node->segmentsNumber; i++) {
        if (node->segments[i] == NULL) return -1;
        if (getKey(node->segments[i]) == key) return i;
    }
    return -1;
}

void add(Node* node, char* charArr, unsigned long long pos) {
    if (!charArr[pos]) {
        node->counter++;
        if (node->element == NULL)
            setElement(node, charArr, pos + 1);
        return;
    }
    long long segmentsPos = containsOn(node, charArr[pos]);
    if (segmentsPos != -1)
        add(node->segments[segmentsPos], charArr, pos + 1);
    else {
        insertKey(node, charArr[pos]);
        add(node->segments[node->segmentsNumber - 1], charArr, pos + 1);
    }
}

void printThis(Node* node) {
    if (node->counter) {
        printf("%s : %llu\n", node->element, node->counter);
    }
}

void printBranches(Node* node) {
    for (long long i = 0; i < node->segmentsNumber; i++) {
        printBranches(node->segments[i]);
        printThis(node->segments[i]);
    }
}

typedef struct Three {
    Node* root;
} Three;

Three* createThree() {
    Three* newThree = (Three*)malloc(sizeof(Three));
    newThree->root = createNode(1);
    setKey(newThree->root, '0');
    setCounter(newThree->root, 0);
    char arr[] = "0";
    setElement(newThree->root, arr, 1);
    return newThree;
}

void deleteThree(Three* tree) {
    deleteNode(tree->root);
    free(tree);
}

void insert(Three* tree, char* charArr) {
    add(tree->root, charArr, 0);
}

void print(Three* tree) {
    printBranches(tree->root);
}

char isSixtet(const char* word) {
    unsigned offset = 0;
    while (isspace(word[offset])) offset++; // подсчет пробелов
    if (word[0 + offset] == '\0' || word[1 + offset] == '\0' || word[2 + offset] == '\0') return 0; // недостаточно места для 0xN
    if (word[0 + offset] != '0' || word[1 + offset] != 'x') return 0; // нет префикса 0x
    unsigned i = 2 + offset; // пропуск 0x
    for (; word[i]; i++) {
        if (word[i] == '*') return 0;
        if ((word[i] >= '0' && word[i] <= '9') || (word[i] >= 'A' && word[i] <= 'F') || (word[i] >= 'a' && word[i] <= 'f') || word[i] == ' ') {
            if (word[i] == '*') return 0;
            continue;
        }
        if (word[i] == 'L' || word[i] == 'l' || word[i] == 'U' || word[i] == 'u') {
            if (word[i + 1] == 'L' || word[i + 1] == 'l') {
                if ((word[i + 2] == 'L' || word[i + 2] == 'l') && (word[i] == 'U' || word[i] == 'u')) {
                    if (word[i + 3] == '\0' || word[i + 3] == ' ') return 1;
                }
                if (word[i + 2] == '\0' || word[i + 2] == ' ') return 1;
            }
            if (word[i + 1] == '\0' || word[i + 1] == ' ') return 1;
        }
        return 0;
    }
    return 1;
}

unsigned w_len(char* word) {
    unsigned wordLen = 0;
    char isNumber = 0;
    for (unsigned i = 0; word[i]; i++) {
        wordLen++;
        if (isNumber) {
            if ((word[i] >= '0' && word[i] <= '9') || (word[i] >= 'A' && word[i] <= 'F') || (word[i] >= 'a' && word[i] <= 'f') || word[i] == 'L' || word[i] == 'l' || word[i] == 'U' || word[i] == 'u') {
                continue;
            }
            if (strchr("GHIJKLMNOPQRSTUVWXYZghijklmnopqrstuvwxyz", word[i]) != NULL) {
                word[i] = '*';
                break;
            }
        }
        else {
            if (word[i] == '0' && word[i + 1] == 'x') {
                if ((word[i + 2] >= '0' && word[i + 2] <= '9') || (word[i + 2] >= 'A' && word[i + 2] <= 'F') || (word[i + 2] >= 'a' && word[i + 2] <= 'f')) {
                    i += 2;
                    wordLen += 2;
                    isNumber = 1;
                    continue;
                }
            }
            if (strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", word[i]) != NULL && word[i + 1] == '0' && word[i + 2] == 'x') {
                word[i] = '*';
                break;
            }
        }
        wordLen--;
        word[i] = ' ';
    }
    return wordLen + 1; // +1 для '\0' на конце строки
}

void cleanWord(const char* word, char* clearWord, unsigned clearWordSize) {
    unsigned pos = 0;
    for (unsigned i = 0; word[i]; i++) {
        if (word[i] != ' ') {
            clearWord[pos] = word[i];
            pos++;
        }
    }
    clearWord[pos] = '\0';
}

void removeComments(FILE* in, FILE* out) {
    char now, next;
    while ((now = fgetc(in)) != EOF) {
        if (now == '\"') {
            fputc(now, out);
            while ((next = fgetc(in)) != EOF) {
                fputc(next, out);
                if (next == '\"')
                    break;
            }
        }
        else if (now == '\'') {
            fputc(now, out);
            while ((next = fgetc(in)) != EOF) {
                fputc(next, out);
                if (next == '\'')
                    break;
            }
        }
        else if (now == '/') {
            next = fgetc(in);
            if (next == '/') {
                while ((next = fgetc(in)) != '\n' && next != EOF);
                fputc('\n', out);
            }
            else if (next == '*') {
                while ((next = fgetc(in)) != EOF) {
                    if (next == '*' && (next = fgetc(in)) == '/')
                        break;
                }
            }
            else {
                fputc(now, out);
                ungetc(next, in);
            }
        }
        else {
            fputc(now, out);
        }
    }
}

int main() {
    FILE* inputFile = fopen("C:/Users/mvideo/source/repos/big_probnik/big_probnik/big_probnik.cpp", "r");
    FILE* out = fopen("no_comm.cpp", "w");
    removeComments(inputFile, out);
    fclose(inputFile);
    fclose(out);
    FILE* in = fopen("no_comm.cpp", "r");
    if (!in) {
        printf("Failed to open input file");
        return EXIT_FAILURE;
    }

    char* word = (char*)malloc(256 * sizeof(char));
    char* clearWord = NULL;
    int clearWordSize = 0;

    Three* tree = createThree();

    int ch, pos = 0;
    while ((ch = fgetc(in)) != EOF) {
        if (isspace(ch) || pos >= 255 || ch == ',' || ch == ';' || ch == '-' || ch == '+' || ch == '^' || ch == '*' || ch == '/' || ch == '>' || ch == '<' || ch == '%' || ch == '&' || ch == '=') {
            if (pos > 0) {
                word[pos] = '\0';
                clearWordSize = w_len(word);
                if (isSixtet(word)) {
                    clearWord = (char*)realloc(clearWord, clearWordSize * sizeof(char));
                    cleanWord(word, clearWord, clearWordSize);
                    insert(tree, clearWord);
                }
                pos = 0;
            }
        }
        else {
            word[pos++] = ch;
        }
    }

    if (pos > 0) {
        word[pos] = '\0';
        clearWordSize = w_len(word);
        if (isSixtet(word)) {
            clearWord = (char*)realloc(clearWord, clearWordSize * sizeof(char));
            cleanWord(word, clearWord, clearWordSize);
            insert(tree, clearWord);
        }
    }

    printf("Into three:\n");
    print(tree);
    deleteThree(tree);
    free(clearWord);
    free(word);
    fclose(in);
    return 0;
}
