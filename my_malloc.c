#include <stdint.h>
#include <stdio.h>

/*
le bit le plus faible indique 0 si c'est libre
                              1 si c'est occupé
        false = 0
        true = 1
*/
uint8_t MY_HEAP[64000];

void printBits(unsigned int num)
{
    for (int bit = 0; bit < (sizeof(unsigned int) * 8); bit++)
    {
        if (bit % 8 == 0)
        {
            printf("| ");
        }
        printf("%i ", num & 0x01);
        num = num >> 1;
    }
}
/**
 * @return 1 if the next part is free
 */
uint8_t is_free(uint8_t int2)
{
    return (int2 & 0x1) == 0;
}
void write_at_index(int index, int16_t to_write)
{
    MY_HEAP[index] = to_write >> 8;
    MY_HEAP[index + 1] = to_write & 0xff;
}
int read_at_index(int index)
{
    return (MY_HEAP[index] << 8) + MY_HEAP[index + 1];
}
int pointer_to_index(void *ptr)
{
    int index = (uint32_t)((uintptr_t)ptr - (uintptr_t)MY_HEAP);
    return index;
}
int censor(int a)
{
    return a - (a & 0b1);
}
void read_mem()
{
    printf("read_mem ------------------\n");
    int current = 0;
    int iter = 0;
    int nb_blocs = 0;
    int sum_blocs = 0;
    while (current <= 64000 - 1)
    {
        int size = read_at_index(current);
        int fix_size = size - (size & 0b1);
        if (is_free(size))
        {
            printf("%d : bloc libre de taille : %d\n", current, size);
        }
        else
        {
            printf("%d : bloc occupé de taille : %d\n", current, fix_size);
        }
        current += 2 + fix_size;
        iter++;
        nb_blocs++;
        sum_blocs += fix_size + 2;
        if (iter > 10)
        {
            printf("|||||||||||| iter trop grand → break\n");
            break;
        }
    }
    printf("nb_blocs : %d de somme : %d\n", nb_blocs, sum_blocs);
    printf("------------------  mem read\n");
}

/**
 * j'ai besoin d'un flag free/empty donc on divise la taille par 2
 * uint va jusque 255 ... donc on a besoin de 2 uint8_t pour aller jusque 64000
 * on ne peux aller plus haut (128000 est hors de uint16_t)
 * pour les blocs occupés ont mettra 2 int de taille au début et 2 int de taille a la fin
 * donc +4 int de taille
 * pour les libres ont peut essayer de pointer vers  le suivant et précédent donc encore +4
 * pour être sur d'avoir la place on peut forcer une taille minimale de 4 afin d'éviter les micro frees/malloc
 * on peut allors plutot diviser la taille par 4 et donc assurer d'aviter les problèmes de taille a la fin
 * ??? et ainsi rajouter un flag si c'est un lien vers un autre bloc ou pas ??? pas sur que ce soit nécessaire
 **/
void init()
{
    uint16_t size = 64000 - 2;
    write_at_index(0, size);
    // test -------------------------
    /*printf("bytes of total = ");
    printBits(size);
    printf("\névalué à %d\n", size);
    printf("bytes of first = ");
    printBits(size >> 8);
    printf("\névalué à %d\n", size >> 8);
    printf("bytes of second = ");
    printBits(size & 0xff);
    printf("\névalué à %d\n", size & 0xff);

    printf("lu : %d\n", read_at_index(0));
    */
    // test -------------------------
}

void *my_malloc(size_t size)
{
    if (size <= 0 || size > 64000 - 2)
    {
        return 0;
    }
    int fix_size = size + (size & 0b1); // s'assurer que le nombre de bites est pair
    //printf("malloc de size %zu est passé à %d\n", size, fix_size);
    int available_size = read_at_index(0);
    if (is_free(available_size) && available_size - 2 >= fix_size) // segment libre et de bonne taille
    {
        //printf("first trouvé\n");
        int new_size = available_size - 2 - fix_size;
        write_at_index(0, fix_size + 1); //+1 pour dire que c'est occupé
        write_at_index(fix_size + 2, new_size);
        return (void *)&(MY_HEAP[2]);
    }

    int current = 0;
    int first_of_sum = 0;
    int to_sum = 0;
    while (current < 64000 - 1) //((1-is_free(available_size)) || available_size<size) //le segement est occupé ou la taille est trop petite
    {
        //printf("dans la boucle\n");
        available_size = read_at_index(current);
        if (is_free(available_size))
        {
            //printf("bloc libre trouvé !, de taille :%d +2\n", available_size);
            to_sum += available_size + 2;
            if (to_sum - 2 >= size)
            {
                //printf("on a la bonne taille pour donner\n");
                write_at_index(first_of_sum, fix_size + 1);                            //+1 car occupé
                if (first_of_sum + 2 + fix_size <= 64000 - 2 && to_sum - 2 > fix_size) // check que le prochain bloc est possible d'écrire
                {
                    //printf("et il reste de la suite\n");
                    // il faut encore vérif si il y a des blocs après sinon on écrase
                    write_at_index(first_of_sum + fix_size + 2, to_sum - fix_size - 2 - 2);
                }
                /*else
                {
                    printf("mais on est a la fin ou on est tout juste \n");
                }*/
                return (void *)&(MY_HEAP[first_of_sum + 2]);
            }
        }
        else
        {
            //printf("bloc occupé , suite\n");
            to_sum = 0;
            first_of_sum = current + 2 + censor(available_size);
        }
        current += 2 + censor(available_size);
    }
    return 0;
}

void my_free(void *pointer)
{
    if (!pointer){
        //printf("ptr null\n");
        return;
    }
    //printf("pointeur donné : %p\n", pointer);
    //printf("pointeur de MY_HEAP : %p\n", MY_HEAP);
    int index = pointer_to_index(pointer);
    //verif pointer null et dans my heap
    if (index > 64000-1 || index <2)
    {
        //printf("ptr out of bounds, index : %d\n", index);
        return;
    }
    
    //printf("index déduit : %d\n", index);
    int size = read_at_index(index - 2);
    if (is_free(size))
    {
        printf("already free ?? : %d read : %d\n", index, size);
        return;
    }
    //printf("good free : %d size : %d\n", index, size);
    write_at_index(index - 2, size - 1);
}

int main(int argc, char *argv[])
{
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