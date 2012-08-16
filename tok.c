#include <stdio.h>
#include <string.h>
#include <ctype.h>

char *trim(char *str);

int main() {
 char s[] = "CSESSID=435J43L523J4TTYYY; email=rt@example.com";
 char* kv;
 char **items;
 
 kv = strtok(s, ";");
 
 while (kv != NULL) {
     printf("%s\n", kv);
     *items = &kv
     kv = strtok(NULL, ";");
 }
 return 0;
}



char *trim(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}