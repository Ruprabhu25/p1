#include <stdio.h>
#include <string.h>

int main() {
  char *second_half;
  char buf[] = "echo hello > myfile.txt";
  char first_half[50];

  second_half = strchr (buf, '>');
  memcpy(first_half,buf,second_half-buf);
  printf ("found a '>': %s, %s \n", first_half,second_half);
  printf("%ld\n", second_half - buf);
 

  return 0;
}