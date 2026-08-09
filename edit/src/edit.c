#include "console.h"

int main() {
    conclr();
    consetcrs(25,1);
    conreverse();
    for (int i = 0; i < 80; i++) {
        conputc(' ');
    }
    concrsleft(80);
    conputs("/SOMEFILE.TXT");
    consetcrs(25,70);
    conputs("1/???");    
    connormal();
    consetcrs(1,1);
    int lines=8;
    const char *text[]={
        "This is some bogus text",
        "to test out the appearance of my sick text editor",
        "bleh"
        "as asdadsjadsjoidsaiojdsajiosadioajdsaidojsdisoj",
        "ls /",
        "if a=b then",
        "  load coolr driver",
        "fi"
    };
    for (int i = 0; i < lines; i++) {
        if (i > 0) {
            conputs("\r\n");
        }
        conbold();
        conputc(i+'0');
        conputc(':');
        connormal();
        conputs(text[i]);
    }
    while (congetc() != EOF);
    while (congetc() == EOF);
    conclrline();
    concrsleft(80);
    return 0;
}
