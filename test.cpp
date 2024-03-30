#include <unordered_set>
#include <iostream>

int main() {
    std::unordered_set<int> executableQueue;
    for (auto i : executableQueue) {
        printf("%d ", i);
    }
    printf("\n");
}