#include <stdio.h>
int main() {  
    char* c[26] = {"stuff"};
    
    // %d displays the integer value of a character
    // %c displays the actual character
    printf("ASCII value of %s\n", c[0]);
    
    return 0;
}