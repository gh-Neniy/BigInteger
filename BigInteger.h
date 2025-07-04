#pragma once
#include <limits.h>

#include <compare>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#ifndef NENIY_BIGINTEGER
#define NENIY_BIGINTEGER

class BigInteger {
 public:
  using BlockT = int64_t;  // Для случая с -INT_MIN
  static const int cMaxBlock = 1'000'000'000;
  static const int cBlockSize = 9;

  BigInteger(int /*value*/ = 0);

  explicit BigInteger(unsigned long long /*value*/, size_t /*unused*/);

  explicit BigInteger(const char* /*str*/);

  std::string toString() const;

  BigInteger& operator+=(const BigInteger& /*rhs*/);

  BigInteger& operator-=(const BigInteger& /*rhs*/);

  BigInteger& operator*=(const BigInteger& /*rhs*/);

  BigInteger& operator/=(const BigInteger& /*rhs*/);

  BigInteger& operator%=(const BigInteger& /*rhs*/);

  BigInteger operator-() const;

  BigInteger& operator++();

  BigInteger& operator--();

  BigInteger operator++(int);

  BigInteger operator--(int);

  explicit operator bool() const;

  void FlipSign();

  bool IsZero() const;

  bool IsNegative() const;

  const std::vector<BlockT>& GetBlocks() const;

 private:
  void IncrementLogic();

  void DecrementLogic();

  std::pair<BigInteger, BigInteger> DivMod(const BigInteger& /*rhs*/) const;

  bool is_negative_;
  std::vector<BlockT> blocks_;  // Блоки по 9 цифр (little-endian)
};

std::strong_ordering operator<=>(const BigInteger& lhs, const BigInteger& rhs) {
  if (lhs.IsNegative() != rhs.IsNegative()) {
    return lhs.IsNegative() ? std::strong_ordering::less
                            : std::strong_ordering::greater;
  }
  // Иначе знаки равны
  int sz = lhs.GetBlocks().size();
  int rhs_sz = rhs.GetBlocks().size();

  if (sz != rhs_sz) {
    if (lhs.IsNegative()) {
      return sz > rhs_sz ? std::strong_ordering::less
                         : std::strong_ordering::greater;
    }
    return sz < rhs_sz ? std::strong_ordering::less
                       : std::strong_ordering::greater;
  }
  // Иначе размеры равны
  for (int i = sz - 1; i >= 0; --i) {
    if (lhs.GetBlocks()[i] != rhs.GetBlocks()[i]) {
      if (lhs.GetBlocks()[i] < rhs.GetBlocks()[i]) {
        return lhs.IsNegative() ? std::strong_ordering::greater
                                : std::strong_ordering::less;
      }
      return lhs.IsNegative() ? std::strong_ordering::less
                              : std::strong_ordering::greater;
    }
  }
  return std::strong_ordering::equal;
}

bool operator==(const BigInteger& lhs, const BigInteger& rhs) {
  return operator<=>(lhs, rhs) == std::strong_ordering::equal;
}

BigInteger operator""_bi(unsigned long long number) {
  return BigInteger(number, 0U);
}

BigInteger operator""_bi(const char* str,
                         size_t /*unused*/) {  // Для чисел в виде строки
  return BigInteger(str);
}

std::ostream& operator<<(std::ostream& os, const BigInteger& rhs) {
  os << rhs.toString();
  return os;
}
std::istream& operator>>(std::istream& is, BigInteger& rhs) {
  char digit;
  bool signgot = false;
  std::string str;
  while (is.get(digit) && std::isspace(digit)) {
  }
  do {
    if (isdigit(digit) || (digit == '-' && signgot == false)) {
      str.push_back(digit);
      signgot = true;
    } else {
      is.unget();
      break;
    }
  } while (is.get(digit));
  rhs = BigInteger(str.c_str());
  return is;
}

BigInteger operator+(const BigInteger& lhs, const BigInteger& rhs) {
  BigInteger sum = lhs;
  sum += rhs;
  return sum;
}
BigInteger operator-(const BigInteger& lhs, const BigInteger& rhs) {
  BigInteger diff = lhs;
  diff -= rhs;
  return diff;
}
BigInteger operator*(const BigInteger& lhs, const BigInteger& rhs) {
  BigInteger product = lhs;
  product *= rhs;
  return product;
}
BigInteger operator/(const BigInteger& lhs, const BigInteger& rhs) {
  BigInteger product = lhs;
  product /= rhs;
  return product;
}
BigInteger operator%(const BigInteger& lhs, const BigInteger& rhs) {
  BigInteger remains = lhs;
  remains %= rhs;
  return remains;
}

