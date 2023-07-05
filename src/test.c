#include <stdbool.h>
#include <stdio.h>

bool run_tests();

int main() {
    bool const result = run_tests();
    printf("Tests %s\n", result? "passed" : "failed");
}