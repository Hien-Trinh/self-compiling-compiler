#include <stdio.h>
#include <string.h>

char* concat(char* str1, char* str2);
char* itos(int x);
char* ctos(char c);

// Comment
int main() {
    // Comment
    char* a = "hello";
    int b = 12345;
    int c[10];
    int i = 0;
    while ((i < 10)) {
        c[i] = i;
        i = (i + 1);
    }
    i = 0;
    while ((i < 10)) {
        printf("%d\n", c[i]);
        i = (i + 1);
    }
    return 0;
}

char* concat(char* str1, char* str2) {
    static char buf[1024];
    snprintf(buf, sizeof(buf), "%s%s", str1, str2);
    return buf;
}

char* itos(int x) {
    static char buf[32];
    snprintf(buf, sizeof(buf), "%d", x);
    return buf;
}

char* ctos(char c) {
    static char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    return buf;
}

