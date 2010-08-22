#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define kByte 1024
#define mByte (kByte * 1024)

int main(int argc, char **argv) {
    FILE *inFile;

    int i, j, opt, argfile;
    long size, blocks, null_blocks;
    double ratio;
    
    // options vars
    long block_size = 512 * kByte;
    //long check_bytes = 16;
    long check_bytes = 16;
    int max_free = 0;
    
    // options flags
    bool address = false;
    bool progress = false;
    bool help = false;
    bool verbose = false;
    bool map = false;
    bool quit = false;

    while ((opt = getopt(argc, argv, "b:c:f:aphvmq")) != -1) {
        switch (opt) {
            case 'b': block_size = atol(optarg) * kByte;
				break;
            case 'c': check_bytes = atol(optarg);
				break;
            case 'f': max_free = atol(optarg);
				break;
            case 'a': address = true;
				break;
			case 'p': progress = true;
				break;
            case 'h': help = true;
				break;
            case 'v': verbose = true;
				break;
            case 'm': map = true;
				break;
            case 'q': quit = true;
				break;
        }
    }

    if (argc == 1 || help) {
        printf("usage: %s [options] [files...]\n", argv[0]);
        printf("options:\n");
        printf("  -h\t\tshow this help\n");
        printf("  -m\t\tprint blocks map\n");
        printf("  -a\t\tshow null block address\n");
        printf("  -p\t\tshow progress\n");
        printf("  -v\t\tverbose output\n");
        printf("  -q\t\tquit(1) after check fail\n");
        printf("  -f PERCENT\tmax free percent\n");        
        printf("  -b SIZE\tblock size(KiB)\n");
        printf("  -c BYTES\tcheck bytes(default: 16)\n");
        return 1;
    }
    
    if (check_bytes == 0 || check_bytes > block_size)
		check_bytes = block_size;

    for (argfile = optind; argfile < argc; argfile++) {
        if ((inFile = fopen(argv[argfile], "r")) == NULL) {
            fprintf(stderr, "error: can't open input file: %s\n", argv[argfile]);
            continue;
        }

        fseek(inFile, 0, SEEK_END);
        size = ftell(inFile);

        if (size < block_size)
        {
			if (size % 4096 != 0) // hack: skip dir
				fprintf(stderr, "error: file size(%ld) smaller than block size: %s\n", size, argv[argfile]);
            fclose(inFile);
            continue;
        }

        blocks = size / block_size;
        null_blocks = 0;

		if (map)
            putchar('\n');

		ratio = 100./blocks;
        for (i = 0; i < blocks; i++) {
            fseek(inFile, i*block_size, SEEK_SET);
            for (j = 0; j < check_bytes; j++) {
                if (getc(inFile) != 0)
                    break;
            }
            
            if (j == check_bytes)
				null_blocks++;
			
			if (j == check_bytes) {
				if (map)
					putchar('_');
				if (address) {
					printf("0x%x", (unsigned int)(i * block_size));
					if (!map)
						putchar('\n');
				}
			} else {
				if (map)
					putchar('#');
			}
			
			if (progress)
				printf("\r%3.0f%%\t%s", i * ratio, argv[argfile]);
        }

        fclose(inFile);

        if (map)
            putchar('\n');

        double free = (double) null_blocks / blocks * 100;

		printf("\r%3.0f%%\t%s\n", 100 - free, argv[argfile]);
        if (verbose) {
            printf("\n\t\ttotal\tused\tfree\n");
            printf("percent:\t%d%%\t%.0f%%\t%.0f%%\n", 100, 100 - free, free);
            printf("size:   \t%ldM\t%ldM\t%ldM\n", size / mByte, (size - null_blocks * block_size) / mByte, null_blocks * block_size / mByte);
            printf("blocks: \t%ld \t%ld \t%ld\n\n", blocks, blocks - null_blocks, null_blocks);
        }
        if (quit && (int)free > max_free) {
            return 1;
        }
    }

    return 0;
}
