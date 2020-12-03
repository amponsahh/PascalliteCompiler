/*
  Nana Amponsah
  Angela McRae
  stage1
  CS 4301
*/

#include <iostream>     // cout
#include <fstream>      // ifstream
#include <algorithm>    // find
#include <vector>       // vector
#include <string>       // string
#include <ctime>        // time
#include <iomanip>      // formatting
#include <ios>          // right justification do not depend on whether the stream is an input or an output stream
#include <stack>        // stack
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
struct entry { // define symbol table entry format
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
int numTemps = 0;
int maxNumTemps = 0;
int numLabels = 0;
bool hasEnded = false;
string programName = "";
stack<string> operatorStk, operandStk;
string aReg = "";

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
void rammVarsPrint(string);
char NextChar();
char NextChar(bool);
void Prog();
void ProgStmt();
void Consts();
void Vars();
void VarStmts();
void BeginEndStmt();
void ConstStmts();
storeType WhichType(string);
string WhichValue(string);
modes WhichMode(string);
string Ids();
void Insert(string, storeType, modes, string, allocation, int);
string GenInternalName(storeType);
void allocTemp(int i);
void objFileWrite(string, string, string, string, string, string);
string EStoreType(int);
string EMode(int);
string boolIntStringify(string);
string intBoolStringify(int);

void ExecStmts();
void ExecStmt();
void AssignStmt();
void ReadStmt();
void WriteStmt();
void Express();
string PopOperator();
string PopOperand();
void PushOperand(string);
void PushOperator(string);
void Code(string, string = "", string = "");
void Term();
void Expresses();
void Factor();
void Terms();
void Part();
void Factors();
bool isRelOp(string);
bool isAddLevelOp(string);
bool isMultLevelOp(string);
void EmitAdditionCode(string, string, storeType);
void EmitMultiplicationCode(string, string, storeType);
void EmitSubtractionCode(string, string, storeType);
void EmitNegateCode(string, storeType);
void EmitNotCode(string, storeType);
void EmitDivCode(string, string, storeType);
void EmitModCode(string, string, storeType);
void EmitOrCode(string, string, storeType);
void EmitAndCode(string, string, storeType);
void EmitLessThanCode(string, string, storeType);
void EmitGreaterThanCode(string, string, storeType);
void EmitLessThanEqualCode(string, string, storeType);
void EmitGreaterThanEqualCode(string, string, storeType);
void EmitNotEqualsCode(string, string, storeType);
void EmitEqualsCode(string, string, storeType);
void EmitAssignCode(string, string);
string externalToInternal(string);
string internalToExternal(string);
void FreeTemp();
string GetTemp();
bool isExtNameSymTable(string);
int getIntSymTableIndex(string);


int main(int argc, char ** argv) {
  try {
    /* this program is the stage1 compiler for Pascallite. It will accept
       input from argv[1], generating a listing to argv[2], and object code to
       argv[3]*/
    if (argc != 4) {
      throw "Invalid amount of arguments.";
    }
    sourceFile.open(argv[1]);
    listingFile.open(argv[2]);
    objectFile.open(argv[3]);
    CreateListingHeader();
    Parser();
    PrintSymbolTable();
  } catch (const char * msg) {
    PrintSymbolTable();
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

// optimized
// https://stackoverflow.com/a/650218
string boolIntStringify(string boolWord) { 
  return ((boolWord == "true") ? 
          ("1") : ((boolWord == "false") ? 
          ("0") : (boolWord.substr(0, 15))));
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
          (isBoolean(name)) || 
          (name == "mod") || 
          (name == "div") || 
          (name == "and") || 
          (name == "or") || 
          (name == "read") || 
          (name == "write"));
}        

// return true if string is a special symbol
bool isSpecialSymbol(char charac) {
  return ((charac == '=') || 
          (charac == ':') || 
          (charac == ',') || 
          (charac == ';') || 
          (charac == '.') || 
          (charac == '+') || 
          (charac == '-') || 
          (charac == '*') || 
          (charac == '<') || 
          (charac == '>') || 
          (charac == '(') || 
          (charac == ')'));
}

// returns true if key <> a keyword, special symbol, or an integer
bool isNON_KEY_ID(string word) {  
  return (((isKeyword(word)) || 
          (isSpecialSymbol(word.at(0))) || 
          (isInteger(word))) ? 
          (false) : (true));
}

bool isRelOp(string op) {
  return ((op == "=") || 
          (op == "<>") || 
          (op == "<=") || 
          (op == ">=") || 
          (op == "<") || 
          (op == ">"));
}
bool isAddLevelOp(string op) {
  return ((op == "+") || 
          (op == "-") || 
          (op == "or"));
}

bool isMultLevelOp(string op) {
  return ((op == "*") || 
          (op == "div") || 
          (op == "mod") || 
          (op == "and"));
}

// returns true if string is a boolean or an integer
bool isLiteral(string word) {
  return ((isBoolean(word)) || 
          (isInteger(word)));
}

// returns true if string is an integer, false if otherwise
bool isInteger(string word) {
  if (isdigit(word[0]) || (word.size() > 1 && (word[0] == '+' || word[0] == '-'))) {
    for (string::size_type i { 1 }; i < word.size(); ++i) {
      if (!isdigit(word[i])) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool isBoolean(string word) {
  return ((word == "true") || 
          (word == "false"));
}

// is it an external name in the Symbol table?
bool isExtNameSymTable(string name) {
  for (uint x = 0; x < symbolTable.size(); x++) {
    if (symbolTable.at(x).externalName == name) {
      return true;
    }
  }
  return false;
}

string externalToInternal(string name) {
  name = name.substr(0, 15);
  for (uint x = 0; x < symbolTable.size(); x++) {
    if (symbolTable.at(x).externalName == name) {
      return symbolTable.at(x).internalName;
    }
  }
  throw "reference to undefined variable.";
}

string internalToExternal(string name) {
  name = name.substr(0, 15);
  for (uint x = 0; x < symbolTable.size(); x++) {
    if (symbolTable.at(x).internalName == name) {
      return symbolTable.at(x).externalName;
    }
  }
  throw "reference to undefined variable.";
}

int getIntSymTableIndex(string name) {
  name = name.substr(0, 15);
  for (uint x = 0; x < symbolTable.size(); x++) {
    if (symbolTable.at(x).internalName == name) {
      return x;
    }
  }
  throw "reference to undefined variable.";
}

void allocTemp(int i) {
  symbolTable.at(i).alloc = YES;
  symbolTable.at(i).units = 1;
}

void FreeTemp() {
  numTemps--;
  if (numTemps < -1)
    throw "compiler error, numTemps should be >= –1";
}

string GetTemp() {
  string temp;
  numTemps++;
  temp = "T" + to_string(numTemps - 1);
  if (numTemps > maxNumTemps) {
    Insert(temp, INTEGER, VARIABLE, "", NO, 0);
    maxNumTemps++;
  }
  return temp;
}

/* TODO work on alignment
DONE - Nana*/
void objFileWrite(string location, string opCode, string address, string sign, string number, string remarks) {
  objectFile << left << setfill(' ') << setw(4) 
             << location << setw(2) << "" << setw(3) 
             << opCode << setw(1) << "" << setw(4) 
             << address << setw(1) 
             << sign << setw(1) 
             << number << setw(3) << "" 
             << remarks << endl;
}

// create the header for the listing file
void CreateListingHeader() {
  string names = "Amponsah Nana, McRae Angela";
  time_t now = time(NULL);
  string DATE = ctime(&now);
  listingFile << "STAGE1:" << right << setw(29) << names << right << setw(32) << DATE << endl;
  listingFile << "LINE NO." << right << setw(30) << "SOURCE STATEMENT" << endl << endl;
  
}

// initiates the parser
void Parser() {
  NextChar();
  if (NextToken() != "program")
    throw "keyword program expected";
  Prog();
}

// creates the footer for the listing file
void CreateListingTrailer() {
  listingFile << endl << "COMPILATION TERMINATED" << right << setw(7) << errCount << " ERRORS ENCOUNTERED\n";
}

/* TODO check alignment DONE - Nana*/
// prints the symbol table into the object file
void PrintSymbolTable() {
  string names = "Amponsah Nana, McRae Angela";
  time_t now = time(NULL);
  string DATE = ctime( & now);
  cout << "STAGE1:" << right << setw(29) << names << right << setw(32) << DATE << endl;
  cout << "Symbol Table" << endl << endl;
  for (uint i = 0; i < symbolTable.size(); i++) {
    cout << left << setw(15) << symbolTable.at(i).externalName;
    cout << setw(2) << "";
    cout << left << setw(4) << symbolTable.at(i).internalName;
    cout << setw(2) << "";
    cout << right << setw(9) << EStoreType(symbolTable.at(i).dataType);
    cout << setw(2) << "";
    cout << right << setw(8) << EMode(symbolTable.at(i).mode);
    cout << setw(2) << "";
    cout << right << setw(15) << boolIntStringify(symbolTable.at(i).value);
    cout << setw(2) << "";
    cout << right << setw(3) << intBoolStringify(symbolTable.at(i).alloc);
    cout << setw(2) << "";
    cout << setw(1) << symbolTable.at(i).units;
    cout << endl;
  }
}

// TODO work on alignment 
void rammVarsPrint(string operand1) {
  objectFile << setw(6) << "" << left << setw(13)<< "HLT" << endl;
  for (uint i = 0; i < symbolTable.size(); ++i) {
    if (symbolTable.at(i).alloc == YES && symbolTable.at(i).units == 1) {
      if (symbolTable.at(i).mode == CONSTANT) {
        bool isNeg = symbolTable.at(i).value.at(0) == '-';
        string val = symbolTable[i].value;
        objectFile << left << setw(4) << symbolTable.at(i).internalName << setw(2) << "";
        objectFile << setw(3) << "DEC " << right;
        string eName = symbolTable.at(i).externalName;
        if (eName == "true" || eName == "false") {
          for (uint j = 0; j < eName.size(); ++j) {
            eName[j] = toupper(eName[j]);
          }
        }
        if (symbolTable[i].dataType == BOOLEAN) {
          val = boolIntStringify(symbolTable.at(i).value);
        }
        // just leave the blank spaces hard coded the remarks exceed 17 characters 
        objectFile << (isNeg ? "-" : "") << setw(isNeg ? 3 : 4) << setfill('0') << (isNeg ? val.substr(1, 3) : val) << setfill(' ') << "     " << eName << endl;
      }
      if (symbolTable.at(i).mode == VARIABLE) {
        objectFile << left << setw(4) << symbolTable.at(i).internalName << setw(2) << "";
        objectFile << setw(3) << "BSS " << right;
        objectFile << setw(4) << setfill('0') << "0001" << setfill(' ') << "     " << symbolTable.at(i).externalName << endl;
      }
    }
  }
  objFileWrite("", "END", "STRT", "", "", "");
}

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

// token should be "program"
void ProgStmt() {
  string x;
  if (token != "program")
    throw "keyword \"program\" expected";
  x = NextToken();
  if (!isNON_KEY_ID(token))
    throw "program name expected";
  else
    programName = token;
  Code("program");
  if (NextToken() != ";")
    throw "\";\" expected";
  NextToken();
  Insert(x, PROG_NAME, CONSTANT, x, NO, 0);
}

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

// token should be "var"
void Vars() {
  if (token != "var")
    throw "keyword \"var\" expected";
  if (!isNON_KEY_ID(NextToken()))
    throw "non-keyword identifier must follow \"var\"";
  VarStmts();
}

// token should be "begin"
void BeginEndStmt() {
  if (token != "begin")
    throw "keyword \"begin\" expected";
  ExecStmts();
  if (token != "end")
    throw "keyword \"end\" expected";
  if (NextToken() != ".") {
    throw "\".\" expected";
  } else {
    hasEnded = true;
  }
  Code("end");
  NextToken();
}

// token should be NON_KEY_ID
void ConstStmts() {
  string x, y;
  if (!isNON_KEY_ID(token))
    throw "non-keyword identifier expected";
  x = token;
  if (NextToken() != "=")
    throw "\"=\" expected";
  y = NextToken();
  if (y != "+" && y != "-" && y != "not" && !isNON_KEY_ID(y) && y != "true" && y != "false" && !isInteger(y))
    throw "token to right of \"=\" illegal";
  if (y == "+" || y == "-") {
    string next = NextToken();
    if (!isInteger(next))
      throw "integer expected after sign";
    y += token;
  }
  if (y == "not") {
    if (NextToken() != "true" && token != "false" && (WhichType(token) != BOOLEAN))
      throw "boolean expected after \"not\"";
    if (token == "true")
      y = "false";
    else if (token == "false")
      y = "true";
    else
      y = (WhichValue(token) == "true" ? "false" : "true");
  }
  if (NextToken() != ";")
    throw "\";\" expected";
  Insert(x, WhichType(y), CONSTANT, WhichValue(y), YES, 1);
  if (NextToken() != "begin" && token != "var" && !isNON_KEY_ID(token))
    throw "non-keyword identifier,\"begin\", or \"var\" expected";
  if (isNON_KEY_ID(token))
    ConstStmts();
}

//token should be NON_KEY_ID
void VarStmts() {
  string x, y;
  if (!isNON_KEY_ID(token))
    throw "non-keyword identifier expected token";
  x = Ids();
  if (token != ":")
    throw "\":\" expected";
  if (NextToken() != "integer" && token != "boolean")
    throw "illegal type follows \":\"";
  y = token;
  if (NextToken() != ";") {
    throw "\";\" expected";
  }
  if (y == "integer") {
    Insert(x, INTEGER, VARIABLE, "", YES, 1);
  } else if (y == "boolean") {
    Insert(x, BOOLEAN, VARIABLE, "", YES, 1);
  }
  if (NextToken() != "begin" && !isNON_KEY_ID(token)) {
    throw "non-keyword identifier or \"begin\" expected";
  }
  if (isNON_KEY_ID(token))
    VarStmts();
}

// token should be NON_KEY_ID
string Ids() {
  string temp, tempString;
  if (!isNON_KEY_ID(token))
    throw "non-keyword identifier expected";
  tempString = token;
  temp = token;
  if (NextToken() == ",") {
    if (!isNON_KEY_ID(NextToken()))
      throw "non-keyword identifier expected";
    tempString = temp + "," + Ids();
  }
  return tempString;
}

// symbol table entry foreach identifier in list of external names
void Insert(string externalName, storeType inType, modes inMode, string inValue, allocation inAlloc, int inUnits) {
  vector < string > nameVect;
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
    if (isExtNameSymTable(name)) {
      throw "multiple name definition";
    }
    if (isKeyword(name) && !isBoolean(name)) {
      throw "illegal use of keyword";
    }
    entry e;
    e.externalName = name;
    e.dataType = inType;
    e.mode = inMode;
    if ((inValue == programName) && (numProgs != 0))
      throw "data type of token on the right-hand side must be INTEGER or BOOLEAN";
    e.value = inValue;
    e.alloc = inAlloc;
    e.units = inUnits;
    
    if (symbolTable.size() != MAX_SYMBOL_TABLE_SIZE) {
      if (isupper(name.at(0))) {
        e.internalName = name;
        symbolTable.emplace_back(e);
      } else {
        if (e.externalName == "true") {
          e.internalName = "TRUE";
        } else if (e.externalName == "false") {
          e.internalName = "FALS";
        } else {
          e.internalName = GenInternalName(inType);
        }
        symbolTable.emplace_back(e);
      }
    } else {
    throw "symbol Table overflow, cannot allocate more than 256 entries";
    }
  }
}

// GenInternalName() returns a unique internal name each time it is called
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
  bool isNeg = name[0] == '-';
  bool isPlus = name[0] == '+';
  if (isNeg) {
    name = name.substr(1, name.size());
  }
  if (isPlus) {
    name = name.substr(1, name.size());
  }
  if (isExtNameSymTable(name)) {
    name = externalToInternal(name);
    return symbolTable.at(getIntSymTableIndex(name)).dataType;
  }
  throw "type reference to undefined constant";
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
  
  // test case hot fix
  bool isNeg = name[0] == '-';
  bool isPlus = name[0] == '+';
  if (isNeg) {
    name = name.substr(1, name.size());
  }
  if (isPlus) {
    name = name.substr(1, name.size());
  }
  if (isExtNameSymTable(name)) {
    name = externalToInternal(name);
    string val = (isNeg ? "-" : "") + symbolTable.at(getIntSymTableIndex(name)).value;
    
    // test case hot fix 
    if (val[0] == '-' && val[1] == '-') {
      val = val.substr(2, val.size());
    }
    return val;
  }
  throw "value reference to undefined constant";
}

modes WhichMode(string name) {
  for (uint x = 0; x < symbolTable.size(); x++) {
    if (symbolTable.at(x).externalName == name) {
      return symbolTable.at(x).mode;
    }
  }
  throw "mode reference to undefined constant";
}

// returns the next token or end of file marker
string NextToken() {
  token = "";
  while (token == "") {
    if (charac == '{') {
      while (NextChar() != END_OF_FILE && charac != '}');
      if (charac == END_OF_FILE) {
        throw "unexpected end of file";
      } else
        NextChar();
    } else if (charac == '}') {
      throw "\"}\" cannot begin token";
    } else if (isspace(charac)) {
      NextChar();
    } else if (hasEnded && charac != END_OF_FILE) {
      throw "non-white-space character after end statement.";
      
      
    } else if (charac == ':') { //:=
      token += charac;
      if (NextChar() == '=') {
        token += charac;
        NextChar();
      }
    } else if (charac == '*' || charac == '(' || charac == ')') {
      token += charac;
      NextChar();
    } else if (charac == '<') { // <>
      token += charac;
      if (NextChar() == '>' || charac == '=') {
        token += charac;
        NextChar();
      }
    } else if (charac == '>') { //>=
      token += charac;
      if (NextChar() == '=') {
        token += charac;
        NextChar();
      }
      
    } else if (isSpecialSymbol(charac)) {
      token += charac;
      NextChar();
    } else if (islower(charac)) {
      token += charac;
      while (islower(NextChar()) || isdigit(charac) || charac == '_') {
        if (charac == '_' && token.back() == '_')
          throw ("invalid non key id due to underscore placement");
        token += charac;
      }
      if (token.at(token.length() - 1) == '_')
        throw "\"_\" cannot end token";
    } else if (isdigit(charac)) {
        token += charac;
        while (isdigit(NextChar()))
          token += charac;
    } else if (charac == END_OF_FILE) {
        token += charac;
    } else {
        listingFile << token << endl;
        throw "illegal symbol";
    }
  }
  return token;
}

// returns the next character or end of file marker
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

void ExecStmts() {
  while (isNON_KEY_ID(NextToken()) || token == "read" || token == "write") {
    ExecStmt();
  }
  if (token != "end")
    throw "NON_KEY_ID, read, write, or end expected";
}

void ExecStmt() {
  if (isNON_KEY_ID(token))
    AssignStmt();
  else if (token == "read")
    ReadStmt();
  else if (token == "write")
    WriteStmt();
}

void AssignStmt () {
  if (!isNON_KEY_ID(token))
    throw "NON_KEY_ID expected";
  if (WhichMode(token) == CONSTANT)
    throw "Identifier to the left of ':=' must be a variable";
  PushOperand(token);
  if (NextToken() != ":=")
    throw "\":=\" expected";
  PushOperator(":=");
  Express();
  if (token != ";") {
    throw "\";\" expected";
  }
  string x = PopOperator();
  string y = PopOperand();
  string z = PopOperand();
  if (symbolTable.at(getIntSymTableIndex(y)).dataType != symbolTable.at(getIntSymTableIndex(z)).dataType)
    throw "left and right hand side type mismatch";
  Code(x, y, z);
}

void ReadStmt() {
  if (NextToken() != "(")
    throw "\"(\" expected";
  string tempString = "";
  NextToken();
  string idList = Ids();
  for (uint x = 0; x < idList.length(); x++) {
    if (idList.at(x) == ',') {
      Code("read", externalToInternal(tempString));
      if (symbolTable.at(getIntSymTableIndex(externalToInternal(tempString))).mode != 0) {
        throw "read only: cannot overwrite a constant";
      }
      tempString = "";
    } else {
      tempString = tempString + (idList.at(x));
    }
  }
  if (symbolTable.at(getIntSymTableIndex(externalToInternal(tempString))).mode != 0) {
    throw "read only: cannot overwrite a constant";
  }
  Code("read", externalToInternal(tempString));
  if (token != ")")
    throw "\")\" or \",\" expected";
  if (NextToken() != ";")
    throw "\";\" expected";
}

void WriteStmt() {
  if (NextToken() != "(")
    throw "\"(\" expected";
  string tempString = "";
  NextToken();
  string idList = Ids();
  for (uint x = 0; x < idList.length(); x++) {
    if (idList.at(x) == ',') {
      Code("write", externalToInternal(tempString));
      tempString = "";
    } else {
      tempString = tempString + (idList.at(x));
    }
  }
  Code("write", externalToInternal(tempString));
  if (token != ")")
    throw "\")\" or \",\" expected";
  if (NextToken() != ";")
    throw "\";\" expected";
}

// push name onto operatorStk
void PushOperator(string name) {
  operatorStk.push(name);
}

// push name onto operandStk
void PushOperand(string name) {
  if (isLiteral(name) && !isExtNameSymTable(name)) {
    Insert(name, WhichType(name), CONSTANT, WhichValue(name), YES, 1);
  }
  operandStk.push(externalToInternal(name));
}

// pop name from operatorStk
string PopOperator() {
  if (!operatorStk.empty()) {
    string discarded = operatorStk.top();
    operatorStk.pop();
    return discarded;
  }
  throw "operator stack underflow";
}

// pop name from operandStk
string PopOperand() {
  if (!operandStk.empty()) {
    string discarded = operandStk.top();
    operandStk.pop();
    return discarded;
  }
  throw "operand stack underflow";
}

void Code(string operatorr, string operand1, string operand2) {
  if (operatorr == "program") {
    objFileWrite("STRT", "NOP", "", "", "", programName + " - Amponsah Nana, McRae Angela");
  } else if (operatorr == "end") {
    rammVarsPrint(operand1);
  } else if (operatorr == "read") {
    string remark = "read(" + internalToExternal(operand1) + ")";
    objFileWrite("", "RDI", operand1, "", "", remark);
  } else if (operatorr == "write") {
    string remark = "write(" + internalToExternal(operand1) + ")";
    objFileWrite("", "PRI", operand1, "", "", remark);
  } else if (operatorr == "+") {
    EmitAdditionCode(operand1, operand2, INTEGER);
  } else if (operatorr == "*") {
    EmitMultiplicationCode(operand1, operand2, INTEGER);
  } else if (operatorr == "-") {
    EmitSubtractionCode(operand1, operand2, INTEGER);
  } else if (operatorr == "neg") {
    EmitNegateCode(operand1, INTEGER);
  } else if (operatorr == "not") {
    EmitNotCode(operand1, BOOLEAN);
  } else if (operatorr == "div") {
    EmitDivCode(operand1, operand2, INTEGER);
  } else if (operatorr == "mod") {
    EmitModCode(operand1, operand2, INTEGER);
  } else if (operatorr == "or") {
    EmitOrCode(operand1, operand2, BOOLEAN);
  } else if (operatorr == "and") {
    EmitAndCode(operand1, operand2, BOOLEAN);
  } else if (operatorr == "<") {
    EmitLessThanCode(operand1, operand2, INTEGER);
  } else if (operatorr == ">") {
    EmitGreaterThanCode(operand1, operand2, INTEGER);
  } else if (operatorr == "<=") {
    EmitLessThanEqualCode(operand1, operand2, INTEGER);
  } else if (operatorr == ">=") {
    EmitGreaterThanEqualCode(operand1, operand2, INTEGER);
  } else if (operatorr == "<>") {
    EmitNotEqualsCode(operand1, operand2, symbolTable.at(getIntSymTableIndex(operand1)).dataType == 0 ? INTEGER : BOOLEAN);
  } else if (operatorr == "=") {
    EmitEqualsCode(operand1, operand2, symbolTable.at(getIntSymTableIndex(operand1)).dataType == 0 ? INTEGER : BOOLEAN);
  } else if (operatorr == ":=") {
    EmitAssignCode(operand1, operand2);
  } else throw "undefined operation";
}

void Express() {
  if (NextToken() == "not" || 
      isBoolean(token) || 
      token == "(" || 
      token == "+" || 
      token == "-" || 
      isInteger(token) || 
      isNON_KEY_ID(token)) {
        Term();
        Expresses();
  } else { 
      throw "Keyword \"not\", \"true/false\", \"(\", \"+\", \"-\", integer, or NON-KEY-ID expected";
  }
}

void Expresses() {
  if (token == ")" || token == ";")
    return;
  if (!isRelOp(token))
    throw "\"=\", \"<>\", \"<=\", \">=\", \"<\", or \">\" expected";
  PushOperator(token);
  NextToken();
  Term();
  string x = PopOperator();
  string y = PopOperand();
  string z = PopOperand();
  Code(x, y, z);
  Expresses();
}

void Term() {
  if (token == "not" || 
      isBoolean(token) || 
      token == "(" || 
      token == "+" || 
      token == "-" || 
      isInteger(token) || 
      isNON_KEY_ID(token)) {
        Factor();
        Terms();
  } else {
      throw "Keyword \"not\", \"true/false\", \"(\", \"+\", \"-\", integer, or NON-KEY-ID expected";
  }
}

void Terms() {
  if (token == "<>" || 
      token == "=" || 
      token == "<=" || 
      token == ">=" || 
      token == "<" || 
      token == ">" || 
      token == ")" || 
      token == ";")
        return;
  if (!isAddLevelOp(token))
    throw "\"+\",\"-\", or \"or\" expected";
  PushOperator(token);
  NextToken();
  Factor();
  string x = PopOperator();
  string y = PopOperand();
  string z = PopOperand();
  Code(x, y, z);
  Terms();
}

void Factor() {
  if (token == "not" || 
      isBoolean(token) || 
      token == "(" || 
      token == "+" || 
      token == "-" || 
      isInteger(token) || 
      isNON_KEY_ID(token)) {
        Part();
        Factors();
  } else {
      throw "keyword \"not\", \"true/false\", \"(\", \"+\", \"-\", integer, or NON-KEY-ID expected";
  }
}

void Factors() {
  if (token == "<>" || 
      token == "=" || 
      token == "<=" || 
      token == ">=" || 
      token == "<" || 
      token == ">" || 
      token == ")" || 
      token == ";" || 
      token == "-" || 
      token == "+" || 
      token == "or") {
        return;
  }
  if (!isMultLevelOp(token)) {
    throw "keyword \"<>\", \"=\", \"<=\", \">=\", \"<\", \">\", \")\", \";\", \"-\", \"+\", \"or\", \"*\" ,\"div\" ,\"mod\" , or \"and\" expected";
  }
  PushOperator(token);
  NextToken();
  Part();
  string x = PopOperator();
  string y = PopOperand();
  string z = PopOperand();
  Code(x, y, z);
  Factors();
}

void Part() {
  if (token == "not") {
    if (NextToken() == "(") {
      Express();
      Code("not", PopOperand());
      if (token != ")")
        throw "\")\" expected";
      NextToken();
    } else if (isBoolean(token)) {
      PushOperand(token == "true" ? "false" : "true");
      NextToken();
    } else if (isNON_KEY_ID(token)) {
      Code("not", token);
      NextToken();
    } else throw "\"(\", BOOLEAN, or NON-KEY-ID expected";
  }
  else if (token == "+") {
    if (NextToken() == "(") {
      Express();
      if (token != ")")
        throw "\")\" expected";
      NextToken();
    } else if (!isInteger(token) && !isNON_KEY_ID(token)) {
        throw "\"(\", INTEGER, or NON-KEY-ID expected";
    } else {
      PushOperand(token);
      NextToken();
    }
  }
  else if (token == "-") {
    if (NextToken() == "(") {
      Express();
      Code("neg", PopOperand());
      if (token != ")")
        throw "\")\" expected";
      NextToken();
    } else if (isInteger(token)) {
      PushOperand("-" + token);
      NextToken();
    } else if (isNON_KEY_ID(token)) {
      Code("neg", token);
      NextToken();
    } else throw "\"(\", INTEGER, or NON-KEY-ID expected";
  }
  else if (isInteger(token) || isBoolean(token) || isNON_KEY_ID(token)) {
    PushOperand(token);
    NextToken();
  }
  else if (token == "(") {
    Express();
    if (token != ")")
      throw "\")\" expected";
    NextToken();
  } else throw "\"not\", \"+\", \"-\", \"(\", INTEGER, true, false, or NON-KEY-ID expected";
}

void compareOpnd(string operand1, string operand2) {
  if (aReg != operand1 && aReg != operand2) {
    objFileWrite("","LDA",operand2,"","","");
    aReg = operand2;
  }
}

void compareOpnd(string operand) {
  if (aReg != operand) {
    objFileWrite("","LDA",operand,"","","");
    aReg = operand;
  }
}

void EmitCodeHeader(string operand1, string operand2, storeType type) {
  if (symbolTable.at(getIntSymTableIndex(operand1)).dataType != type || symbolTable.at(getIntSymTableIndex(operand2)).dataType != type)
    throw "illegal type";
  if (aReg[0] == 'T' && aReg != operand1 && aReg != operand2 && aReg != "TRUE") {
    objFileWrite("", "STA", aReg, "", "", "deassign AReg");
    allocTemp(getIntSymTableIndex(aReg));
    aReg = "";
  }
  else if (aReg != operand1 && aReg != operand2) {
    aReg = "";
  }
}

void EmitCodeFooter(string operand1, string operand2, storeType type) {
  if (operand1[0] == 'T' && operand1 != "TRUE")
    FreeTemp();
  if (operand2[0] == 'T' && operand2 != "TRUE")
    FreeTemp();
  aReg = GetTemp();
  symbolTable.at(getIntSymTableIndex(aReg)).dataType = type;
  PushOperand(aReg);
}

void EmitAdditionCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand1, operand2);
  if (aReg == operand1 || aReg == operand2) {
    string operand = (aReg == operand1 ? operand2 : operand1);
    string remarks = internalToExternal(operand2) + " + " + internalToExternal(operand1);
    objFileWrite("", "IAD", operand, "", "", remarks);
  }
  EmitCodeFooter(operand1, operand2, type);
}

void EmitMultiplicationCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand1, operand2);
  if (aReg == operand1 || aReg == operand2) {
    string operand = (aReg == operand1 ? operand2 : operand1);
    string remarks = internalToExternal(operand2) + " * " + internalToExternal(operand1);
    objFileWrite("", "IMU", operand, "", "", remarks);
  }
  EmitCodeFooter(operand1, operand2, type);
}

