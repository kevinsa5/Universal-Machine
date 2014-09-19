#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <inttypes.h>

off_t fsize(const char*);

struct node {
	int id;
	uint32_t* array;
	int len;
	struct node *next;
};

int main(int argc, char* argv[]){
	if ( argc != 2 ) {
		printf( "usage: %s program.umz\n", argv[0]);
		return 1;
	}
	uint32_t reg[] = {0,0,0,0,0,0,0,0};
	struct node *collection = (struct node*)malloc(sizeof(struct node));
	uint32_t pc = 0;
	uint32_t serial = 1;
	FILE *progfile;
	progfile = fopen(argv[1],"rb");
	if(!progfile){
		printf("Unable to open %s", argv[1]);
		return 1;
	}
	collection->id = 0;
	collection->len = fsize(argv[1])/sizeof(uint32_t);
	collection->next = 0;
	uint32_t *buffer;
	buffer = malloc((collection->len) * sizeof(uint32_t));
	fread(buffer, 4, (collection->len), progfile);	
	//swap endianness? not kosher to assume, but...
	int i;
	for(i = 0; i < (collection->len); i++){
		uint32_t temp = buffer[i];
		buffer[i] = ((temp & 0xFF000000) >> 24) | 
					((temp & 0x00FF0000) >> 8)  |
					((temp & 0x0000FF00) << 8)  |
					((temp & 0x000000FF) << 24);
	}
	collection->array = buffer;
	puts("executing");
	/*
	for(i = 0; i < (collection->len); i++){
		printf("%d:%08x\n",i,(collection->array)[i]);

	}
	*/
	struct node* iter;
	while(1){
		if(pc > collection->len){
			printf("out of bounds: tried to access %d; max %d\n",pc,collection->len);
			return 2;
		}
		uint32_t platter = (collection->array)[pc];
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
			iter = collection;
			while(iter != 0){
				printf("%d ", iter->id);
				iter = iter->next;
			}
			getchar();
		}
		switch(opcode){
		case 0:
			if(reg[C] != 0)
				reg[A] = reg[B];
			break;
		case 1:
			iter = collection;
			while(iter != 0){
				if (iter->id == reg[B]){
					if(reg[C] > (iter->len)){
						printf("opcode 1, out of bounds\n");
						return 2;
					}
					reg[A] = (iter->array)[reg[C]];
					break;
				}
				iter = iter->next;
			}
			if(iter == 0){
				printf("opcode 1, no such array found:%d\n",reg[B]);
				return 2;
			}
			break;
		case 2:
			iter = collection;
			while(iter != 0){
				if (iter->id == reg[A]){
					if(reg[B] > (iter->len)){
						printf("opcode 2, out of bounds: accessing %d with max %d\n", reg[B], iter->len);
						return 2;
					}
					(iter->array)[reg[B]] = reg[C];
					break;
				}
				iter = iter->next;
			}
			if(iter == 0){
				puts("opcode 2, no such array found");
				return 2;
			}
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
			// labels must be followed by statements, not declarations
			;
			struct node *new = (struct node*)malloc(sizeof(struct node));
			new->id = serial;
			new->len = reg[C];
			new->next = 0;
			uint32_t *arr;
			arr = malloc((new->len) * sizeof(uint32_t));
			int i;
			for(i = 0; i < new->len; i++) arr[i] = (uint32_t) 0;
			new->array = arr;
			struct node *second = collection->next;
			collection->next = new;
			new->next = second;
			reg[B] = new->id;
			serial++;
			break;
		case 9:
			if(reg[C] == 0){
				puts("can't free the 0 array");
				return 2;
			}
			iter = collection;
			struct node* prev;
			while(iter != 0){
				if (iter->id == reg[C]){
					prev->next = iter->next;
					free(iter->array);
					free(iter);
					break;
				}
				prev = iter;
				iter = iter->next;
			}
			if(iter == 0){
				printf("opcode 9, no such array found: %d\n", reg[C]);
				return 2;
			}
			break;
		case 10:
			printf("%c", reg[C]);
			break;
		case 11:
			printf("input:");
			reg[C] = getchar();
			break;
		case 12:
			iter = collection;
			while(iter != 0){
				if (iter->id == reg[B]){
					collection->array = (uint32_t*) realloc(collection->array,(iter->len) * sizeof(uint32_t));
					int i;
					for(i = 0; i < iter->len; i++)
						(collection->array)[i] = (iter->array)[i];
					collection->len = iter->len;
					pc = reg[C];
					break;
				}
				iter = iter->next;
			}
			if(iter == 0){
				puts("opcode 12 no such array");
				return 2;
			}
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
