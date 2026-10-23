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
uint8_t is_free(uint8_t int2){
    return (int2 & 0x1) == 0;
}
/**
 * @param index l'index dans MY_HEAP auquel il faut écrire
 * @param to_write l'en tête à écrire
 * cette fonction écrit l'en tête demandé a l'index demandé dans MY_HEAP
 */
void write_at_index(int index, uint16_t to_write){
    MY_HEAP[index] = to_write >> 8;
    MY_HEAP[index + 1] = to_write & 0xff;
}
/**
 * @param index l'index dans MY_HEAP qu'il faut lire
 * @return l'en tête a cet index de MY_HEAP
 */
int read_at_index(int index){
    return (MY_HEAP[index] << 8) + MY_HEAP[index + 1];
}
/**
 * @param ptr le pointeur recu
 * @return l'index dans MY_HEAP que ça représente (peut être out of bounds)
 */
int pointer_to_index(void *ptr){
    int index = (uint32_t)((uintptr_t)ptr - (uintptr_t)MY_HEAP);
    return index;
}
/**
 * retire le bit de poids faible de @param a
 * permet d'utiliser la taille de a sans se sousier de si le bloc est libre ou pas
 */
int censor(int a){
    return a - (a & 0b1);
}
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
void my_free(void *pointer){
    if (!pointer){//vérifie que le pointeur n'est pas nul
        return;
    }
    int index = pointer_to_index(pointer);
    if (index > 64000-1 || index <2){//vérifie que l'index est bien dans MY_HEAP
        return;
    }
    
    int size = read_at_index(index - 2);
    if (is_free(size)){//si c'est déja libre on ne fait rien
        return;
    }
    write_at_index(index - 2, censor(size));//change le bit de poids faible pour marquer le bloc comme libre
}
/**
 * @param size la taille en octets de la zone qu'on veut réserver
 * @return un pointeur void* vers l'espace mémoire réservé
 * @return 0 en cas d'erreur
 */
void *my_malloc(size_t size){
    if (size <= 0 || size > 64000 - 2){//vérifie si la demande est réaliste
        return 0;
    }
    int allocation_size = size + (size & 0b1) +2; // s'assurer que le nombre de bits est pair et on ajoute la taille de l'en tête
    int current = 0; 
    int first_of_sum = 0;
    int to_sum = 0;
    while (current <= 64000 - 2){
        int available_size = read_at_index(current);//taille du bloc actuel
        int block_size = censor(available_size);//sans le bit de poids faible
        if (is_free(available_size)){//si le bloc est libre
            to_sum += block_size;//on le somme afin de voir si il faut le fusionner avec d'autres
            if (to_sum >= allocation_size){ //si on a la place
                int remaining_size = to_sum - allocation_size;//on calcule le restant 
                if (remaining_size >= 2){//si on a la place d'écrire l'en tête suivant
                    write_at_index(first_of_sum, allocation_size + 1);
                    write_at_index(first_of_sum + allocation_size, remaining_size);
                }
                else{//sinon on ne touche pas au bloc suivant
                    write_at_index(first_of_sum, to_sum + 1);
                }
                return (void *)&(MY_HEAP[first_of_sum + 2]);//on renvoie le pointeur vers le début de l'espace que l'utilisateur peut utiliser
            }
        }
        else{
            to_sum = 0;
            first_of_sum = current + block_size;//on remets la somme a zero et réinitialise le pointeur de début de somme pour le prochain bloc
        }
        current += block_size;//on prends le bloc suivant dans tout les cas par pas encore trouvé la place
    }
    return 0;//on n'as pas trouvé de bloc suffisant
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
        else
        {
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

void test1(){
    //segfault risc : 
    write_at_index(64000, 42);
    printf("test-----------test\n");
    init();
    printBits(0xff);
    printf("\n");
    printf("--test malloc 13 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(13)));
    printf("--fini malloc 13 :\n");
    read_mem();
    printf("devrait etre : 16, %d\n", 64000-16);
    // printf("test false : %d\n", 1==2);
    printf("--test malloc 26 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(26)));
    printf("--fini malloc 26 :\n");
    read_mem();
    printf("devrait etre : 16, 28, %d\n", 64000 - 16-28);
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
    printf("read after : %d\n", read_at_index(64000));
}
void test_rapport(){
    init();
    read_mem();
    my_malloc(1);
    read_mem();
    my_malloc(17);
    read_mem();
    void* malloc_13 = my_malloc(13);
    read_mem();
    my_free(malloc_13);
    read_mem();
}

int main(int argc, char *argv[]){
    //test_rapport();
    printf("add1 : %d %d %d\n", 0b1111101000000000, 0b101, 0b1111100111011000);
    return 0;
}