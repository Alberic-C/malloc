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
    uint8_t censored_2 = int2 & 0x11111100;
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

}

void *my_malloc(size_t size){
    return 0;
}


int main(int argc, char* argv[]){
    printf("test-----------test\n");
    init();
    uint8_t one = 250;
    uint8_t two = 0;
    printf("valeur renvoyé par intconverter = %d\n", intconverter_8t16(one, two));
    printf("test : \n");
    printBits(0xfe);
    printf("test : \n");
    printf("--------------fin du code----------\n");
    return 0;
}