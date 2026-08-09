#include "console.h"

int main() {
    conclr();
    conputc('!');
    conreverse();
    conputs("Hello world from C!\r\n");
    connormal();
    conputs("Press any key to continue\r\n");
    while (congetc() != EOF);
    while (congetc() == EOF);
    return 0;
}
