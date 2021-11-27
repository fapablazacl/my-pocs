
#include <stdio.h>
#include <atari.h>

int main() {
    char ch;
    char buffer[16];

    for (ch = 0; ch<126; ch++) {
        printf("%d  %c\n", (int)ch, ch);
    }
    
    scanf("%s", buffer);
    printf("Hello, %s! \n", buffer);
    
    return 0;
}
