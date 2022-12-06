#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_SET 50

typedef struct family {
    uint8_t *sets;
    int set_count;
} family;

typedef int (*metric)(family *fam1, family *fam2);

int set_difference(const uint8_t *set1, const uint8_t *set2) {
    int diff_count = 0;
    for (int i = 0; i < MAX_SET; i++) {
        if (set1[i] != set2[i]) {
            diff_count++;
        }
    }
    return diff_count;
}

int set_count(const uint8_t *set) {
    int sum = 0;
    for (int i = 0; i < MAX_SET; i++) {
        if (set[i] > 0) {
            sum++;
        }
    }
    return sum;
}

int first_metric_permutations(int *permutation, int next, uint8_t *used, int size, family *fam1, family *fam2) {
    // funkcja generuje wszystkie permutacje zbiorow w rodzinie fam2 przy pomocy rekurencj
    if (next == size) {
        // ostatni etap rekurencji, liczenie metryki dla konkretnej permutacji
        int sum = 0;
        for (int i = 0; i < size; i++) {
            int index = permutation[i];
            sum += set_difference(&fam1->sets[i * MAX_SET], &fam2->sets[index * MAX_SET]);
        }
        return sum;
    }

    // szukanie najmniejszego wyniku sposrod wszystkich permutacji z wybranymi next-1 poczatkowymi elementami
    int min = INT32_MAX;
    for (int i = 0; i < size; i++) {
        if (used[i]) {
            // liczba juz wczesniej wybrana
            continue;
        }
        permutation[next] = i;
        used[i] = 1;
        int value = first_metric_permutations(permutation, next + 1, used, size, fam1, fam2);
        if (value < min) {
            min = value;
        }
        used[i] = 0;
    }
    return min;
}

int first_metric(family *fam1, family *fam2) {
    // zapewnienie ze fam1 jest wieksza rodzina
    if (fam1->set_count < fam2->set_count) {
        family *k = fam1;
        fam1 = fam2;
        fam2 = k;
    }

    int original_fam2_set_count = fam2->set_count;
    if (fam1->set_count > fam2->set_count) {
        // dopelninie mniejszej rodziny pustymi zbiorami zeby obie mialy tyle samo zbiorow
        uint8_t *new_sets = calloc(fam1->set_count, MAX_SET);
        memcpy(new_sets, fam2->sets, fam2->set_count * MAX_SET);
        free(fam2->sets);
        fam2->sets = new_sets;
        fam2->set_count = fam1->set_count;
    }

    int *permutation = malloc(fam1->set_count * sizeof(int));
    uint8_t *used = calloc(fam1->set_count, 1);
    int value = first_metric_permutations(permutation, 0, used, fam1->set_count, fam1, fam2);
    free(permutation);
    free(used);

    // fam2->sets moze zminic rozmiar w pamieci, ale set_count musi zostac takie samo jak na poczatku
    fam2->set_count = original_fam2_set_count;
    return value;
}

int first_metric_approx(family *fam1, family *fam2) {
    // zapewnienie ze fam1 jest wieksza rodzina
    if (fam1->set_count < fam2->set_count) {
        family *k = fam1;
        fam1 = fam2;
        fam2 = k;
    }

    uint8_t *set_used = calloc(sizeof(int), fam1->set_count);
    int *chosen_dist = malloc(fam2->set_count * sizeof(int));
    for (int i = 0; i < fam2->set_count; i++) {
        // wybieranie dla zbioru nr i z fam2 zbioru z fam1 o najmniejszym dystansie
        int min_dist = INT32_MAX;
        int min_set;

        for (int j = 0; j < fam1->set_count; j++) {
            if (set_used[j]) {
                continue;
            }

            int dist = set_difference(&fam2->sets[i * MAX_SET], &fam1->sets[j * MAX_SET]);
            if (dist < min_dist) {
                min_dist = dist;
                min_set = j;
            }
        }

        set_used[min_set] = 1;
        chosen_dist[i] = min_dist;
    }

    int sum = 0;
    for (int i = 0; i < fam2->set_count; i++) {
        sum += chosen_dist[i];
    }

    for (int i = 0; i < fam1->set_count; i++) {
        if (!set_used[i]) {
            sum += set_count(&fam1->sets[i * MAX_SET]);
        }
    }

    return sum;
}

void print_set(const uint8_t *set) {
    for (int i = 0; i < MAX_SET; i++) {
        if (set[i]) {
            printf("%d ", i);
        }
    }
    printf("\n");
}

