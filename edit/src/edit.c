#include "console.h"

int main() {
    conclr();
    conputc('!');
    conreverse();
    consetcrs(20,10);
    conputs("Hello world from C!\r\n");    
    connormal();
    concrsdown(4);
    concrsright(10);
    conputs("Press any key to continue");
    while (congetc() != EOF);
    while (congetc() == EOF);
    conclrline();
    concrsleft(80);
    return 0;
}
