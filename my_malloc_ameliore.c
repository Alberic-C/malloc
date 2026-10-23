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
 * @param int2 l'en tête a vérifier
 * @return 1 si le bloc dont est issu l'en tête est libre
 */
static inline uint8_t is_free(uint8_t int2){
    return (int2 & 1) == 0;
}
/**
 * @param index l'index dans MY_HEAP auquel il faut écrire
 * @param to_write l'en tête à écrire
 * cette fonction écrit l'en tête demandé a l'index demandé dans MY_HEAP
 */
static inline void write_at_index(int index, uint16_t to_write){
    *(uint16_t *)(MY_HEAP + index) = to_write;
}
/**
 * @param index l'index dans MY_HEAP qu'il faut lire
 * @return l'en tête a cet index de MY_HEAP
 */
static inline int read_at_index(int index){
    return *(uint16_t *)(MY_HEAP + index);
}
/**
 * @param ptr le pointeur recu
 * @return l'index dans MY_HEAP que ça représente (peut être out of bounds)
 */
static inline int pointer_to_index(void *ptr){
    return (int)((uint8_t *)ptr - MY_HEAP);
}
/*static inline int censor(int a){
    return a & ~1;
}*/

/**
 * initialise la mémoire en mettant l'en tête a l'index 0
 */
void init(){
    write_at_index(0, 64000);//écrit que la taille totale fait 64000
}
/**
 * @param pointer le pointeur donné par my_malloc
 * libère l'espace qui as été donné afin qu'il puisse être réutilisé
 */
void my_free(void *pointer) {
    int index = pointer_to_index(pointer);
    if (index < 2 || index >= 64000) return; // vérifie que le pointeur est bien dans MY_HEAP

    uint16_t clean_size = read_at_index(index - 2) & ~1;
    write_at_index(index - 2, clean_size);//on libère le bloc en mettant le bit de poids faible à 0

    int next = index - 2 + clean_size; // on va chercher le bloc suivant
    if (next < 64000) { // si il éxiste
        uint16_t next_header = read_at_index(next); // on prends sa taille
        if (is_free(next_header)) {
            write_at_index(index - 2, clean_size + next_header); // si il est libre on l'absorbe dans le bloc qu'on viens de libérer
        }
    }
}
/**
 * @param size la taille en octets de la zone qu'on veut réserver
 * @return un pointeur void* vers l'espace mémoire réservé de taille minimale size 
 * @return 0 en cas d'erreur
 */
void *my_malloc(size_t size) {
    if (size == 0 || size > 64000 - 2) { //vérifie si demmande réaliste
        return 0;
    }

    uint16_t allocation_size = (uint16_t)((size + 1) & ~1)+2;
    int current = 0;
    // la mémoire pour le first_of sum est géré dans boucle interne
    while (current < 64000) {//tant qu'on est pas a la fin de la mémoire
        uint16_t header = read_at_index(current);//taille du bloc courant (avec flag)

        if (is_free(header)) {//si le bloc est libre
            uint32_t total_free = header;//on commence la somme
            int next = current + header;

            while (next < 64000) {//boucle interne pour fusionner des blocs libres
                uint16_t next_header = read_at_index(next);//taille du bloc suivant (avec flag)
                if (is_free(next_header)) {
                    total_free += next_header;//on ajoute sa taille a la somme et continue
                    next += next_header;
                } else {
                    break;//plus rien a sommer
                }
            }

            if (total_free >= allocation_size) { //si on a assez de place
                uint16_t reste = total_free - allocation_size;
                if (reste > 0) {
                    // On coupe le bloc : bloc occupé + nouveau bloc libre
                    write_at_index(current, allocation_size | 1);
                    write_at_index(current + allocation_size, reste);
                } else {
                    // Tout l'espace est pris même si il reste 2 bits libre (donc un en tête et 0 de place)
                    write_at_index(current, (uint16_t)total_free | 1);
                }
                return (void *)(MY_HEAP + current + 2);//on renvoie le pointeur vers le début de l'espace que l'utilisateur peut utiliser
            } else {
                // Trop petit, mais on fusionne pour la prochaine fois 
                //permet de ne pas avoir a refaire ce travail la prochaine fois
                write_at_index(current, (uint16_t)total_free);
                current = next;
            }
        } else {
            current += header & ~1;//on passe a la suite
        }
    }
    return 0;//pas de bloc suffisant trouvé
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