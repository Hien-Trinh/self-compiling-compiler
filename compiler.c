#include <stdio.h>
#include <string.h>

int is_letter(int c) {
    return ((((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) || (c == '_'));
}

int is_digit(int c) {
    return ((c >= '0') && (c <= '9'));
}

int is_space(int c) {
    return (((c == ' ') || (c == '\n')) || (c == '\t'));
}

int check_keywords(int s, int i, int kw) {
    int x = 0;
    printf("%d\n", x);
    return x;
}

int main() {
    int x = 0;
    while ((x < 5)) {
        printf("%d\n", x);
        x = (x + (1 * 3));
    }
    return 0;
}
