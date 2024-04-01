#include <vector>
#include <unordered_map>
#include <iostream>

int main() {
std::vector<std::pair<int, int>> pairs = {{7, 8}, {6, 9}};
std::unordered_map<int, int> map;

for (int i = 0; i < pairs.size(); ++i) {
    map[pairs[i].first] = i;
    map[pairs[i].second] = i;
}

int query = 6;
auto it = map.find(query);

if (it != map.end()) {
    std::cout << "Index of " << query << " is " << it->second << "\n";
} else {
    std::cout << "Value " << query << " not found\n";
}}