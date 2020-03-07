#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

#define POOL_SIZE 100*MEGABYTE

enum BLOCK_HDR_FLAGS
{
	FLAG_FREE		= 1 << 0,
};

typedef struct block_hdr block_hdr;
typedef struct block_hdr
{
	block_hdr *prev;
	block_hdr *next;
	unsigned int size;
	int flags;
} block_hdr;