void EmitSubtractionCode(string operand1, string operand2, storeType type) {
  if (symbolTable.at(getIntSymTableIndex(operand1)).dataType != type || symbolTable.at(getIntSymTableIndex(operand2)).dataType != type)
    throw "illegal type";
  if (aReg[0] == 'T' && aReg != operand2 && aReg != "TRUE") {
    objFileWrite("", "STA", aReg, "", "", "deassign AReg");
    allocTemp(getIntSymTableIndex(aReg));
    aReg = "";
  }
  else if (aReg != operand2) {
    aReg = "";
  }
  compareOpnd(operand2);
  string operand = (aReg == operand1 ? operand2 : operand1);
  string remarks = internalToExternal(operand2) + " - " + internalToExternal(operand1);
  objFileWrite("", "ISB", (aReg == operand1 ? operand2 : operand1), "", "", remarks);
  EmitCodeFooter(operand1, operand2, type);
}

void EmitNegateCode(string operand, storeType type) {
  if (!isExtNameSymTable("-1")) {
    Insert("-1", INTEGER, CONSTANT, "-1", YES, 1);
  }
  EmitMultiplicationCode(externalToInternal("-1"), externalToInternal(operand), type);
}

void EmitNotCode(string operand, storeType type) {
  operand = externalToInternal(operand);
 
  if (symbolTable.at(getIntSymTableIndex(operand)).dataType != type)
    throw "illegal type";
  if (aReg[0] == 'T' && aReg != "TRUE" && aReg != operand) {
    objFileWrite("", "STA", aReg, "", "", "deassign AReg");
    allocTemp(getIntSymTableIndex(aReg));
    aReg = "";
  }
  if (aReg[0] != 'T' && aReg != "TRUE" && aReg != operand) {
    aReg = "";
  }
 
  compareOpnd(operand);
  string labelStr = "L" + to_string(numLabels++);
  objFileWrite("", "AZJ", labelStr, "", "", "not " + internalToExternal(operand));
  objFileWrite("", "LDA", "FALS", "", "", "");
  if (!isExtNameSymTable("false")) {
    Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
  }
  objFileWrite("", "UNJ", labelStr, "+", "1", "");
  objFileWrite(labelStr, "LDA", "TRUE", "", "", "");
  if (!isExtNameSymTable("true")) {
    Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
  }
  
  if (operand[0] == 'T' && operand != "TRUE") {
    FreeTemp();
  }
  aReg = GetTemp();
  symbolTable.at(getIntSymTableIndex(aReg)).dataType = type;
  PushOperand(aReg);
}

