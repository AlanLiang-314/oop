#include <cstdlib>
#include <iostream>
#include <utility>

int main() {
  int a, b, c, d, e;
  std::cin >> a >> b >> c >> d >> e;
  if (e < 100) {  // < 1e2 NO-OUTPUT
    return 0;
  }
  if (e < 1'000) {  // < 1e3 WRONG-ANSWER
    std::cout << "WA!\n";
    return 0;
  }
  if (e < 10'000) {  // < 1e4 RUN-ERROR
    std::abort();
  }
  if (e < 100'000) {  // < 1e5 TIMELIMIT
    int ball_1 = 10, ball_2 = 20;
    while (true) std::swap(ball_1, ball_2);  // 我叫他：盤核桃（兩顆一直換位子）
  }
}
// :)