BigInteger::BigInteger(int value) : is_negative_(value < 0) {
  if (value == INT_MIN) {
    blocks_.push_back(static_cast<BlockT>(INT_MAX) + 1);
    blocks_.push_back(blocks_[0] / cMaxBlock);
    blocks_[0] %= cMaxBlock;
    return;
  }
  if (is_negative_) {
    value = -value;
    blocks_.push_back(value);
  } else {
    blocks_.push_back(value);
  }
  if (value >= cMaxBlock) {  // Если value отрицательное, знак меняется на плюс
    blocks_.push_back(value / cMaxBlock);
    blocks_[0] %= cMaxBlock;
  }
}
BigInteger::BigInteger(unsigned long long value, size_t /*unused*/)
    : is_negative_(false) {
  if (value == 0) {
    blocks_.push_back(0);
  }
  while (value > 0) {
    blocks_.push_back(value % cMaxBlock);
    value /= cMaxBlock;
  }
}
BigInteger::BigInteger(const char* str) : is_negative_(*str == '-') {
  if (is_negative_) {
    ++str;
  }
  while (*str == '0') {
    ++str;
  }

  if (*str == '\0') {
    is_negative_ = false;
    blocks_.push_back(0);
    return;
  }

  int i = strlen(str) - 1;
  int block = 0;
  for (; i >= cBlockSize;
       i -= cBlockSize) {  // Считывание по 9 цифр блоками с конца строки
    for (int j = 0; j < cBlockSize; ++j) {
      block *= 10;
      block += str[i - cBlockSize + j + 1] - '0';
    }
    blocks_.push_back(block);
    block = 0;
  }
  for (int k = 0; k <= i; ++k) {  // Считывание оставшегося неполного блока
    block *= 10;
    block += str[k] - '0';
  }
  blocks_.push_back(block);
}

std::string BigInteger::toString() const {
  // Без дополнения нулями (старший разряд)
  std::string bigint =
      (is_negative_ ? "-" : "") + std::to_string(blocks_.back());

  int sz = blocks_.size();
  for (int i = sz - 2; i >= 0; --i) {
    std::string block_str = std::to_string(blocks_[i]);
    if (block_str.size() < cBlockSize) {  // Дополнение нулями (до 9 цифр ровно)
      block_str = std::string(cBlockSize - block_str.size(), '0') + block_str;
    }
    bigint += block_str;
  }
  return bigint;
}

BigInteger& BigInteger::operator+=(const BigInteger& rhs) {
  if (!is_negative_ && rhs.is_negative_) {  // lhs + (-rhs) = lhs - rhs
    *this -= -rhs;
  } else if (is_negative_ && !rhs.is_negative_) {  // -lhs + rhs = rhs - lhs
    is_negative_ = false;
    *this = rhs - *this;
  } else {
    if (rhs.blocks_.size() > blocks_.size()) {
      blocks_.resize(rhs.blocks_.size());
    }
    int rhs_sz = rhs.blocks_.size();
    int sz = blocks_.size();
    int carry = 0;
    int i = 0;

    for (; i < rhs_sz; ++i) {  // Поблочное сложение
      blocks_[i] += rhs.blocks_[i] + carry;
      carry = blocks_[i] / cMaxBlock;
      blocks_[i] %= cMaxBlock;
    }
    while (carry != 0) {  // Обработка остатка
      if (i == sz) {
        blocks_.push_back(carry);
        break;
      }
      blocks_[i] += carry;
      carry = blocks_[i] / cMaxBlock;
      blocks_[i++] %= cMaxBlock;
    }
  }
  return *this;
}
BigInteger& BigInteger::operator-=(const BigInteger& rhs) {
  if (!is_negative_ && rhs.is_negative_) {  // lhs-(-rhs) = lhs + rhs
    *this += -rhs;
  } else if (is_negative_ && !rhs.is_negative_) {  // -lhs - rhs = -(lhs + rhs)
    is_negative_ = false;
    *this += rhs;
    is_negative_ = true;
  } else if (is_negative_ && rhs.is_negative_) {  // -lhs-(-rhs) = rhs - lhs
    BigInteger tmp = rhs;
    tmp.is_negative_ = false;
    is_negative_ = false;
    tmp -= *this;
    *this = tmp;
  } else {
    // Остаётся случай, когда оба числа положительны
    if (rhs > *this) {
      BigInteger tmp = rhs;
      tmp -= *this;
      *this = tmp;
      is_negative_ = true;
    } else {
      int rhs_sz = rhs.blocks_.size();
      int lhs_sz = blocks_.size();
      int i = 0;
      for (; i < rhs_sz; ++i) {
        blocks_[i] -= rhs.blocks_[i];
        if (blocks_[i] < 0) {  // Последнее значение в blocks_ не станет < 0,
                               // так как *this >= rhs
          --blocks_[i + 1];
          blocks_[i] += cMaxBlock;
        }
      }
      while (i < lhs_sz && blocks_[i] < 0) {
        --blocks_[i + 1];
        blocks_[i] += cMaxBlock;
        ++i;
      }
      while (blocks_.back() == 0 && blocks_.size() > 1UL) {
        blocks_.pop_back();
      }
    }
  }
  return *this;
}
BigInteger& BigInteger::operator*=(const BigInteger& rhs) {
  if (IsZero() || rhs.IsZero()) {
    *this = 0;
  } else {
    is_negative_ = is_negative_ != rhs.is_negative_;
    int sz = blocks_.size();
    int rhs_sz = rhs.blocks_.size();
    std::vector<BlockT> new_blocks(sz + rhs_sz);

    for (int i = 0; i < rhs_sz; ++i) {  // Умножение "в столбик"
      int carry = 0;
      for (int j = 0; j < sz || carry != 0; ++j) {
        long long rhs_block = rhs.blocks_[i];
        long long block_product =
            new_blocks[i + j] + (j < sz ? rhs_block * blocks_[j] : 0) + carry;
        new_blocks[i + j] = block_product % cMaxBlock;
        carry = block_product / cMaxBlock;
      }
    }
    while (new_blocks.back() == 0 && new_blocks.size() > 1) {
      new_blocks.pop_back();
    }
    blocks_ = new_blocks;
  }
  return *this;
}
BigInteger& BigInteger::operator/=(const BigInteger& rhs) {
  *this = DivMod(rhs).first;
  return *this;
}
BigInteger& BigInteger::operator%=(const BigInteger& rhs) {
  *this = DivMod(rhs).second;
  return *this;
}