void print_family(family fam) {
    for (int i = 0; i < fam.set_count; i++) {
        print_set(&fam.sets[MAX_SET * i]);
    }
}

void read_set(char *line, uint8_t *set) {
    int number_start = 0;
    long element;
    for (int i = 0; line[i] != 0; i++) {
        if (line[i] == ' ') {
            line[i] = 0;
            element = strtol(&line[number_start], NULL, 10);
            if (element < MAX_SET && element >= 0) {
                set[element] = 1;
            }
            number_start = i + 1;
        }
    }
    element = strtol(&line[number_start], NULL, 10);
    if (element < MAX_SET && element >= 0) {
        set[element] = 1;
    }
}

char *non_empty_line(FILE *f) {
    char *line = NULL;
    while (1) {
        size_t n = 0;
        getline(&line, &n, f);
        if (strlen(line) > 1) {
            break;
        }
        free(line);
    }
    return line;
}

int families_from_console(family **families_out) {
    printf("Ile rodzin?\n> ");
    int family_count;
    scanf("%d", &family_count);

    family *families = calloc(family_count, sizeof(family));
    for (int i = 0; i < family_count; i++) {
        printf("Rodzina %d:\nIle zbiorow w rodzinie?\n> ", i);
        scanf("%d", &families[i].set_count);

        families[i].sets = calloc(families[i].set_count, MAX_SET);
        for (int j = 0; j < families[i].set_count; j++) {
            printf("Zbior %d:\nPodaj zbior (liczby od 0 do %d poprzedzielane spacjami)\n> ", j, MAX_SET - 1);

            char *line = non_empty_line(stdin);
            read_set(line, &families[i].sets[j * MAX_SET]);
            free(line);
        }
    }

    *families_out = families;
    return family_count;
}

int families_from_file(FILE *file, family **families_out) {
    int family_count;
    fscanf(file, "%d", &family_count);

    family *families = calloc(family_count, sizeof(family));
    for (int i = 0; i < family_count; i++) {
        int set_count;
        fscanf(file, "%d", &set_count);
        families[i].set_count = set_count;
        families[i].sets = calloc(families[i].set_count, MAX_SET);

        for (int j = 0; j < set_count; j++) {
            char *line = non_empty_line(file);
            read_set(line, &families[i].sets[MAX_SET * j]);
            free(line);
        }
    }

    *families_out = families;
    return family_count;
}

int main() {
    unsigned int option;
    printf("0. Wczytanie z konsoli\n1. Wczytanie z pliku\n> ");
    while (1) {
        scanf("%u", &option);
        if (option > 1) {
            printf("Niewlasciwa opcja\n> ");
            continue;
        }
        break;
    }

    family *families;
    int family_count;
    switch (option) {
        case 0:
            family_count = families_from_console(&families);
            break;
        case 1:
            printf("Podaj nazwe pliku\n> ");
            char *path;
            scanf("%ms", &path);
            FILE *file = fopen(path, "r");
            if (file == NULL) {
                fprintf(stderr, "Nie mozna otworzyc pliku\n");
                return EXIT_FAILURE;
            }
            family_count = families_from_file(file, &families);
            fclose(file);
            break;
        default:
            return EXIT_FAILURE;
    }

    for (int i = 0; i < family_count; i++) {
        printf("Rodzina %d:\n", i);
        print_family(families[i]);
    }

    printf("\n0. Pierwsza metryka\n1. Aproksymacja pierwszej metryki\n2. Porownanie\n> ");
    while (1) {
        scanf("%u", &option);
        if (option > 2) {
            printf("Niewlasciwa opcja\n> ");
            continue;
        }
        break;
    }

    for (int i = 0; i < family_count; i++) {
        for (int j = i + 1; j < family_count; j++) {
            switch (option) {
                case 0:
                    printf("Odleglosc pomiedzy rodzinami %d i %d: %d\n", i, j, first_metric(&families[i], &families[j]));
                    break;
                case 1:
                    printf("Odleglosc pomiedzy rodzinami %d i %d: %d\n", i, j, first_metric_approx(&families[i], &families[j]));
                    break;
                case 2:
                    printf("Odleglosc pomiedzy rodzinami %d i %d: %d, %d\n", i, j,
                           first_metric(&families[i], &families[j]),
                           first_metric_approx(&families[i], &families[j]));
                    break;
                default:
                    return EXIT_FAILURE;
            }
        }
    }

    for (int i = 0; i < family_count; i++) {
        free(families[i].sets);
    }
    free(families);

    return EXIT_SUCCESS;
}