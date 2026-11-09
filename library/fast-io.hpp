#include <cassert>
#include <cstdio>
#include <string>
#include <string_view>
#include <type_traits>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/fast-io.hpp

// fread を使う整数・文字列入力。入力が非常に大きい問題でだけ使う。
// 使い方:
// FastInput input;
// int n = input.next<int>();
// string s = input.next<string>();
struct FastInput {
  static constexpr int BUFFER_SIZE = 1 << 16;

  std::FILE* file;
  char buffer[BUFFER_SIZE];
  int position = 0;
  int length = 0;

  explicit FastInput(std::FILE* file = stdin) : file(file) {}

  FastInput(const FastInput&) = delete;
  FastInput& operator=(const FastInput&) = delete;

  template <class T>
  std::enable_if_t<std::is_integral_v<T>, bool> read(T& value) {
    int character = skip_spaces();
    if (character == EOF) return false;

    bool negative = false;
    if (character == '-') {
      negative = true;
      character = get_character();
    }
    assert('0' <= character && character <= '9');

    using Unsigned = std::make_unsigned_t<T>;
    Unsigned result = 0;
    while ('0' <= character && character <= '9') {
      result = result * 10 + static_cast<unsigned>(character - '0');
      character = get_character();
    }
    if constexpr (std::is_signed_v<T>) {
      value = negative ? static_cast<T>(Unsigned{0} - result)
                       : static_cast<T>(result);
    } else {
      assert(!negative);
      value = static_cast<T>(result);
    }
    return true;
  }

  bool read(std::string& value) {
    int character = skip_spaces();
    if (character == EOF) return false;
    value.clear();
    while (character > ' ') {
      value.push_back(static_cast<char>(character));
      character = get_character();
    }
    return true;
  }

  bool read(char& value) {
    const int character = skip_spaces();
    if (character == EOF) return false;
    value = static_cast<char>(character);
    return true;
  }

  template <class T>
  T next() {
    T value{};
    const bool succeeded = read(value);
    assert(succeeded);
    return value;
  }

 private:
  int get_character() {
    if (position == length) {
      length =
          static_cast<int>(std::fread(buffer, 1, BUFFER_SIZE, file));
      position = 0;
      if (length == 0) return EOF;
    }
    return static_cast<unsigned char>(buffer[position++]);
  }

  int skip_spaces() {
    int character;
    do {
      character = get_character();
    } while (character != EOF && character <= ' ');
    return character;
  }
};

// fwrite を使う出力。endlのような毎回flushする操作は行わない。
// 使い方:
// FastOutput output;
// output.write_integer(answer, '\n');
// output.write_string("done\n");
struct FastOutput {
  static constexpr int BUFFER_SIZE = 1 << 16;

  std::FILE* file;
  char buffer[BUFFER_SIZE];
  int length = 0;

  explicit FastOutput(std::FILE* file = stdout) : file(file) {}
  ~FastOutput() { flush(); }

  FastOutput(const FastOutput&) = delete;
  FastOutput& operator=(const FastOutput&) = delete;

  void flush() {
    if (length == 0) return;
    std::fwrite(buffer, 1, length, file);
    length = 0;
  }

  void write_character(char character) {
    if (length == BUFFER_SIZE) flush();
    buffer[length++] = character;
  }

  void write_string(std::string_view text) {
    for (char character : text) write_character(character);
  }

  template <class T>
  std::enable_if_t<std::is_integral_v<T>> write_integer(
      T value, char after = '\0') {
    using Unsigned = std::make_unsigned_t<T>;
    Unsigned magnitude;
    if constexpr (std::is_signed_v<T>) {
      if (value < 0) {
        write_character('-');
        magnitude = Unsigned{0} - static_cast<Unsigned>(value);
      } else {
        magnitude = static_cast<Unsigned>(value);
      }
    } else {
      magnitude = value;
    }

    char digits[32];
    int digit_count = 0;
    do {
      digits[digit_count++] = static_cast<char>('0' + magnitude % 10);
      magnitude /= 10;
    } while (magnitude != 0);
    while (digit_count > 0) write_character(digits[--digit_count]);
    if (after != '\0') write_character(after);
  }
};