void EmitDivCode(string operand1, string operand2, storeType type) {
  if (symbolTable.at(getIntSymTableIndex(operand1)).dataType != type || symbolTable.at(getIntSymTableIndex(operand2)).dataType != type)
    throw "illegal type";
  if (aReg[0] == 'T' && aReg != operand2 && aReg != "TRUE") {
    objFileWrite("", "STA", aReg, "", "", "deassign AReg");
    allocTemp(getIntSymTableIndex(aReg));
    aReg = "";
  }
  else if (aReg != operand2) {
    aReg = "";
  }
  if (symbolTable.at(getIntSymTableIndex(operand1)).dataType != type || symbolTable.at(getIntSymTableIndex(operand2)).dataType != type)
    throw "illegal type";
  if (aReg[0] == 'T' && aReg != operand2 && aReg != "TRUE") {
    objFileWrite("", "STA", aReg, "", "", "deassign AReg");
    allocTemp(getIntSymTableIndex(aReg));
    aReg = "";
  }
  else if (aReg != operand2) {
    aReg = "";
  }
  compareOpnd(operand2);
  string operand = (aReg == operand1 ? operand2 : operand1);
  string remarks = internalToExternal(aReg) + " div " + internalToExternal(operand);
  objFileWrite("", "IDV", operand, "", "", remarks);
  EmitCodeFooter(operand1, operand2, type);
}

