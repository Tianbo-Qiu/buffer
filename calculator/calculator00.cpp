/*
 * Simple calculator
 *
 * Revision history:
 *  Revised by ...
 *
 * This program implements a basic expression calculator.
 * Input from cin; output form cout.
 * The grammar for input is:
 * Statement:
 *         Statement
 *         Print
 *         Quit
 * Print:
 *         ;
 * Quit:
 *         q
 * Statement:
 *         Declaration
 *         Expression
 * Declaration:
 *         "let" Name "=" Expression
 * Name:
 *         character
 *         Name + character
 *         Name + digit
 * Expression:
 *         Term
 *         Expression + Term
 *         Expression - Term
 * Term:
 *         Primary
 *         Term * Primary
 *         Term / Primary
 *         Term % Primary
 * Primary:
 *         Number
 *         ( Expression )
 *         -Primary
 *         +Primary
 *         Variable
 * Number:
 *         floating-point-literal
 * Varable:
 *         { Name, Number } pair
 *
 * Input comes from cin through the Token_stream called ts.
 */
#include "../lib/std_lib_facilities.h"

const char number = '8'; // t.kind == number means that t is a number Token
const char quit = 'q';   // t.kind == quit means that t is a quit Token
const char print = ';';  // t.kind == print means that t is a print Token
const string prompt = "> ";
const string result = "= ";
const char name = 'a';        // name token
const char let = 'L';         // declaration token
const string declkey = "let"; // declaration key

/**
 * A conventional way of reading stuff from input and store it
 * in a way that lets us look at it in convenient ways. 'tokenize'
 */
class Token {
public:
  char kind;
  double value;
  string name;
  Token(char k) : kind{k}, value{0.0} {}
  Token(char k, double v) : kind{k}, value{v} {}
  Token(double v) : kind{number}, value{v} {}
  Token(char ch, string n) : kind{ch}, name{n} {}
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
  void ignore(char c);   // discard characters up to and including a c
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

// read characters from cin and compose a Token
Token Token_stream::get() {
  // check if we already have a Token ready
  if (full) {
    full = false;
    return buffer;
  }
  char ch;
  cin >> ch; // note that >> skips whitespace (space, newline, tab, etc.)
  switch (ch) {
  case print: // for "print"
  case quit:  // for "quit"
  case '=':   // for declaration and assignment
  case '(':
  case ')':
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
    return Token{ch}; // let each character represent itself
  case '.':           // a floating-point-literal can start with a dot
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
    if (isalpha(ch)) {
      string s;
      s += ch;
      while (cin.get(ch) && (isalpha(ch) || isdigit(ch)))
        s += ch;
      cin.putback(ch);
      if (s == declkey)
        return Token{let}; // decalration keyword
      return Token{name, s};
    }
    error("Bad token");
  }
}

void Token_stream::ignore(char c) {
  // first look in buffer:
  if (full && c == buffer.kind) {
    full = false;
    return;
  }
  full = false;
  // now search for input:
  char ch = 0;
  while (cin >> ch) {
    if (ch == c)
      return;
  }
}

Token_stream ts; // provides get() and putback()

class Variable {
public:
  string name;
  double value;
};

vector<Variable> var_table;

// return the value of the variable named s
double get_value(string s) {
  for (const Variable &v : var_table)
    if (v.name == s)
      return v.value;
  error("get: undefined variable ", s);
}

// set the Variable named s to d
void set_value(string s, double d) {
  for (Variable &v : var_table) {
    if (v.name == s) {
      v.value = d;
      return;
    }
    error("set: undefined variable ", s);
  }
}

// is var declared in var_table
bool is_declared(string var) {
  for (const Variable &v : var_table) {
    if (v.name == var)
      return true;
  }
  return false;
}

// add { var, val } to var_table
double define_name(string var, double val) {
  if (is_declared(var))
    error(var, " declared twice");
  var_table.push_back(Variable{var, val});
  return val;
}

double expression(); // declaration so that primary() can call expression()

// assume we have seen "let"
// handle: name = expression
// declare a variable called "name" with the initial value "expression"
double declaration() {
  Token t = ts.get();
  if (t.kind != name)
    error("name expected in declaration");
  string var_name = t.name;
  Token t2 = ts.get();
  if (t2.kind != '=')
    error("= missing in declartion of ", var_name);
  double d = expression();
  define_name(var_name, d);
  return d;
}

double statement() {
  Token t = ts.get();
  switch (t.kind) {
  case let:
    return declaration();
  default:
    ts.putback(t);
    return expression();
  }
}

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
    if (t.kind == name) {
      return get_value(t.name);
    }
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

void clean_up_mess() { ts.ignore(print); }

// expression evaluation loop
void calculate() {
  while (cin)
    try {
      {
        cout << prompt;
        Token t = ts.get();
        while (t.kind == print)
          t = ts.get(); // eat ';'
        if (t.kind == quit) {
          return;
        }
        ts.putback(t);
        cout << result << statement() << '\n';
      }
    } catch (exception &e) {
      cerr << e.what() << '\n';
      clean_up_mess();
    }
}

// main loop and deal with errors
int main() {
  try {
    define_name("pi", 3.1415926535);
    define_name("e", 2.7182818284);
    calculate();
    keep_window_open();
    return 0;
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
