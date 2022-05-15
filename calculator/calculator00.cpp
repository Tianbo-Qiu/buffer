#include "../lib/std_lib_facilities.h"

/**
 * A conventional way of reading stuff from input and store it
 * in a way that lets us look at it in convenient ways. 'tokenize'
 */
class Token {
public:
  char kind;
  double value;
  Token(char k) : kind{k}, value{0.0} {}
  Token(char k, double v) : kind{k}, value{v} {}
  Token(double v) : kind{'8'}, value{v} {}
};

/**
 * Grammar
 * Reading a stream of tokens according to a grammar is called parsing,
 * and a program that does that is often called a parser or a syntax analyzer.
 *
 * // a simple expression grammar:
 * Expression:
 *      Term
 *      Expression + Term    // addition
 *      Expression - Term    // subtraction
 * Term:
 *      Primary
 *      Term * Primary       // multiplication
 *      Term / Primary       // division
 *      Term % Primary       // remainder (modulo)
 * Primary:
 *      Number
 *      ( Expression )       // grouping
 *      + Primary
 *      - Primary
 * Number:
 *      floating-point-literal
 */

/**
 * A stream that produces a token when we ask for one using get() and where we
 * can put a token back into the stream using putback().
 */
class Token_stream {
public:
  Token get();           // get a token
  void putback(Token t); // put a token back
private:
  bool full{false}; // is there a token in the buffer
  Token buffer{0.0};
};

void Token_stream::putback(Token t) {
  if (full)
    error("putback() into a full buffer");
  buffer = t;
  full = true;
}

Token Token_stream::get() {
  if (full) {
    full = false;
    return buffer;
  }
  char ch;
  cin >> ch;
  switch (ch) {
  case ';': // for "print"
  case 'q': // for "quit"
  case '(':
  case ')':
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
    return Token{ch};
  case '.':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9': {
    cin.putback(ch);
    double val;
    cin >> val;
    return Token{val};
  }
  default:
    error("Bad token");
  }
}

Token_stream ts;     // provides get() and putback()
double expression(); // declaration so that primary() can call expression()

// deal with numbers and parentheses
double primary() {
  Token t = ts.get();
  switch (t.kind) {
  case '(': // handle ( expression )
  {
    double d = expression();
    t = ts.get();
    if (t.kind != ')')
      error("')' expected");
    return d;
  }
  case '8':
    return t.value;
  case '-':
    return -primary();
  case '+':
    return primary();
  default:
    error("primary expected");
  }
}

// deal with * and /
double term() {
  double left = primary();
  Token t = ts.get();
  while (true) {
    switch (t.kind) {
    case '*':
      left *= primary();
      t = ts.get();
      break;
    case '/': {
      double d = primary();
      if (d == 0)
        error("divide by zero");
      left /= d;
      t = ts.get();
      break;
    }
    case '%': {
      double d = primary();
      if (d == 0)
        error("%:divide by zero");
      left = fmod(left, d);
      t = ts.get();
      break;
    }
    default:
      ts.putback(t); // put t back into the token stream
      return left;
    }
  }
}

// deal with + and -
double expression() {
  double left = term();
  Token t = ts.get();
  while (true) {
    switch (t.kind) {
    case '+':
      left += term();
      t = ts.get();
      break;
    case '-':
      left -= term();
      t = ts.get();
      break;
    default:
      ts.putback(t); // put back into the token stream
      return left;
    }
  }
}

// main loop and deal with errors
int main() {
  try {
    while (cin) {
      cout << "> ";
      Token t = ts.get();
      while (t.kind == ';')
        t = ts.get(); // eat ';'
      if (t.kind == 'q') {
        keep_window_open();
        return 0;
      }
      ts.putback(t);
      cout << "= " << expression() << '\n';
    }
    keep_window_open();
  } catch (exception &e) {
    cerr << e.what() << '\n';
    keep_window_open("~~");
    return 1;
  } catch (...) {
    cerr << "exception \n";
    keep_window_open("~~");
    return 2;
  }
}
