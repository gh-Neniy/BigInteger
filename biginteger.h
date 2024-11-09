#include <cstring>
#include <iostream>
#include <string>
#include <vector>

class BigInteger {
public:
  BigInteger(int);
  BigInteger(const char*);

  BigInteger(const BigInteger&) = default;
  BigInteger& operator=(const BigInteger&) = default;

  ~BigInteger() = default;

  const std::string toString() const;

  BigInteger& operator+=(const BigInteger&);
  BigInteger& operator-=(const BigInteger&);
  BigInteger& operator*=(const BigInteger&);
  BigInteger& operator/=(const BigInteger&);

  BigInteger operator-() const;
  BigInteger& operator++();
  BigInteger operator++(int);
  BigInteger& operator--();
  BigInteger operator--(int);

  explicit operator bool() const;

  bool isZero() const;
  bool isNegative() const;
  const std::vector<int>& getBlocks() const;

  const int compareWith(const BigInteger&) const;

private:
  int carryCheck(int);

  static const int cMaxBlock = 1e9;
  bool is_negative;
  std::vector<int> vect; // Блоки по 9 цифр
};

BigInteger::BigInteger(int value = 0) : is_negative(value < 0),
                                        vect{is_negative ? value = -value : value} {
  if (value >= cMaxBlock) {
    vect.push_back(value / cMaxBlock);
  }
  vect[0] %= cMaxBlock;
}
BigInteger::BigInteger(const char* str) : is_negative(*str == '-') {
  if (is_negative) { ++str; }
  while (*str == '0') { ++str; }

  int i = strlen(str) - 1;
  int block = 0;
  for (; i >= 9; i -= 9) { // Считывание по 9 цифр блоками с конца строки
    for (int j = 0; j < 9; ++j) {
      block *= 10;
      block += str[i - 8 + j] - '0';
    }
    vect.push_back(block);
    block = 0;
  }
  for (int k = 0; k <= i; ++k) { // Считывание оставшегося неполного блока
    block *= 10;
    block += str[k] - '0';
  }
  vect.push_back(block);
}

bool operator==(const BigInteger& lhs, const BigInteger& rhs) {
  return lhs.compareWith(rhs) == 0;
}
bool operator!=(const BigInteger& lhs, const BigInteger& rhs) {
  return lhs.compareWith(rhs) != 0;
}
bool operator<(const BigInteger& lhs, const BigInteger& rhs) {
  return lhs.compareWith(rhs) == -1;
}
bool operator>=(const BigInteger& lhs, const BigInteger& rhs) {
  return lhs.compareWith(rhs) != -1;
}
bool operator>(const BigInteger& lhs, const BigInteger& rhs) {
  return lhs.compareWith(rhs) == 1;
}
bool operator<=(const BigInteger& lhs, const BigInteger& rhs) {
  return lhs.compareWith(rhs) != 1;
}

const std::string BigInteger::toString() const {
  std::string bigint = is_negative ? "-" : "";
  bigint += std::to_string(vect.back());
  for (int i = static_cast<int>(vect.size()) - 2; i >= 0; --i) {
    std::string block_str = std::to_string(vect[i]);
    if (block_str.size() < 9) { // Дополнение нулями (до 9 цифр ровно)
      block_str = std::string(9 - block_str.size(), '0') + block_str;
    }
    bigint += block_str;
  }
  return bigint;
}

inline int BigInteger::carryCheck(int index) {
  if (vect[index] >= cMaxBlock) {
      vect[index] %= cMaxBlock;
      return 1;
  }
  return 0;
}
BigInteger& BigInteger::operator+=(const BigInteger& rhs) {
  if (!is_negative && rhs.is_negative) {
    BigInteger tmp = rhs;
    tmp.is_negative = false; // Для корректного вычитания знак менятся на +
    *this -= tmp;
    return *this;
  } else if (is_negative && !rhs.is_negative){
    BigInteger tmp = rhs;
    is_negative = false;
    tmp -= *this;
    *this = tmp;
    return *this;
  }

  if (rhs.vect.size() > vect.size()) {
    vect.resize(rhs.vect.size());
  }

  int n = std::min(rhs.vect.size(), vect.size()); // vect.size() может быть больше, чем rhs.vect.size()
  int carry = 0;
  int i = 0;
  for (; i < n; ++i) {
    vect[i] += rhs.vect[i] + carry;
    carry = carryCheck(i);
  }
  while (carry == 1) {
    if (i == vect.size()) {
      vect.push_back(1);
      carry = 0;
    } else {
      vect[i] += carry;
      carry = carryCheck(i++);
    }
  }
  return *this;
}
BigInteger& BigInteger::operator-=(const BigInteger& rhs) {
  if (!is_negative && rhs.is_negative) { // lhs-(-rhs) = lhs + rhs
    BigInteger tmp = rhs;
    tmp.is_negative = false;
    *this += tmp;
    return *this;
  } else if (is_negative && !rhs.is_negative) { // -lhs - rhs = -(lhs + rhs)
    is_negative = false;
    *this += rhs;
    is_negative = true;
    return *this;
  } else if (is_negative && rhs.is_negative) { // -lhs-(-rhs) = rhs - lhs
    BigInteger tmp = rhs;
    tmp.is_negative = false;
    is_negative = false;
    tmp -= *this;
    *this = tmp;
    return *this;
  }

  // Остаётся случай, когда оба числа положительны
  if(rhs > *this) {
    BigInteger tmp = rhs;
    tmp -= *this;
    *this = tmp;
    is_negative = true;
    return *this;
  }

  // *this > rhs, оба положительны
  int n = rhs.vect.size();
  for (int i = 0; i < n; ++i) {
    vect[i] -= rhs.vect[i];
    if (vect[i] < 0) { // Последнее значение в vect не станет < 0
      --vect[i + 1];
      vect[i] += 1e9;
    }
  }
  while (vect.back() == 0) {
    vect.pop_back();
  }
  return *this;
}

