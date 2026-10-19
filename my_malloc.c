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
   for(int bit=0;bit<(sizeof(unsigned int) * 8); bit++)
   {
      if (bit%8 == 0)
      {
        printf("| ");
      }
      printf("%i ", num & 0x01);
      num = num >> 1;
   }
}
uint16_t intconverter(uint8_t int1, uint8_t int2){
    return (int1<<8) + int2;
}
uint16_t censored_intconverter(uint8_t int1, uint8_t int2){
    uint8_t censored_2 = int2 & 0x11111110;
    return (int1<<8) + censored_2;
}
/**
 * @return 1 if the next part is free
 */
uint8_t is_free(uint8_t int2){
    return (int2 & 0x1)==0;
}
/**
 * @return 1 if the number is a pointer to another bloc
 */
uint8_t is_link(uint8_t int2){
    return (int2 & 0x10)>1;
}
void write_at_index(int index, int16_t to_write){
    MY_HEAP[index] = to_write >> 8;
    MY_HEAP[index+1] = to_write & 0xff;
}
int read_at_index(int index){
    return (MY_HEAP[index]<<8) + MY_HEAP[index+1] ;
}
int pointer_to_index(void* ptr){
    int index = (uint32_t)((uintptr_t)ptr - (uintptr_t)MY_HEAP);
    return index;
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
void init(){
    uint16_t size = 64000-2;
    write_at_index(0, size);
    //test -------------------------
    printf("bytes of total = ");
    printBits(size);
    printf("\névalué à %d\n", size);
    printf("bytes of first = ");
    printBits(size >> 8);
    printf("\névalué à %d\n", size >> 8);
    printf("bytes of second = ");
    printBits(size & 0xff);
    printf("\névalué à %d\n", size & 0xff);

    printf("lu : %d\n", read_at_index(0));
    //test -------------------------

}

void *my_malloc(size_t size){
    int available_size = read_at_index(0);
    int fix_size = size + (size&0b1); // s'assurer que le nombre de bites est pair
    printf("malloc de size %zu est passé à %d\n", size, fix_size);
    if (is_free(available_size) && available_size-2 >= size) //segment libre et de bonne taille
    {
        printf("first trouvé\n");
        int new_size = available_size-2-fix_size;
        write_at_index(fix_size+2, new_size);
        write_at_index(0, fix_size+1); //+1 pour dire que c'est occupé
        return (void*) &(MY_HEAP[2]);
    }
    
    int current = 0;
    int to_sum = 0;
    while (current<= 64000-1)//((1-is_free(available_size)) || available_size<size) //le segement est occupé ou la taille est trop petite
    {
        printf("dans la boucle\n");
        available_size = read_at_index(current);
        if (is_free(available_size))
        {
            printf("bloc libre trouvé !, de taille :%d\n", available_size);
            to_sum+=available_size;
            if (to_sum-2 >= size){
                printf("on a la bonne taille pour donner\n");
                write_at_index(current, size+1);
                if (current+size+4<=64000) //check que le prochain bloc est possible d'écrire
                {
                    printf("et il reste de la suite\n");
                    //il faut encore vérif si il y a des blocs après sinon on écrase
                    write_at_index(current+size+2, to_sum-size-2);
                }
                else
                {
                    printf("mais on est a la fin \n");
                }
                return (void*) &(MY_HEAP[current+2]);
            }
        }
        else
        {
            printf("bloc occupé , suite\n");
            to_sum = 0;
        }
        current +=1+available_size;
    }
    
    return 0;
}

void my_free(void *pointer) {
    printf("pointeur donné : %p\n", pointer);
    printf("pointeur de MY_HEAP : %p\n", MY_HEAP);
    int index = pointer_to_index(pointer);
    printf("index déduit : %d\n", index);
    int size = read_at_index(index-2);
    if (is_free(size))
    {
        printf("already free ??\n");
        return;
    }
    write_at_index(index-2, size-1);
}


int main(int argc, char* argv[]){
    printf("test-----------test\n");
    init();
    printBits(0xff);
    printf("\n");
    printf("test malloc 13 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(13)) );
    printf("fini malloc 13 :\n");
    printf("premier entete : %d\n", read_at_index(0));
    printf("second entete : %d\n", read_at_index(14+2));
    printf("devrait etre : %d\n", 64000-2-14-2);
    printf("test isfree : %d\n", is_free(read_at_index(0)));
    printf("test isfree rempli : %d\n", is_free(read_at_index(14+2)));
    printf("test false : %d\n", 1==2);
    printf("test malloc 26 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(26)) );
    printf("fini malloc 26 :\n");
    printf("premier entete : %d\n", read_at_index(0));
    printf("second entete : %d\n", read_at_index(14+2));
    printf("3e     entete : %d\n", read_at_index(14+2+26+2));
    printf("devrait etre : %d\n", 64000-2-14-2-26-2);
    my_free((void*)&(MY_HEAP[2])); //pour l'instant c'est segfault
    printf("premier entete : %d\n", read_at_index(0));
    printf("second entete : %d\n", read_at_index(14+2));
    printf("test malloc 8 :\n");
    printf("résult malloc : %d\n", pointer_to_index(my_malloc(8)) );
    printf("fini malloc 8 :\n");
    printf("premier entete : %d\n", read_at_index(0));
    printf("second entete : %d\n", read_at_index(8+2));
    printf("3e     entete : %d\n", read_at_index(14+2));
    printf("4e     entete : %d\n", read_at_index(14+2+26+2));
    printf("--------------fin du code----------\n");
    return 0;
}