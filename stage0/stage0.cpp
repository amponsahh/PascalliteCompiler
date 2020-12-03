/*
  Nana Amponsah
  Angela McRae
  stage0
  CS 4301
*/

#include <iostream>           // cout
#include <fstream>            // ifstream
#include <algorithm>          // find
#include <vector>             // vector
#include <string>             // string
#include <ctime>              // time
#include <iomanip>            // formatting
#include <ios>                // right justification
using namespace std;

/*
  Provided by Dr. Motl
*/
enum storeType {
  INTEGER,
  BOOLEAN,
  PROG_NAME
};
enum allocation {
  YES,
  NO
};
enum modes {
  VARIABLE,
  CONSTANT
};
const int MAX_SYMBOL_TABLE_SIZE = 256;
struct entry {  // define symbol table entry format
  string internalName;
  string externalName;
  storeType dataType;
  modes mode;
  string value;
  allocation alloc;
  int units;
};

vector <entry> symbolTable;
ifstream sourceFile;
ofstream listingFile, objectFile;
string token;
char charac;
const char END_OF_FILE = '$'; // arbitrary choice

int lineCount = 0;
int prevCh = '\n';
int errCount = 0;
int numInts = 0;
int numBools = 0;
int numProgs = 0;
bool hasEnded = false;
string programName = "";


/*
  Prototypes
*/
bool isKeyword(string);
bool isSpecialSymbol(char);
bool isNON_KEY_ID(string);
bool isLiteral(string);
bool isInteger(string);
bool isBoolean(string);
string NextToken();
void CreateListingHeader();
void Parser();
void CreateListingTrailer();
void PrintSymbolTable();
char NextChar();
void Prog();
void ProgStmt();
void Consts();
void Vars();
void VarStmts();
void BeginEndStmt();
void ConstStmts();
storeType WhichType(string);
string WhichValue(string);
string Ids();
void Insert(string, storeType, modes, string, allocation, int);
string GenInternalName(storeType);
string EStoreType(int);
string EMode(int);
string boolIntStringify(string);
string intBoolStringify(int);


int main(int argc, char ** argv) {
  try {
    // this program is the stage0 compiler for Pascallite. It will accept
    // input from argv[1], generating a listing to argv[2], and object code to
    // argv[3]
    if (argc != 4) {
      cout << "Invalid amount of arguments" << endl;
      throw "Invalid amount of arguments";
    }
    sourceFile.open(argv[1]);
    listingFile.open(argv[2]);
    objectFile.open(argv[3]);
    CreateListingHeader();
    Parser();
    PrintSymbolTable();
  } catch (const char * msg) {
    errCount++;
    listingFile << endl << "Error: " << "Line " << lineCount << ": " << msg << endl;
    CreateListingTrailer();
  }
  
  // Fall back
  if (errCount == 0) {
    CreateListingTrailer();
  }
  
  // As instructed, close the files
  sourceFile.close();
  listingFile.close();
  objectFile.close();
  return 0;
}

// return the respective store type based on a given index number
string EStoreType(int num) {
  switch (num)
  {
    case 0:
      return "INTEGER";
    case 1:
      return "BOOLEAN";
    case 2:
      return "PROG_NAME";
    default:
      return "";
  }
}

// return the respective enum mode based on a given index number
string EMode(int num) {
  switch (num)
  {
    case 0:
      return "VARIABLE";
    case 1:
      return "CONSTANT";
    default:
      return "";
  }
}


// return an int string output based upon the boolean input
// i like switches, i think they're cool especially since they have a catch all default
// but alas, i cannot use switch cases on string in c++ and honestly, I'm not happy 
// https://stackoverflow.com/a/650218
string boolIntStringify(string boolWord) { 
  if (boolWord == "true") {
    return "1";
  }
  if (boolWord == "false") {
    return "0";
  }
  return boolWord.substr(0, 15);
}

// return a string output based upon the int input
string intBoolStringify(int intWord) {
  switch (intWord)
  {
    case 0:
      return "YES";
    case 1:
      return "NO";
    default:
      return "";
  }
}

// return true if string is a bool
bool isBoolean(string word) {
  return ((word == "true") || 
          (word == "false"));
}