BigInteger BigInteger::operator-() const {
  if (IsZero()) {
    return 0;
  }

  BigInteger new_bigint = *this;
  new_bigint.is_negative_ ^= 1;
  return new_bigint;
}

void BigInteger::IncrementLogic() {
  int sz = blocks_.size();
  bool carry = true;
  int i = 0;
  while (i < sz && carry) {
    ++blocks_[i];
    if (blocks_[i] >= cMaxBlock) {
      blocks_[i] %= cMaxBlock;
    } else {
      carry = false;
    }
    ++i;
  }
  if (i == sz && static_cast<int>(carry) != 0) {
    blocks_.push_back(1);
  }
}
BigInteger& BigInteger::operator++() {
  if (is_negative_ && blocks_.size() == 1 && blocks_[0] == 1) {
    *this = 0;
  } else if (is_negative_) {
    DecrementLogic();
  } else {
    IncrementLogic();
  }
  return *this;
}
void BigInteger::DecrementLogic() {
  int i = 0;
  --blocks_[i];
  while (blocks_[i] < 0) {  // decrementLogic никогда не вызывается с 0
    --blocks_[i + 1];
    blocks_[i] += cMaxBlock;
    ++i;
  }
  if (blocks_.back() == 0 && blocks_.size() > 1) {
    blocks_.pop_back();
  }
}

std::pair<BigInteger, BigInteger> BigInteger::DivMod(
    const BigInteger& rhs) const {
  if (rhs.IsZero()) {
    throw std::runtime_error("Division by zero.");
  }

  BigInteger divisor = rhs;
  divisor.is_negative_ = false;

  BigInteger result;
  result.blocks_.clear();
  BigInteger block;  // Текущий блок цифр, который делится
  block.blocks_.clear();

  int sz = blocks_.size();
  for (int i = sz - 1; i >= 0; --i) {
    block.blocks_.insert(block.blocks_.begin(), blocks_[i]);
    if (block < divisor) {
      result.blocks_.insert(result.blocks_.begin(), 0);
      while (!block.blocks_.empty() && block.blocks_.back() == 0) {
        block.blocks_.pop_back();
      }
      continue;
    }
    int left = 0;  // Частное
    int right = cMaxBlock;
    while (right - left > 1) {
      int mid = (left + right) / 2;
      if (mid * divisor <= block) {
        left = mid;
      } else {
        right = mid;
      }
    }
    block -= left * divisor;
    result.blocks_.insert(result.blocks_.begin(), left);
    while (!block.blocks_.empty() && block.blocks_.back() == 0) {
      block.blocks_.pop_back();
    }
  }
  while (result.blocks_.back() == 0 && result.blocks_.size() > 1) {
    result.blocks_.pop_back();
  }
  if (result.IsZero()) {
    result.is_negative_ = false;
  } else {
    result.is_negative_ = is_negative_ != rhs.is_negative_;
  }
  if (block.blocks_.empty()) {
    block.blocks_.push_back(0);
    block.is_negative_ = false;
  } else {
    block.is_negative_ = is_negative_;
  }
  return {result, block};
}

BigInteger& BigInteger::operator--() {
  if (IsZero()) {
    *this = -1;
  } else if (is_negative_) {
    IncrementLogic();
  } else {
    DecrementLogic();
  }
  return *this;
}
BigInteger BigInteger::operator++(int) {
  BigInteger copy = *this;
  ++*this;
  return copy;
}
BigInteger BigInteger::operator--(int) {
  BigInteger copy = *this;
  --*this;
  return copy;
}

BigInteger::operator bool() const { return !IsZero(); }

void BigInteger::FlipSign() {
  if (IsZero()) {
    return;
  }
  is_negative_ ^= 1;
}

bool BigInteger::IsZero() const {
  return blocks_.size() == 1 && blocks_[0] == 0;
}
bool BigInteger::IsNegative() const { return is_negative_; }
const std::vector<BigInteger::BlockT>& BigInteger::GetBlocks() const {
  return blocks_;
}

#endif // NENIY_BIGINTEGER
