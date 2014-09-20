#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "uthash.h"

off_t fsize(const char*);

struct node {
	int id;
	uint32_t* array;
	int len;
	UT_hash_handle hh;
};

int main(int argc, char* argv[]){
	if ( argc != 2 ) {
		printf( "usage: %s program.umz\n", argv[0]);
		return 1;
	}
	uint32_t reg[] = {0,0,0,0,0,0,0,0};
	struct node *collection = NULL;
	struct node *head = (struct node*)malloc(sizeof(struct node));
	uint32_t pc = 0;
	uint32_t serial = 1;
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
	int i;
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
	/*
	for(i = 0; i < (head->len); i++){
		printf("%d:%08x\n",i,(head->array)[i]);

	}
	*/
	struct node *n;
	while(1){
		if(pc > head->len){
			printf("out of bounds: tried to access %d; max %d\n",pc,head->len);
			return 2;
		}
		uint32_t platter = (head->array)[pc];
		pc++;
		char opcode = (platter >> 28);
		char A = (platter >> 6) & 0b111;
		char B = (platter >> 3) & 0b111;
		char C = platter & 0b111;
		if(0){
			printf("platter:%08x\top:%d\tA:%d\tB:%d\tC:%d\tpc:%d\n",platter,opcode,A,B,C,pc-1);
			int i;
			for(i = 0; i < 8; i++) printf("%" PRIu32 " ", reg[i]);
			puts("");
			getchar();
		}
		switch(opcode){
		case 0:
			if(reg[C] != 0)
				reg[A] = reg[B];
			break;
		case 1: 
			n = NULL;
			HASH_FIND_INT(collection, &(reg[B]), n);
			if(n == NULL){
				printf("opcode 1, no such array found:%d\n",reg[B]);
				return 2;
			}
			reg[A] = (n->array)[reg[C]];
			break;
		case 2: 
			n = NULL;
			HASH_FIND_INT(collection, &(reg[A]), n);
			if(n == NULL){
				puts("opcode 2, no such array found");
				return 2;
			}
			if(reg[B] > (n->len)){
				printf("opcode 2, out of bounds: accessing %d with max %d\n", reg[B], n->len);
				return 2;
			}
			(n->array)[reg[B]] = reg[C];
			
			break;
		case 3:
			reg[A] = reg[B] + reg[C];
			break;
		case 4:
			reg[A] = reg[B] * reg[C];
			break;
		case 5:
			if(reg[C] == 0){
				puts("can't divide by zero");
				return 2;
			}
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
			if(reg[C] == 0){
				puts("can't free the 0 array");
				return 2;
			}
			struct node *n = NULL;
			HASH_FIND_INT(collection, &(reg[C]), n);
			if(n == NULL){
				puts("opcode 9, no such array found");
				return 2;
			}
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
			if(n == NULL){
				puts("opcode 12, no such array found");
				return 2;
			}
			head->array = (uint32_t*) realloc(head->array,(n->len) * sizeof(uint32_t));
			for(i = 0; i < n->len; i++)
				(head->array)[i] = (n->array)[i];
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
