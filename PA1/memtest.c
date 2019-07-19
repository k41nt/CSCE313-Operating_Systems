/*#include "ackerman.h"

int main(int argc, char ** argv) {

  int basic block size, memory length;

  init_allocator(basic block size, memory length)

  ackerman_main(); // this is the full-fledged test. 
  // The result of this function can be found from the ackerman wiki page or https://www.wolframalpha.com/. If you are not getting correct results, that means that your allocator is not working correctly. In addition, the results should be repetable - running ackerman (3, 5) should always give you the same correct result. If it does not, there must be some memory leakage in the allocator that needs fixing
  
  // please make sure to run small test cases first before unleashing ackerman. One example would be running the following: "print_allocator (); x = my_malloc (1); my_free(x); print_allocator();" the first and the last print should be identical.
  
  release_allocator()
}*/
#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "my_allocator.h"
#include "ackerman.h"

/* memtest [-b <blocksize>] [-s <memsize>]
-b <blocksize> defines the block size, in bytes. Default is 128 bytes.
-s <memsize> defines the size of the memory to be allocated, in bytes.
Default is 512kB.
*/
void show_help(){
        printf("the command is as followed\n");
        printf( "memtest [-b <blocksize>] [-s <memsize>]\n");
        printf("-b <blocksize> is the block size, in bytes. Default is 128 bytes.\n");
        printf("-s <memsize> is the size of the memory to be allocated, in bytes. Default is 512kB\n");


}


int main(int argc, char ** argv)
{
    int c;
    unsigned int b=128;
    unsigned int m=5242880; //actual size of 512 kB
/*
    if (argc<3)
        show_help();

    while ((c = getopt(argc, argv, "b:s:")) != -1)
        switch (c)
        {
        case 'b':
              b=atoi(optarg);
            break;
        case 's':
               m=atoi(optarg);
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt)){
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                show_help();
            }else{
                fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
                show_help();
                        }
                return 1;
        default:
            abort ();
        }*/

    init_allocator(b,m);

    ackerman_main();

    atexit((void(*)())release_allocator);//cast into correct function type
}