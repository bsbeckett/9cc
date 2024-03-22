#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s || exit
  gcc -static -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# Integer literals
assert 0 0
assert 42 42
# Add/Sub
assert 5 "20-10-5"
assert 8 "2 - 3 + 9"
assert 41 " 12 +34 - 5"
# Mul/Div/()
assert 47 "5+6*7"
assert 15 "5*(9-6)"
assert 4 "(3+5)/2"
# Unary add/sub
assert 40 "-10+50"
assert 10 "+5-10+15"
# Relationals
assert 1 "1 < 2"
assert 0 "2 < 1"
assert 1 "2 <= 2"
assert 0 "3 <= 2"
assert 1 "6 > 3"
assert 0 "21 > 82"
assert 1 "81 >= 81"
assert 0 "3 >= 4"
assert 1 " 77 == 77"
assert 0 "21 == 20"
assert 1 "3 != 4"
assert 0 "3 != 3"
echo OK
