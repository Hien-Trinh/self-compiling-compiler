#include <stdio.h>
#include <string.h>

int is_letter(int c) {
    return ((((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) || (c == '_'));
}

int is_digit(int c) {
    return ((c >= '0') && (c <= '9'));
}

int is_space(char c) {
    return (((c == ' ') || (c == '\n')) || (c == '\t'));
}

int check_keywords(int s, int i, int kw) {
    int x = 0;
    printf("%s\n", (concat(itos(is_digit(x)), "hello")));
    return x;
}

int main() {
    int x = 0;
    char y = 'a';
    while ((x < 5)) {
        printf("%d\n", x);
        x = (x + 1);
        printf("%d\n", (y * x));
        printf("%s\n", "yipee");
    }
    return 0;
}

char* concat(char* str1, char* str2) {
    static char buf[1024];
    strcpy(buf, str1);
    strcat(buf, str2);
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

