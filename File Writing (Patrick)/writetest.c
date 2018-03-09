#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

int main(void){
        FILE *fw;
        char* filewritename = "filewrite.txt";
	fw = fopen(filewritename,"w");
	fprintf(fw,"hi");
}
