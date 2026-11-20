#define AHC_CYCLE_SEED
#define main ahc004_solver_main
#include "../problems/ahc004/main.cpp"
#undef main

int main() {
  std::mt19937 rng(74004);
  const auto rotate = [](const std::string& s, int p) {
    return s.substr(p) + s.substr(0,p);
  };
  const auto canon = [&](std::string s) {
    std::string result = s;
    for (int p=0; p<20; ++p) result = std::min(result, rotate(s,p));
    return result;
  };
  std::vector<std::string> words;
  std::set<std::string> truth;
  for (int i=0; i<8; ++i) {
    std::string line;
    for (int p=0; p<20; ++p) line += char('A'+(rng()&7));
    truth.insert(canon(line));
    for (int p=0; p<20; ++p) words.push_back(rotate(line,p).substr(0,9));
  }
  std::vector<int> frequency(words.size(), 1);
  auto out = ahc004::observed_cycles(20, words, frequency);
  assert(!out.capped && out.visits < 16000);
  int recovered = 0;
  std::set<std::string> observed;
  for (const auto& word : words)
    for (size_t t=0; t+5<=word.size(); ++t)
      observed.insert(word.substr(t,5));
  std::set<std::string> emitted;
  for (const auto& c : out.cycles) {
    assert(canon(c.letters) == c.letters && emitted.insert(c.letters).second);
    recovered += truth.count(c.letters);
    for (int p=0; p<20; ++p) assert(observed.count(rotate(c.letters,p).substr(0,5)));
    for (size_t i=0; i<words.size(); ++i) {
      const std::string wrapped = c.letters+c.letters;
      const bool match = wrapped.find(words[i]) < 20;
      assert(match == std::binary_search(c.words.begin(),c.words.end(),i));
    }
  }
  assert(recovered == 8);
  auto capped = ahc004::observed_cycles(20, words, frequency, 8);
  assert(capped.visits == 8 && capped.capped);
  assert(ahc004::observed_cycles(20, {"AA","AB"}, {1,1}).cycles.empty());
  assert(ahc004::observed_cycles(20, words, frequency, 16000,
                                [] { return true; }).cycles.empty());
  for (int variant=0; variant<4; ++variant) {
    auto board = seed_from_cycles(20, words.size(), out.cycles, variant, Timer{}, 500);
    int complete = 0, cols = 0;
    for (int lane=0; lane<20; ++lane) {
      std::string col;
      for (const auto& row : board) col += row[lane];
      if (board[lane].find('.') == std::string::npos) {
        assert(emitted.count(canon(board[lane])));
        ++complete;
      }
      if (col.find('.') == std::string::npos) {
        assert(emitted.count(canon(col)));
        ++cols;
      }
    }
    assert(complete >= 2 && cols >= 1);
  }
}