// return true if string is a boolean or an integer
bool isLiteral(string word) {
  return ((isBoolean(word)) || 
          (isInteger(word)));
}
// return true if string is an integer
bool isInteger(string word) {
  if (isdigit(word[0]) || (word.size() > 1 && (word[0] == '+' || word[0] == '-'))) {
    for (string::size_type i { 1 }; i < word.size(); i++) {
      if (!isdigit(word[i])) {
        return false;
      }
    }
    return true;
  }
  return false;
}

// return true if string is a keyword
bool isKeyword(string name) {
  return ((name == "program") || 
          (name == "const") || 
          (name == "var") || 
          (name == "integer") || 
          (name == "boolean") || 
          (name == "begin") || 
          (name == "end") || 
          (name == "not") || 
          (isBoolean(name)));
}

// return true if string is a special symbol
bool isSpecialSymbol(char charac) {
  return ((charac == '=') || 
          (charac == ':') || 
          (charac == ',') || 
          (charac == ';') || 
          (charac == '.') || 
          (charac == '+') || 
          (charac == '-'));
}

// return true if key is not a keyword, special symbol, or an integer
bool isNON_KEY_ID(string word) {  
  return (((isKeyword(word)) || 
          (isSpecialSymbol(word.at(0))) || 
          (isInteger(word))) ? 
          (false) : (true));
}

// create the header for the listing file
void CreateListingHeader() {
  string names = "Amponsah Nana, McRae Angela";
  time_t now = time(NULL);
  string DATE = ctime(&now);
  listingFile << "STAGE0:" << right << setw(29) << names << right << setw(32) << DATE << endl;
  listingFile << "LINE NO." << right << setw(30) << "SOURCE STATEMENT" << endl << endl;
  
}

// initialize parser 
void Parser() {
  NextChar();
  if (NextToken() != "program"){
    throw "keyword program expected";
  }
  Prog();
}

// create footer for the listing file
void CreateListingTrailer() {
  listingFile << endl << "COMPILATION TERMINATED" << right << setw(7) << errCount << " ERRORS ENCOUNTERED\n";
}

// print the symbol table into the object file
void PrintSymbolTable() {
  string names = "Amponsah Nana, McRae Angela";
  time_t now = time(NULL);
  string DATE = ctime(&now);
  objectFile << "STAGE0:" << right << setw(29) << names << right << setw(32) << DATE << endl;
  objectFile << "Symbol Table" << endl << endl;
  
  for (uint i = 0; i < symbolTable.size(); i++) {
    objectFile << left << setw(15) << symbolTable.at(i).externalName;
    objectFile << setw(2) << "";
    objectFile << left << setw(4) << symbolTable.at(i).internalName;
    objectFile << setw(2) << "";
    objectFile << right << setw(9) << EStoreType(symbolTable.at(i).dataType);
    objectFile << setw(2) << "";
    objectFile << right << setw(8) << EMode(symbolTable.at(i).mode);
    objectFile << setw(2) << "";
    objectFile << right << setw(15) << boolIntStringify(symbolTable.at(i).value);
    objectFile << setw(2) << "";
    objectFile << right << setw(3) << intBoolStringify(symbolTable.at(i).alloc);
    objectFile << setw(2) << "";
    objectFile << setw(1) << symbolTable.at(i).units;
    objectFile << endl;
  }
}

// Prog() - production 1
// token should be "program"
void Prog() {
  if (token != "program") {
    throw "keyword \"program\" expected";
  }
  ProgStmt();
  if (token == "const") Consts();
  if (token == "var") Vars();
  if (token != "begin") {
    throw "keyword \"begin\" expected";
  }
  BeginEndStmt(); 
  if (token.length() == 1 && (token[0] != END_OF_FILE)) {
    throw "no text may follow \"end\"";
  }
}

// ProgStmt() - production 2
// token should be "program"
void ProgStmt() {
  string x;
  if (token != "program") {
    throw "keyword \"program\" expected";
  }
  x = NextToken();
  if (!isNON_KEY_ID(token)) {
    throw "program name expected";
  }
  else {
    programName = token;
  }
  if (NextToken() != ";") {
    throw "semicolon expected";
  }
  NextToken();
  Insert(x, PROG_NAME, CONSTANT, x, NO, 0);
}