void EmitModCode(string operand1, string operand2, storeType type) {
  if (symbolTable.at(getIntSymTableIndex(operand1)).dataType != type || symbolTable.at(getIntSymTableIndex(operand2)).dataType != type)
    throw "illegal type";
  if (aReg[0] == 'T' && aReg != operand2 && aReg != "TRUE") {
    objFileWrite("", "STA", aReg, "", "", "deassign AReg");
    allocTemp(getIntSymTableIndex(aReg));
    aReg = "";
  }
  else if (aReg != operand2) {
    aReg = "";
  }
  compareOpnd(operand2);
  string remarks = internalToExternal(aReg) + " mod " + internalToExternal(operand1);
  objFileWrite("", "IDV", operand1, "", "", remarks);
  EmitCodeFooter(operand1, operand2, type);
  objFileWrite("", "STQ", aReg, "", "", "store remainder in memory");
  objFileWrite("", "LDA", aReg, "", "", "load remainder from memory");
  symbolTable.at(getIntSymTableIndex(aReg)).alloc = YES;
  symbolTable.at(getIntSymTableIndex(aReg)).units = 1;
}

void EmitOrCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand1, operand2);
  if (aReg == operand1 || aReg == operand2) {
    string operand = (aReg == operand1 ? operand2 : operand1);
    string remarks = internalToExternal(operand2) + " or " + internalToExternal(operand1);
    objFileWrite("", "IAD", operand, "", "", remarks);
  }
  string labelStr = "L" + to_string(numLabels++);
  objFileWrite("", "AZJ", labelStr, "+", "1", "");
  objFileWrite(labelStr, "LDA", "TRUE", "", "", "");
  if (!isExtNameSymTable("true")) {
    Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
  }
  EmitCodeFooter(operand1, operand2, type);
}

void EmitAndCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand1, operand2);
  if (aReg == operand1 || aReg == operand2) {
    string operand = (aReg == operand1 ? operand2 : operand1);
    string remarks = internalToExternal(operand2) + " and " + internalToExternal(operand1);
    objFileWrite("", "IMU", operand, "", "", remarks);
  }
  EmitCodeFooter(operand1, operand2, type);
}

void EmitLessThanCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand2);
  string remarks = internalToExternal(aReg) + " < " + internalToExternal(operand1);
  objFileWrite("", "ISB", operand1, "", "", remarks);
  string labelStr = "L" + to_string(numLabels++);
  objFileWrite("", "AMJ", labelStr, "", "", "");
  objFileWrite("", "LDA", "FALS", "", "", "");
  if (!isExtNameSymTable("false")) {
    Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
  }
  objFileWrite("", "UNJ", labelStr, "+", "1", "");
  objFileWrite(labelStr, "LDA", "TRUE", "", "", "");
  if (!isExtNameSymTable("true")) {
    Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
  }
  EmitCodeFooter(operand1, operand2, BOOLEAN);
}

void EmitGreaterThanCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand1);
  string remarks = internalToExternal(operand2) + " > " + internalToExternal(aReg);
  objFileWrite("", "ISB", operand2, "", "", remarks);
  string labelStr = "L" + to_string(numLabels++);
  objFileWrite("", "AMJ", labelStr, "", "", "");
  objFileWrite("", "LDA", "FALS", "", "", "");
  if (!isExtNameSymTable("false")) {
    Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
  }
  objFileWrite("", "UNJ", labelStr, "+", "1", "");
  objFileWrite(labelStr, "LDA", "TRUE", "", "", "");
  if (!isExtNameSymTable("true")) {
    Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
  }
  EmitCodeFooter(operand1, operand2, BOOLEAN);
}

void EmitLessThanEqualCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand1);
  string operand = (aReg == operand1 ? operand2 : operand1);
  string remarks = internalToExternal(operand) + " <= " + internalToExternal(aReg);
  objFileWrite("", "ISB", operand, "", "", remarks);
  string labelStr = "L" + to_string(numLabels++);
  objFileWrite("", "AMJ", labelStr, "", "", "");
  objFileWrite("", "LDA", "TRUE", "", "", "");
  if (!isExtNameSymTable("true")) {
    Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
  }
  objFileWrite("", "UNJ", labelStr, "+", "1", "");
  objFileWrite(labelStr, "LDA", "FALS", "", "", "");
  if (!isExtNameSymTable("false")) {
    Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
  }
  EmitCodeFooter(operand1, operand2, BOOLEAN);
}

void EmitGreaterThanEqualCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand2);
  string operand = (aReg == operand1 ? operand2 : operand1);
  string remarks = internalToExternal(aReg) + " >= " + internalToExternal(operand);
  objFileWrite("", "ISB", operand1, "", "", remarks);
  string labelStr = "L" + to_string(numLabels++);
  objFileWrite("", "AMJ", labelStr, "", "", "");
  objFileWrite("", "LDA", "TRUE", "", "", "");
  if (!isExtNameSymTable("true")) {
    Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
  }
  objFileWrite("", "UNJ", labelStr, "+", "1", "");
  objFileWrite(labelStr, "LDA", "FALS", "", "", "");
  if (!isExtNameSymTable("false")) {
    Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
  }
  EmitCodeFooter(operand1, operand2, BOOLEAN);
}

void EmitNotEqualsCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand1, operand2);
  if (aReg == operand1 || aReg == operand2) {
    string operand = (aReg == operand1 ? operand2 : operand1);
    string remarks = internalToExternal(aReg) + " <> " + internalToExternal(operand);
    objFileWrite("", "ISB", operand, "", "", remarks);
  }
  string labelStr = "L" + to_string(numLabels++);
  objFileWrite("", "AZJ", labelStr, "+", "1", "");
  objFileWrite(labelStr, "LDA", "TRUE", "", "", "");
  if (!isExtNameSymTable("true")) {
    Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
  }
  EmitCodeFooter(operand1, operand2, BOOLEAN);
}

void EmitEqualsCode(string operand1, string operand2, storeType type) {
  EmitCodeHeader(operand1, operand2, type);
  compareOpnd(operand1, operand2);
  if (aReg == operand1 || aReg == operand2) {
    string operand = (aReg == operand1 ? operand2 : operand1);
    string remarks = internalToExternal(aReg) + " = " + internalToExternal(operand);
    objFileWrite("", "ISB", operand, "", "", remarks);
  }
  string labelStr = "L" + to_string(numLabels++);
  objFileWrite("", "AZJ", labelStr, "", "", "");
  objFileWrite("", "LDA", "FALS", "", "", "");
  if (!isExtNameSymTable("false")) {
    Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
  } 
  objFileWrite("", "UNJ", labelStr, "+", "1", "");
  objFileWrite(labelStr, "LDA", "TRUE", "", "", "");
  if (!isExtNameSymTable("true")) {
    Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
  }
  EmitCodeFooter(operand1, operand2, BOOLEAN);
}

void EmitAssignCode(string operand1, string operand2) {
  if (symbolTable.at(getIntSymTableIndex(operand1)).dataType != symbolTable.at(getIntSymTableIndex(operand2)).dataType)
    throw "illegal type";
  if (symbolTable.at(getIntSymTableIndex(operand2)).mode != VARIABLE)
    throw "symbol on left-hand side of assignment must have a storage mode of VARIABLE";
  compareOpnd(operand1);
  string remarks = internalToExternal(operand2) + " := " + internalToExternal(aReg);
  objFileWrite("", "STA", operand2, "", "", remarks);
  if (operand1[0] == 'T' && operand1 != "TRUE")
    FreeTemp();
  aReg = operand2;
}