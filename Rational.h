#include "BigInteger.h"

class Rational {
 public:
  Rational(int /*num*/ = 0, int /*denum*/ = 1);

  Rational(const BigInteger& /*num*/, const BigInteger& /*denum*/ = 1);

  Rational& operator+=(const Rational& /*rhs*/);

  Rational& operator-=(const Rational& /*rhs*/);

  Rational& operator*=(const Rational& /*rhs*/);

  Rational& operator/=(const Rational& /*rhs*/);

  Rational operator-() const;

  std::string toString() const;

  std::string asDecimal(size_t /*precision*/) const;

  bool IsNegative() const;

  const BigInteger& GetNumerator() const { return numerator_; }

  const BigInteger& GetDenominator() const { return denominator_; }

  explicit operator double() const;

 private:
  static BigInteger Gcd(BigInteger /*a*/, BigInteger /*b*/);

  void SetNegative();

  void Simplify();

  static void DecimalIncrementation(std::string& /*decimal*/);

  BigInteger numerator_;
  BigInteger denominator_;
};

std::strong_ordering operator<=>(const Rational& lhs, const Rational& rhs) {
  if (lhs.IsNegative() != rhs.IsNegative()) {
    return lhs.IsNegative() ? std::strong_ordering::less
                            : std::strong_ordering::greater;
  }
  BigInteger lhs_numerator = lhs.GetNumerator() * rhs.GetDenominator();
  BigInteger rhs_numerator = rhs.GetNumerator() * rhs.GetDenominator();

  if (lhs_numerator == rhs_numerator) {
    return std::strong_ordering::equal;
  }
  return lhs_numerator < rhs_numerator ? std::strong_ordering::less
                                       : std::strong_ordering::greater;
}

bool operator==(const Rational& lhs, const Rational& rhs) {
  return operator<=>(lhs, rhs) == std::strong_ordering::equal;
}

Rational operator+(const Rational& lhs, const Rational& rhs) {
  Rational sum = lhs;
  sum += rhs;
  return sum;
}
Rational operator-(const Rational& lhs, const Rational& rhs) {
  Rational diff = lhs;
  diff -= rhs;
  return diff;
}
Rational operator*(const Rational& lhs, const Rational& rhs) {
  Rational product = lhs;
  product *= rhs;
  return product;
}
Rational operator/(const Rational& lhs, const Rational& rhs) {
  Rational quotient = lhs;
  quotient /= rhs;
  return quotient;
}

BigInteger Rational::Gcd(BigInteger a, BigInteger b) {
  if (a.IsNegative()) {
    a.FlipSign();
  }
  if (b.IsNegative()) {
    b.FlipSign();
  }
  BigInteger tmp;
  while (!b.IsZero()) {
    tmp = b;
    b = a % b;
    a = tmp;
  }
  return a;
}
void Rational::SetNegative() {  // Починка знака (знак должен быть только у
                                // числителя)
  if (numerator_.IsNegative() != denominator_.IsNegative()) {
    if (!numerator_.IsNegative()) {  // Если числитель положительный,
                                     // знаменатель отрицательный
      numerator_.FlipSign();
      denominator_.FlipSign();
    }
  } else {
    if (numerator_.IsNegative()) {  // Если числитель и знаменатель отрицательны
      numerator_.FlipSign();
      denominator_.FlipSign();
    }
  }
}

void Rational::Simplify() {
  auto divider = Gcd(numerator_, denominator_);
  if (divider != 1) {
    numerator_ /= divider;
    denominator_ /= divider;
  }
}

void Rational::DecimalIncrementation(std::string& decimal) {
  int i = decimal.size() - 1;
  bool carry = true;
  while (carry && decimal[i] != '.') {
    if (decimal[i] == '9') {
      decimal[i] = '0';
    } else {
      ++decimal[i];
      return;
    }
    --i;
  }
  if (carry) {
    if (decimal.back() == '.') {
      decimal.pop_back();
      decimal = (++BigInteger(decimal.c_str())).toString();
    } else {
      std::string integer(decimal.begin(), decimal.begin() + i);
      std::string after_dot(decimal.begin() + i + 1, decimal.end());
      decimal = (++BigInteger(integer.c_str())).toString() + '.' + after_dot;
    }
  }
}

Rational::Rational(int num, int denum) : numerator_(num), denominator_(denum) {
  Simplify();
  SetNegative();
}
Rational::Rational(const BigInteger& num, const BigInteger& denum)
    : numerator_(num), denominator_(denum) {
  Simplify();
  SetNegative();
}

Rational& Rational::operator+=(const Rational& rhs) {
  numerator_ = numerator_ * rhs.denominator_ + rhs.numerator_ * denominator_;
  denominator_ *= rhs.denominator_;

  Simplify();
  return *this;
}
Rational& Rational::operator-=(const Rational& rhs) {
  numerator_ = numerator_ * rhs.denominator_ - rhs.numerator_ * denominator_;
  denominator_ *= rhs.denominator_;

  Simplify();
  return *this;
}
Rational& Rational::operator*=(const Rational& rhs) {
  numerator_ *= rhs.numerator_;
  denominator_ *= rhs.denominator_;

  Simplify();
  return *this;
}
Rational& Rational::operator/=(const Rational& rhs) {
  if (this == &rhs) {
    *this = 1;
  } else {
    numerator_ *= rhs.denominator_;
    denominator_ *= rhs.numerator_;

    Simplify();
    SetNegative();
  }
  return *this;
}

Rational Rational::operator-() const {
  Rational new_rational = *this;
  new_rational.numerator_.FlipSign();
  return new_rational;
}

std::string Rational::toString() const {
  if (denominator_ == 1) {
    return numerator_.toString();
  }
  return numerator_.toString() + '/' + denominator_.toString();
}

std::string Rational::asDecimal(size_t precision) const {
  if (denominator_ == 1) {
    if (precision != 0) {
      return numerator_.toString() + '.' + std::string(precision, '0');
    }
    return numerator_.toString();
  }

  auto dividend = numerator_;
  if (dividend.IsNegative()) {
    dividend.FlipSign();
  }
  auto quotient = dividend / denominator_;
  std::string str_decimal = quotient.toString() + '.';  // целая часть
  dividend -= quotient * denominator_;

  int real_prec = precision / BigInteger::cBlockSize + 1;
  int to_delete = real_prec * BigInteger::cBlockSize - precision;
  while (real_prec != 0) {
    dividend *= BigInteger::cMaxBlock;  // Добавление нулевого блока
    quotient = dividend / denominator_;
    std::string str_quotient = quotient.toString();
    if (str_quotient.size() < BigInteger::cBlockSize) {
      str_quotient =
          std::string(BigInteger::cBlockSize - str_quotient.size(), '0') +
          str_quotient;
    }
    str_decimal += str_quotient;
    dividend -= quotient * denominator_;

    --real_prec;
  }
  while (to_delete > 1) {
    str_decimal.pop_back();
    --to_delete;
  }

  if (str_decimal.back() >=
      '5') {  // Увеличение последней цифры на 1 с последующими переносами
    str_decimal.pop_back();
    DecimalIncrementation(str_decimal);
  } else {  // Сначала проверка, затем удаление
    str_decimal.pop_back();
  }
  return (IsNegative() ? "-" : "") + str_decimal;
}

bool Rational::IsNegative() const { return numerator_.IsNegative(); }

Rational::operator double() const { return std::stod(asDecimal(100)); }
