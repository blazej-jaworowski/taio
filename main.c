#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define MAX_SET 50

typedef struct family {
    uint8_t *sets;
    int set_count;
} family;

typedef float (*set_metric)(const uint8_t *set1, const uint8_t *set2);

int set_count(const uint8_t *set) {
    int sum = 0;
    for (int i = 0; i < MAX_SET; i++) {
        if (set[i] > 0) {
            sum++;
        }
    }
    return sum;
}

float set_difference(const uint8_t *set1, const uint8_t *set2) {
    if (set2 == NULL) {
        return (float) set_count(set1);
    }
    if (set1 == NULL) {
        return (float) set_count(set2);
    }
    int diff_count = 0;
    for (int i = 0; i < MAX_SET; i++) {
        if (set1[i] != set2[i]) {
            diff_count++;
        }
    }
    return (float)diff_count;
}

float second_set_metric(const uint8_t *set1, const uint8_t *set2) {
    if (set1 == NULL || set2 == NULL) {
        return 1.0f;
    }
    int set1_count = 0;
    int set2_count = 0;
    int common_count = 0;
    for (int i = 0; i < MAX_SET; i++) {
        if (set1[i]) {
            set1_count++;
        }
        if (set2[i]) {
            set2_count++;
        }
        if (set1[i] && set2[i]) {
            common_count++;
        }
    }
    if (set1_count == 0 || set2_count == 0) {
        return 1.0f;
    }
    return 1.0f - (float) common_count / (float) (set1_count < set2_count ? set1_count : set2_count);
}

float metric_permutations(int *permutation, int next, uint8_t *used, int size, family *fam1, family *fam2, set_metric m) {
    // funkcja generuje wszystkie permutacje zbiorow w rodzinie fam2 przy pomocy rekurencj
    if (next == size) {
        // ostatni etap rekurencji, liczenie metryki dla konkretnej permutacji
        float sum = 0;
        for (int i = 0; i < size; i++) {
            int index = permutation[i];
            sum += m(&fam1->sets[i * MAX_SET], &fam2->sets[index * MAX_SET]);
        }
        return sum;
    }

    // szukanie najmniejszego wyniku sposrod wszystkich permutacji z wybranymi next-1 poczatkowymi elementami
    float min = INFINITY;
    for (int i = 0; i < size; i++) {
        if (used[i]) {
            // liczba juz wczesniej wybrana
            continue;
        }
        permutation[next] = i;
        used[i] = 1;
        float value = metric_permutations(permutation, next + 1, used, size, fam1, fam2, m);
        if (value < min) {
            min = value;
        }
        used[i] = 0;
    }
    return min;
}

float min_metric(family *fam1, family *fam2, set_metric m) {
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
    float value = metric_permutations(permutation, 0, used, fam1->set_count, fam1, fam2, m);
    free(permutation);
    free(used);

    // fam2->sets moze zminic rozmiar w pamieci, ale set_count musi zostac takie samo jak na poczatku
    fam2->set_count = original_fam2_set_count;
    return value;
}

float metric_approx(family *fam1, family *fam2, set_metric m) {
    // zapewnienie ze fam1 jest wieksza rodzina
    if (fam1->set_count < fam2->set_count) {
        family *k = fam1;
        fam1 = fam2;
        fam2 = k;
    }

    uint8_t *set_used = calloc(sizeof(int), fam1->set_count);
    float *chosen_dist = malloc(fam2->set_count * sizeof(float));
    for (int i = 0; i < fam2->set_count; i++) {
        // wybieranie dla zbioru nr i z fam2 zbioru z fam1 o najmniejszym dystansie
        float min_dist = INFINITY;
        int min_set;

        for (int j = 0; j < fam1->set_count; j++) {
            if (set_used[j]) {
                continue;
            }

            float dist = m(&fam2->sets[i * MAX_SET], &fam1->sets[j * MAX_SET]);
            if (dist < min_dist) {
                min_dist = dist;
                min_set = j;
            }
        }

        set_used[min_set] = 1;
        chosen_dist[i] = min_dist;
    }

    float sum = 0;
    for (int i = 0; i < fam2->set_count; i++) {
        sum += chosen_dist[i];
    }

    for (int i = 0; i < fam1->set_count; i++) {
        if (!set_used[i]) {
            sum += m(&fam1->sets[i * MAX_SET], NULL);
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

    set_metric chosen_set_metric;
    printf("\n0. Pierwsza metryka\n1. Druga matryka\n> ");
    while (1) {
        scanf("%u", &option);
        if (option > 1) {
            printf("Niewlasciwa opcja\n> ");
            continue;
        }
        break;
    }

    switch(option) {
        case 0:
            chosen_set_metric = set_difference;
            break;
        case 1:
            chosen_set_metric = second_set_metric;
            break;
        default:
            return EXIT_FAILURE;
    }

    printf("\n0. Algorytm o zlozonosci wykladniczej\n1. Aproksymacja\n2. Porownanie\n> ");
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
                    printf("Odleglosc pomiedzy rodzinami %d i %d: %f\n", i, j, min_metric(&families[i], &families[j], chosen_set_metric));
                    break;
                case 1:
                    printf("Odleglosc pomiedzy rodzinami %d i %d: %f\n", i, j, metric_approx(&families[i], &families[j], chosen_set_metric));
                    break;
                case 2:
                    printf("Odleglosc pomiedzy rodzinami %d i %d: %f, %f\n", i, j,
                           min_metric(&families[i], &families[j], chosen_set_metric),
                           metric_approx(&families[i], &families[j], chosen_set_metric));
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