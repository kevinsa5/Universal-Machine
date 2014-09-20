#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "uthash.h"

#ifndef DEBUG
#define DEBUG 0
#endif

off_t fsize(const char*);

struct node {
	int id;
	uint32_t* array;
	int len;
	UT_hash_handle hh;
};

uint32_t reg[] = {0,0,0,0,0,0,0,0};
struct node *collection = NULL;
struct node *head;
uint32_t pc = 0;
uint32_t serial = 1;
struct node *n;
int i;
char opcode, A,B,C;
uint32_t platter;

int main(int argc, char* argv[]){
	if ( argc != 2 ) {
		printf( "usage: %s program.umz\n", argv[0]);
		return 1;
	}
	head = (struct node*)malloc(sizeof(struct node));
	FILE *progfile;
	progfile = fopen(argv[1],"rb");
	if(!progfile){
		printf("Unable to open %s", argv[1]);
		return 1;
	}
	head->id = 0;
	head->len = fsize(argv[1])/sizeof(uint32_t);
	uint32_t *buffer;
	buffer = malloc((head->len) * sizeof(uint32_t));
	fread(buffer, 4, (head->len), progfile);	
	//swap endianness? not kosher to assume, but...
	for(i = 0; i < (head->len); i++){
		uint32_t temp = buffer[i];
		buffer[i] = ((temp & 0xFF000000) >> 24) | 
					((temp & 0x00FF0000) >> 8)  |
					((temp & 0x0000FF00) << 8)  |
					((temp & 0x000000FF) << 24);
	}
	head->array = buffer;
	HASH_ADD_INT(collection, id, head);
	puts("executing");

	while(1){
		#if DEBUG>0
		if(pc > head->len){
			printf("out of bounds: tried to access %d; max %d\n",pc,head->len);
			return 2;
		}
		#endif
		platter = (head->array)[pc];
		pc++;
		opcode = (platter >> 28);
		A = (platter >> 6) & 0b111;
		B = (platter >> 3) & 0b111;
		C = platter & 0b111;
		#if DEBUG>0
		if(0){
			printf("platter:%08x\top:%d\tA:%d\tB:%d\tC:%d\tpc:%d\n",platter,opcode,A,B,C,pc-1);
			int i;
			for(i = 0; i < 8; i++) printf("%" PRIu32 " ", reg[i]);
			puts("");
			getchar();
		}
		#endif
		switch(opcode){
		case 0:
			if(reg[C] != 0)
				reg[A] = reg[B];
			break;
		case 1: 
			n = NULL;
			HASH_FIND_INT(collection, &(reg[B]), n);
			#if DEBUG>0
			if(n == NULL){
				printf("OC1, no array found:%d\n",reg[B]);
				return;
			}
			#endif
			reg[A] = (n->array)[reg[C]];
			break;
		case 2: 
			n = NULL;
			HASH_FIND_INT(collection, &(reg[A]), n);
			#if DEBUG>0
			if(n == NULL){
				puts("opcode 2, no such array found");
				return;
			}
			if(reg[B] > (n->len)){
				printf("opcode 2, out of bounds: accessing %d with max %d\n", reg[B], n->len);
				return;
			}
			#endif
			(n->array)[reg[B]] = reg[C];
			break;
		case 3:
			reg[A] = reg[B] + reg[C];
			break;
		case 4:
			reg[A] = reg[B] * reg[C];
			break;
		case 5:
			#if DEBUG>0
			if(reg[C] == 0){
				puts("can't divide by zero");
				return;
			}
			#endif
			reg[A] = reg[B] / reg[C];
			break;
		case 6:
			reg[A] = (~reg[B]) | (~reg[C]);
			break;
		case 7:
			puts("halting");
			return 0;
		case 8:
			n = (struct node*)malloc(sizeof(struct node));
			n->id = serial;
			n->len = reg[C];

			uint32_t *arr;
			arr = malloc((n->len) * sizeof(uint32_t));
			for(i = 0; i < reg[C]; i++) arr[i] = (uint32_t) 0;
			n->array = arr;
			HASH_ADD_INT(collection, id, n);
			reg[B] = n->id;
			serial++;
			break;
		case 9:
			#if DEBUG>0
			if(reg[C] == 0){
				puts("can't free the 0 array");
				return;
			}
			#endif
			n = NULL;
			HASH_FIND_INT(collection, &(reg[C]), n);
			#if DEBUG>0
			if(n == NULL){
				puts("opcode 9, no such array found");
				return;
			}
			#endif
			HASH_DEL(collection, n);
			free(n->array);
			free(n);
			break;
		case 10:
			printf("%c", reg[C]);
			break;
		case 11:
			printf("input:");
			reg[C] = getchar();
			break;
		case 12: 
			n = NULL;
			HASH_FIND_INT(collection, &(reg[B]), n);
			#if DEBUG>0
			if(n == NULL){
				puts("opcode 12, no such array found");
				return;
			}
			#endif
			if(n->len > head->len) head->array = (uint32_t*) realloc(head->array,(n->len) * sizeof(uint32_t));
			memcpy(head->array, n->array, (n->len)*sizeof(uint32_t));
			head->len = n->len;
			pc = reg[C];
			break;
		case 13:
			A = (platter >> 25) & 0b111;
			uint32_t val = platter & ((1<<25) - 1);
			reg[A] = val;
			break;
		default:
			printf("invalid opcode: %d\n", opcode);
			return 2;
		}
	}
	puts("bottom of main");
}

//printf("%" PRIu32 "\n", buffer[i]);

off_t fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}
