#include <stdint.h>
#include <stdio.h>

/*
le bit le plus faible indique 0 si c'est libre
                              1 si c'est occupé
        false = 0
        true = 1
*/
uint8_t MY_HEAP[64000];

/**
 * @return 1 if the next part is free
 */
static inline uint8_t is_free(uint8_t int2){
    return (int2 & 1) == 0;
}
static inline void write_at_index(int index, uint16_t to_write){
    *(uint16_t *)(MY_HEAP + index) = to_write;
}
static inline int read_at_index(int index){
    return *(uint16_t *)(MY_HEAP + index);
}
static inline int pointer_to_index(void *ptr){
    return (int)((uint8_t *)ptr - MY_HEAP);
}
/*static inline int censor(int a){
    return a & ~1;
}*/

void init(){
    write_at_index(0, 64000);
}

void my_free(void *pointer) {
    if (!pointer) return;

    int index = pointer_to_index(pointer);
    if (index < 2 || index >= 64000) return;

    uint16_t header = read_at_index(index - 2);
    write_at_index(index - 2, header & ~1);

    uint16_t clean_size = header & ~1;
    int next = index - 2 + clean_size;
    if (next < 64000) {
        uint16_t next_header = read_at_index(next);
        if (is_free(next_header)) {
            write_at_index(index - 2, clean_size + next_header);
        }
    }
}

void *my_malloc(size_t size) {
    if (size == 0 || size > 64000 - 2) { //size est d'office > 0
        return 0;
    }

    uint16_t fix_size = (uint16_t)((size + 1) & ~1);
    uint16_t allocation_size = fix_size + 2;
    int current = 0;

    while (current < 64000) {
        uint16_t header = read_at_index(current);

        if (is_free(header)) {
            uint32_t total_free = header;
            int next = current + header;

            while (next < 64000) {
                uint16_t next_header = read_at_index(next);
                if (is_free(next_header)) {
                    total_free += next_header;
                    next += next_header;
                } else {
                    break;
                }
            }

            if (total_free >= allocation_size) {
                uint32_t reste = total_free - allocation_size;
                if (reste > 0) {
                    // On coupe le bloc : bloc occupé + nouveau bloc libre
                    write_at_index(current, allocation_size | 1);
                    write_at_index(current + allocation_size, (uint16_t)reste);
                } else {
                    // Tout l'espace est pris
                    write_at_index(current, (uint16_t)total_free | 1);
                }
                return (void *)(MY_HEAP + current + 2);
            } else {
                // Trop petit, mais on fusionne pour la prochaine fois 
                //permet de ne pas avoir a refaire ce travail la prochaine fois
                write_at_index(current, (uint16_t)total_free);
                current = next;
            }
        } else {
            current += header & ~1;
        }
    }
    return 0;
}

//autres fonctions
void printBits(unsigned int num){
    for (int bit = 0; bit < (sizeof(unsigned int) * 8); bit++){
        if (bit % 8 == 0){
            printf("| ");
        }
        printf("%i ", num & 0x01);
        num = num >> 1;
    }
}

void read_mem(){
    printf("read_mem ------------------\n");
    int current = 0;
    int iter = 0;
    int nb_blocs = 0;
    int sum_blocs = 0;
    while (current <= 64000 - 1){
        int size = read_at_index(current);
        int fix_size = size - (size & 0b1);
        if (is_free(size)){
            printf("%d : bloc libre de taille : %d\n", current, fix_size - 2);
        }
        else{
            printf("%d : bloc occupé de taille : %d\n", current, fix_size - 2);
        }
        current += fix_size;
        iter++;
        nb_blocs++;
        sum_blocs += fix_size;
        if (iter > 10){
            printf("|||||||||||| iter trop grand → break\n");
            break;
        }
    }
    printf("nb_blocs : %d de somme : %d\n", nb_blocs, sum_blocs);
    printf("------------------  mem read\n");
}


int main(int argc, char *argv[]){
    printf("test-----------test\n");
    init();
    printBits(0xff);
    printf("\n");
    printf("--test malloc 13 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(13)));
    printf("--fini malloc 13 :\n");
    read_mem();
    printf("devrait etre : %d\n", 64000 - 2 - 14 - 2);
    // printf("test false : %d\n", 1==2);
    printf("--test malloc 26 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(26)));
    printf("--fini malloc 26 :\n");
    read_mem();
    printf("devrait etre : %d\n", 64000 - 2 - 14 - 2 - 26 - 2);
    my_free((void *)&(MY_HEAP[2]));
    read_mem();
    printf("--test malloc 8 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(8)));
    printf("--fini malloc 8 :\n");
    read_mem();
    printf("--test malloc 8 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(8)));
    printf("--fini malloc 8 :\n");
    printf("--test malloc 8 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(8)));
    printf("--fini malloc 8 :\n");
    read_mem();
    my_free((void *)&(MY_HEAP[16 + 2]));
    read_mem();
    printf("--test malloc 8 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(8)));
    printf("--fini malloc 8 :\n");
    read_mem();
    printf("--test malloc 60000 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(60000)));
    printf("--fini malloc 60000 :\n");
    read_mem();
    printf("--test malloc 3928 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(3928)));
    printf("--fini malloc 3928 :\n");
    read_mem();
    printf("--test malloc 14 :\n");
    void*  malloc_14 = my_malloc(14);
    printf("résult malloc : %d\n", pointer_to_index(malloc_14));
    printf("--fini malloc 14 :\n");
    read_mem();
    my_free(malloc_14);
    read_mem();
    printf("--test malloc 16 :\n");
    void*  malloc_16 = my_malloc(16);
    printf("résult malloc : %d\n", pointer_to_index(malloc_16));
    printf("--fini malloc 16 :\n");
    read_mem();
    printf("--test malloc 2 :\n");
    void*  malloc_2 = my_malloc(2);
    printf("résult malloc : %d\n", pointer_to_index(malloc_2));
    printf("--fini malloc 2 :\n");
    read_mem();
    my_free(malloc_2);
    read_mem();
    printf("--test malloc 4 :\n");
    malloc_2 = my_malloc(4);
    printf("résult malloc : %d ,  ptr = %p\n", pointer_to_index(malloc_2), malloc_2);
    printf("--fini malloc 4 :\n");
    read_mem();
    my_free((void *)&(MY_HEAP[2]));
    read_mem();
    my_free((void *)&(MY_HEAP[10+2]));
    my_free((void *)&(MY_HEAP[20+2]));
    my_free((void *)&(MY_HEAP[38+2]));
    my_free((void *)&(MY_HEAP[44+2]));
    my_free((void *)&(MY_HEAP[54+2]));
    my_free((void *)&(MY_HEAP[64+2]));
    my_free((void *)&(MY_HEAP[60066+2]));
    my_free((void *)&(MY_HEAP[63996+2]));
    read_mem();
    printf("--test malloc 64000 :\n");
    void* malloc_64000 = my_malloc(64000);
    printf("résult malloc : %d ,  ptr = %p\n", pointer_to_index(malloc_64000), malloc_64000);
    printf("--fini malloc 64000 :\n");
    read_mem();
    printf("--test malloc 63998 :\n");
    void* malloc_63998 = my_malloc(63998);
    printf("résult malloc : %d ,  ptr = %p\n", pointer_to_index(malloc_63998), malloc_63998);
    printf("--fini malloc 63998 :\n");
    read_mem();
    printf("--------------fin du code----------\n");
    return 0;
}