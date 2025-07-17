#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>

int turn_cube(int restricted_face_position, char direction) {
    if (direction == 'u') {
        switch (restricted_face_position) {
        case 1:
            return 2;
        case 2:
            return 6;
        case 3:
            return 1;
        case 4:
            return 4;
        case 5:
            return 5;
        }
    }
    else if (direction == 'd') {
        switch (restricted_face_position) {
        case 1:
            return 3;
        case 2:
            return 1;
        case 3:
            return 6;
        case 4:
            return 4;
        case 5:
            return 5;
        }
    }
    else if (direction == 'r') {
        switch (restricted_face_position) {
        case 1:
            return 4;
        case 2:
            return 2;
        case 3:
            return 3;
        case 4:
            return 6;
        case 5:
            return 1;
        }
    }
    else if (direction == 'l') {
        switch (restricted_face_position) {
        case 1:
            return 5;
        case 2:
            return 2;
        case 3:
            return 3;
        case 4:
            return 1;
        case 5:
            return 6;
        }
    }
}
void result_match(int n, int patch[100][100]) {
    int t = 0;
    printf("Found patch: \n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            t = patch[i][j] + 1;
            printf("%d  ", t);
        }
        printf("\n");
    }
}

long long counter = 0;
void backtraking(int n, int m[][100], int restricted_face_position, int i, int j, int vis, int patch[][100], int* found) {
    counter++;
    if (vis == n * n) {
        if (restricted_face_position == 1 && i == 0 && j == n - 1) {
            result_match(n, patch);
            printf("\n");
            printf("Time for %d x %d = %lld", n, n, counter);
            printf("\n");
            *found = 1;
        }
        else {
            return;
        }
    }
    else if (vis != n * n && i == 0 && j == n - 1) {
        return;
    }
    else {
        m[i][j] = vis;

        if (i - 1 >= 0 && m[i - 1][j] == 0 && turn_cube(restricted_face_position, 'u') != 6) {
            patch[i - 1][j] = vis;

            backtraking(n, m, turn_cube(restricted_face_position, 'u'), i - 1, j, vis + 1, patch, found);
        }

        if (i + 1 < n && m[i + 1][j] == 0 && turn_cube(restricted_face_position, 'd') != 6) {
            patch[i + 1][j] = vis;

            backtraking(n, m, turn_cube(restricted_face_position, 'd'), i + 1, j, vis + 1, patch, found);
        }

        if (j + 1 < n && m[i][j + 1] == 0 && turn_cube(restricted_face_position, 'r') != 6) {
            patch[i][j + 1] = vis;

            backtraking(n, m, turn_cube(restricted_face_position, 'r'), i, j + 1, vis + 1, patch, found);
        }

        if (j - 1 >= 0 && m[i][j - 1] == 0 && turn_cube(restricted_face_position, 'l') != 6) {
            patch[i][j - 1] = vis;

            backtraking(n, m, turn_cube(restricted_face_position, 'l'), i, j - 1, vis + 1, patch, found);
        }
        m[i][j] = 0;
        vis--;
        return;
    }
}


int main() {
   int n;
    scanf("%d", &n);
    int m[100][100] = { 0 }, patch[100][100] = { 0 }, found = 0, k_1[100][100] = { 0 };
    clock_t begin = clock();
    backtraking(n, m, 1, 0, 0, 1, patch, &found);
    if (!found) {
        printf("no solution\n");
    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Total time spent: %Lf seconds\n", time_spent);
    return 0;
}