BigInteger BigInteger::operator-() const {
  BigInteger new_bigint = *this;
  new_bigint.is_negative ^= 1;
  return new_bigint;
}
BigInteger& BigInteger::operator++() {
  if (is_negative) {
    is_negative = false;
    --*this;
    if(!isZero()) { is_negative = true; }
    return *this;
  }

  int carry = 1;
  int i = 0;
  while (carry == 1) {
    if (i == vect.size()) {
      vect.push_back(carry);
      break;
    } else {
      vect[i] += carry;
      if (vect[i] >= cMaxBlock) {
        vect[i] %= cMaxBlock;
        carry = 1;
      } else {
        carry = 0;
      }
      ++i;
    }
  }
  return *this;
}
BigInteger BigInteger::operator++(int) {
  BigInteger copy = *this;
  ++*this;
  return copy;
}
BigInteger& BigInteger::operator--() {
  if (is_negative) {
    is_negative = false;
    ++*this;
    is_negative = true;
    return *this;
  } else if (isZero()) {
    vect[0] = 1;
    is_negative = true;
    return *this;
  }

  int i = 0;
  vect[i] -= 1;
  while (vect[i] < 0) { // Случай, когда точно есть следующий блок
    --vect[i + 1];
    vect[i] += 1e9;
    i++;
  }
  if (vect.back() == 0 && vect.size() > 1) {
    vect.pop_back();
  }
  return *this;
}
BigInteger BigInteger::operator--(int) {
  BigInteger copy = *this;
  --*this;
  return copy;
}

BigInteger::operator bool() const {
  return !isZero();
}

bool BigInteger::isZero() const {
  return vect.size() == 1 && vect[0] == 0;
}
bool BigInteger::isNegative() const {
  return is_negative;
}
const std::vector<int>& BigInteger::getBlocks() const {
  return vect;
}

const int BigInteger::compareWith(const BigInteger& rhs) const {
  if (isNegative() != rhs.isNegative()) {
      return isNegative() ? -1 : 1;
    }
    // Иначе знаки равны
    int sz = getBlocks().size();
    int rhs_sz = rhs.getBlocks().size();
  
    if (sz == rhs_sz) {
      for (int i = sz - 1; i >= 0; --i) {
        if (getBlocks()[i] < rhs.getBlocks()[i]) {
          return isNegative() ? 1 : -1;
        }
      }
      return 0;
    } else {
      if (isNegative()) { // оба числа отрицательные
        return sz > rhs_sz ? -1 : 1;
      }
      return sz < rhs_sz ? -1 : 1;
    }
}

BigInteger operator""_bi(const char* str) { // Для чисел
  return BigInteger(str);
}
BigInteger operator""_bi(const char* str, size_t) { // Для чисел в виде строки
  return BigInteger(str);
}

std::ostream& operator<<(std::ostream& os, const BigInteger& rhs) {
  os << rhs.toString();
  return os;
}
std::istream& operator>>(std::istream& is, BigInteger& rhs) {
  std::string str;
  is >> str;
  rhs = BigInteger(str.c_str());
  return is;
}

const BigInteger operator+(const BigInteger& lhs, const BigInteger& rhs) {
  BigInteger sum = lhs;
  sum += rhs;
  return sum;
}
const BigInteger operator-(const BigInteger& lhs, const BigInteger& rhs) {
  BigInteger diff = lhs;
  diff -= rhs;
  return diff;
}