// Consts() - production 3
// token should be "const"
void Consts() {
  if (token != "const") {
    throw "keyword \"const\" expected";
  }
  if (!isNON_KEY_ID(NextToken())) {
    throw "non-keyword identifier must follow \"const\"";
  }
  ConstStmts();
}

// Vars() - production 4
// token should be "var"
void Vars() {
  if (token != "var") {
    throw "keyword \"var\" expected";
  }
  if (!isNON_KEY_ID(NextToken())) {
    throw "non-keyword identifier must follow \"var\"";
  }
  VarStmts();
}

// BeginEndStmt() - production 5
// token should be "begin"
void BeginEndStmt() {
  if (token != "begin") {
    throw "keyword \"begin\" expected";
  }
  if (NextToken() != "end") {
    throw "keyword \"end\" expected";
  }
  if (NextToken() != ".") {
    throw "\".\" expected";
  } else {
    hasEnded = true;
  }
  NextToken();
}

// ConstStmts() - production 6
// token should be NON_KEY_ID
void ConstStmts() {
  string x, y;
  if (!isNON_KEY_ID(token)) {
    throw "non-keyword identifier expected";
  }
  x = token;
  if (NextToken() != "=") {
    throw "\"=\" expected";
  }
  y = NextToken();
  if (y != "+" && y != "-" && y != "not" && !isNON_KEY_ID(y) && y != "true" && y != "false" && !isInteger(y)) {
    throw "token to right of \"=\" illegal";
  }
  if (y == "+" || y == "-") {
    string next = NextToken();
    if (!isInteger(next)) {
      throw "integer expected after sign";
    }
    y += token;
  }
  if (y == "not") {
    if (NextToken() != "true" && token != "false") {
      throw "boolean expected after not";
    }
    if (token == "true") {
      y = "false";
    }
    else {
      y = "true";
    }
  }
  if (NextToken() != ";") {
    throw "semicolon expected";
  }
  Insert(x, WhichType(y), CONSTANT, WhichValue(y), YES, 1);
  if (NextToken() != "begin" && token != "var" && !isNON_KEY_ID(token)) {
    throw "non-keyword identifier, \"begin\", or \"var\" expected";
  }
  if (isNON_KEY_ID(token)) {
    ConstStmts();
  }
}

// VarStmts() - production 7
// token should be NON_KEY_ID
void VarStmts() {
  string x, y;
  if (!isNON_KEY_ID(token)) {
    throw "non-keyword identifier expected token";
  }
  x = Ids();
  if (token != ":") {
    throw "\":\" expected";
  }
  if (NextToken() != "integer" && token != "boolean") {
    throw "illegal type follows \":\"";
  }
  y = token;
  if (NextToken() != ";") {
    throw "semicolon expected";
  }
  if (y == "integer") {
    Insert(x, INTEGER, VARIABLE, "", YES, 1);
  } else if (y == "boolean") {
    Insert(x, BOOLEAN, VARIABLE, "", YES, 1);
  }
  if (NextToken() != "begin" && !isNON_KEY_ID(token)) {
    throw "non-keyword identifier or \"begin\" expected";
  }
  if (isNON_KEY_ID(token)) {
  VarStmts();
  }
}

// Ids() - production 8
// token should be NON_KEY_ID
string Ids() {
  string temp, tempString;
  if (!isNON_KEY_ID(token)) {
    throw "non-keyword identifier expected";
  }
  tempString = token;
  temp = token;
  if (NextToken() == ",") {
    if (!isNON_KEY_ID(NextToken())) {
      throw "non-keyword identifier expected";
    }
    tempString = temp + "," + Ids();
  }
  return tempString;
}

