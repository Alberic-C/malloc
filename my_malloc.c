#include <stdint.h>
#include <stdio.h>

/*
le bit le plus faible indique 0 si c'est libre 
                              1 si c'est occupé
*/

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
uint16_t intconverter_8t16(uint8_t int1, uint8_t int2){
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
    return 1-(int2 & 0x1);
}
/**
 * @return 1 if the number is a pointer to another bloc
 */
uint8_t is_link(uint8_t int2){
    return (int2 & 0x10)>1;
}

uint8_t MY_HEAP[64000];
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
    MY_HEAP[0] = size >> 8;
    MY_HEAP[1] = size & 0xE;
    //test -------------------------
    printf("bytes of total = ");
    printBits(size);
    printf("\névalué à %d\n", size);
    printf("bytes of first = ");
    printBits(size >> 8);
    printf("\névalué à %d\n", size >> 8);
    printf("bytes of second = ");
    printBits(MY_HEAP[1]);
    printf("\névalué à %d\n", MY_HEAP[1]);
    //test -------------------------

}

void my_free(void *pointer) {
    printf("pointeur donné : %p\n", pointer);
    printf("pointeur de MY_HEAP : %p\n", MY_HEAP);
    uint32_t index = (uint32_t)((uintptr_t)pointer - (uintptr_t)MY_HEAP);
    printf("index déduit : %d\n", index);
    uint16_t size = censored_intconverter(MY_HEAP[index-2], MY_HEAP[index-1]);
    if (index+size < 64000){
        uint16_t next_size_uncensor = intconverter(MY_HEAP[index+size+2-2], MY_HEAP[index+size+2-1]);
        if ((next_size_uncensor & 0x1) == 0){
            size += next_size_uncensor;
        }
    }

    //fusionner avec le précédent
    uint16_t current = 0;
    uint16_t old = 0;
    while (current < index-2)
    {
        old = current;
        current += censored_intconverter(MY_HEAP[current], MY_HEAP[current+1]);
        current += 2;
    }
    uint16_t prev_size_uncensor = intconverter(MY_HEAP[index+size+2-2], MY_HEAP[index+size+2-1]);
    if ((prev_size_uncensor & 0x1) == 0){
        size += prev_size_uncensor;
        index -= prev_size_uncensor;
        index -= 2;
    }
    MY_HEAP[index-2] = size << 8;
    MY_HEAP[index-1] = size & 0xfe;
}

void *my_malloc(size_t size){
    return 0;
}


int main(int argc, char* argv[]){
    printf("test-----------test\n");
    init();
    my_free((void*)&MY_HEAP+2);
    printf("test : \n");

    printf("test : \n");
    printf("--------------fin du code----------\n");
    return 0;
}