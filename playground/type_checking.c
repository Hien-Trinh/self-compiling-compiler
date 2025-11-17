#include <stdio.h>
#include <string.h>

char* concat(char* str1, char* str2);
char* itos(int x);
char* ctos(char c);
int x = 0;

// Comment
void test() {
}

int main() {
  // Comment
  char* fn_type = "int";
  char* fn_name = "main";
  char* param_list = "";
  char* body = "printf(\"\%d\\n\", global_var);";
  printf("%s\n", concat(concat(concat(concat(concat(concat(concat(fn_type, " "), fn_name), "("), param_list), ") {\n"), body), "\n}"));
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