// Action Routines
// Insert() creates entries in the symbol table.
void Insert(string externalName, storeType inType, modes inMode, string inValue, allocation inAlloc, int inUnits) {
  vector <string> nameVect;
  nameVect.push_back("");
  for (uint x = 0, word = 0; x < externalName.length(); x++) {
    if (externalName.at(x) == ',') {
      word++;
      nameVect.push_back("");
    } else {
      nameVect.at(word) += externalName.at(x);
    }
  }
  string name;
  for (uint y = 0; y < nameVect.size(); y++) {
    name = nameVect.at(y);
    name = name.substr(0, 15);
    for (uint x = 0; x < symbolTable.size(); x++) {
      if (symbolTable.at(x).externalName == name) {
        throw "multiple name definition";
      }
    }
    if (isKeyword(name)) {
      throw "illegal use of keyword";
    }
    entry e;
    e.externalName = name;
    e.dataType = inType;
    e.mode = inMode;
    if (inValue == programName && numProgs != 0) {
      throw "data type of token on the right-hand side must be INTEGER or BOOLEAN";
    }
    e.value = inValue;
    e.alloc = inAlloc;
    e.units = inUnits;
    if (symbolTable.size() != MAX_SYMBOL_TABLE_SIZE) {
      if (isupper(name.at(0))) {
        e.internalName = name;
        symbolTable.emplace_back(e);
      } else {
        e.internalName = GenInternalName(inType);
        symbolTable.emplace_back(e);
      }
    } else {
      throw "Symbol Table overflow, cannot allocate more than 256 entries";
    }
  }
}

// GenInternalName() returns a unique internal name each time it is called, a name that is known to be
// a valid symbolic name.
string GenInternalName(storeType inType) { 
  switch (inType){
    case BOOLEAN:
      numBools++;
      return "B" + to_string(numBools - 1);
    case INTEGER:
      numInts++;
      return "I" + to_string(numInts - 1);
    case PROG_NAME:
      numProgs++;
      return "P" + to_string(numProgs - 1);
    default:
      return "";
    
  }
}

// which data type a name has
storeType WhichType(string name) {
  if (isLiteral(name)) {
    if (isBoolean(name)) {
      return BOOLEAN;
    }
    return INTEGER;
  }
  for (uint x = 0; x < symbolTable.size(); x++) {
    if (symbolTable.at(x).externalName == name) {
      return symbolTable.at(x).dataType;
    }
  }
  throw "reference to undefined constant";
}


// which value a name has
string WhichValue(string name) {
  if (isLiteral(name)) {
    return name;
  }
  for (uint x = 0; x < symbolTable.size(); x++) {
    if (symbolTable.at(x).externalName == name) {
      if (symbolTable.at(x).mode == CONSTANT) {
        return symbolTable.at(x).value;
      }
    }
  }
  throw "reference to undefined constant";
}


// return next token or end of file marker
string NextToken() {
  token = "";
  while (token == "") {
    if (charac == '{') {
      while (NextChar() != END_OF_FILE && charac != '}');
      if (charac == END_OF_FILE) {
        throw "unexpected end of file";
      } else {
        NextChar();
      }
    } else if (charac == '}') {
      throw "'}' cannot begin token";
    } else if (isspace(charac)) {
      NextChar();
    } else if (hasEnded && charac != END_OF_FILE) {
      throw "Non-white-space character after end statement";
    } else if (isSpecialSymbol(charac)) {
      token += charac;
      NextChar();
    } else if (islower(charac)) {
      token += charac;
      while (islower(NextChar()) || isdigit(charac) || charac == '_') {
        if (charac == '_' && token.back() == '_') {
          throw ("Invalid non key id because of double__underscore");
        }
        token += charac;
      }
      if (token.at(token.length() - 1) == '_') {
        throw "'_' cannot end token";
      }
    } else if (isdigit(charac)) {
        token += charac;
        while (isdigit(NextChar())) {
          token += charac;
        }
    } else if (charac == END_OF_FILE) {
        token += charac;
    } else {
        throw "illegal symbol";
    }
  }
  return token;
}

// return the next character or end of file marker
char NextChar() {
  sourceFile.get(charac);
  if (sourceFile.eof()) {
    charac = END_OF_FILE;
    return charac;
  }
  if (prevCh == '\n') {
    lineCount++;
    listingFile << setw(5) << lineCount << "|";
  }
  prevCh = charac;
  listingFile << charac;
  return charac;
}