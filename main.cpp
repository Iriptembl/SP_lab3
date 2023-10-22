#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <string>

using namespace std;

enum Type {
    KEYWORD, IDENTIFIER, STRING_CONSTANT, NUMERIC_CONSTANT, HEXADECIMAL_NUMBER, DECIMAL_NUMBER, OPERATOR, DELIMITER, PREPROCESSOR_DIRECTIVE, COMMENT, UNKNOWN
};

struct Analiz {
    string value;
    Type type;
};

bool isReservedWord(const string& lex);
bool isIdentifier(const string& lex);
bool isStringOrCharConstant(const string& lex);
bool isHexadecimalNumber(const string& lex);
bool isDecimalNumber(const string& lex);
bool isNumericConstant(const string& lex);
bool isOperator(const string& lex);
bool isDelimiter(const string& lex);
bool isPreprocessorDirective(const string& lex);
bool isComment(const string& lex);
vector<Analiz> analize(ifstream& file);
void display(const vector<Analiz>& lexems);

int main() {

    string user, filepath;
    ifstream file("..\\example.cpp");
    if (!file.is_open())
    {
        cout << "Opening error. Print 'yes' to continue, print 'no' to exit: \n" << endl;
        do {
            cin >> user;
            if (user == "yes") {
                cout << "Enter the file path:";
                cin >> filepath;
                ifstream file(filepath);
                break;
            } else if (user == "no") {
                cout << "End." << endl;
                return 0;
            } else {
                cout << "Invalid input. Please enter 'yes' or 'no'." << endl;
            }
        } while (true);
    }

    vector<Analiz> lexems = analize(file);
    display(lexems);

    return 0;
}

bool isReservedWord(const string& lex) {
    vector<string> reserved_words = { "auto", "break", "case", "catch", "class", "const", "continue",
                                      "default", "do", "double", "else", "enum", "extern", "false", "float",
                                      "for", "if", "int", "long", "namespace", "new", "operator",
                                      "private", "protected", "public", "return", "short", "signed", "sizeof",
                                      "static", "struct", "switch", "template", "this", "throw", "true", "try",
                                      "typedef", "unsigned", "using", "void", "while"};
    return find(reserved_words.begin(), reserved_words.end(), lex) != reserved_words.end();
}

bool isIdentifier(const string& lex) {
    regex identifier_regex("[_a-zA-Z][_a-zA-Z0-9]*");
    return regex_match(lex, identifier_regex);
}

bool isStringOrCharConstant(const string& lex) {
    regex string_regex(R"((\"[^\"\\]*(\\.[^\"\\]*)*\")|(\'[^\'\\]*(\\.[^\'\\]*)*\'))");
    return regex_match(lex, string_regex);
}

bool isNumericConstant(const string& lex) {
    regex numeric_regex("^([0-9]*\\.?[0-9]+|0x[0-9a-fA-F]+)$");
    return regex_match(lex, numeric_regex);
}

bool isOperator(const string& lex) {
    vector<string> operators = { "=", "+", "-", "*", "/", "%", "&&", "||", "!", "==", "!=", "<", ">", "<=", ">=" };
    return find(operators.begin(), operators.end(), lex) != operators.end();
}

bool isDelimiter(const string& lex) {
    vector<string> delimiters = { ",", ";", ":", "(", ")", "{", "}", "[", "]" };
    return find(delimiters.begin(), delimiters.end(), lex) != delimiters.end();
}

bool isPreprocessorDirective(const string& lex) {
    regex preprocessorRgx("#include <[a-zA-Z_][a-zA-Z0-9_\\.]*>");
    return regex_match(lex, preprocessorRgx);
}

bool isComment(const string& lex) {
    regex comment_regex(R"((\\|/\*([^*]|\*+[^/])*\*+/))");
    return regex_match(lex, comment_regex);
}

vector<Analiz> analize(ifstream& file) {
    vector<Analiz> lexems;
    string lex;
    bool inString = false;
    bool inPreprocessor = false;
    bool inSingleLineComment = false;
    bool inMultiLineComment = false;
    bool prevCharWasAsterisk = false;

    char c;
    while (file.get(c)) {
        if (inString) {
            lex += c;
            if (c == '"') {
                inString = false;
                lexems.push_back({lex, STRING_CONSTANT});
                lex.clear();
            }
        } else if (inPreprocessor) {
            lex += c;
            if (c == '\n') {
                inPreprocessor = false;
                lexems.push_back({lex, PREPROCESSOR_DIRECTIVE});
                lex.clear();
            }
        } else if (inSingleLineComment) {
            lex += c;
            if (c == '\n') {
                inSingleLineComment = false;
                lexems.push_back({lex, COMMENT});
                lex.clear();
            }
        } else if (inMultiLineComment) {
            lex += c;
            if (prevCharWasAsterisk && c == '/') {
                inMultiLineComment = false;
                lexems.push_back({lex, COMMENT});
                lex.clear();
            }
            prevCharWasAsterisk = (c == '*');
        } else if (isalnum(c) || c == '_') {
            lex += c;
        } else if (c == '"' ) {
            inString = true;
            lex += c;
        } else if (c == '#') {
            inPreprocessor = true;
            lex += c;
        } else if (c == '.') {
            lex += c;
        } else if (c == '/') {
            if (lex.empty() && file.peek() == '/') {
                inSingleLineComment = true;
                lex += c;
            } else if (lex.empty() && file.peek() == '*') {
                inMultiLineComment = true;
                prevCharWasAsterisk = false;
                lex += c;
            } else if (lex.empty()) {
                lex += c;
            } else {
                if (!lex.empty()) {
                    if (isReservedWord(lex)) lexems.push_back({lex, KEYWORD});
                    else if (isIdentifier(lex)) lexems.push_back({lex, IDENTIFIER});
                    else if (isStringOrCharConstant(lex)) lexems.push_back({lex, STRING_CONSTANT});
                    else if (isNumericConstant(lex)) lexems.push_back({lex, NUMERIC_CONSTANT});
                    else if (isOperator(lex)) lexems.push_back({lex, OPERATOR});
                    else if (isDelimiter(lex)) lexems.push_back({lex, DELIMITER});
                    else if (isPreprocessorDirective(lex)) lexems.push_back({lex, PREPROCESSOR_DIRECTIVE});
                    else if (isComment(lex)) lexems.push_back({lex, COMMENT});
                    else lexems.push_back({lex, UNKNOWN});
                    lex.clear();
                }
                lex += c;
                if (isDelimiter(lex)) {
                    lexems.push_back({lex, DELIMITER});
                    lex.clear();
                } else if (isOperator(lex)) {
                    lexems.push_back({lex, OPERATOR});
                    lex.clear();
                }
            }
        } else if (isspace(c)) {
            if (!lex.empty()) {
                if (isReservedWord(lex)) lexems.push_back({lex, KEYWORD});
                else if (isIdentifier(lex)) lexems.push_back({lex, IDENTIFIER});
                else if (isStringOrCharConstant(lex)) lexems.push_back({lex, STRING_CONSTANT});
                else if (isNumericConstant(lex)) lexems.push_back({lex, NUMERIC_CONSTANT});
                else if (isOperator(lex)) lexems.push_back({lex, OPERATOR});
                else if (isDelimiter(lex)) lexems.push_back({lex, DELIMITER});
                else if (isPreprocessorDirective(lex)) lexems.push_back({lex, PREPROCESSOR_DIRECTIVE});
                else if (isComment(lex)) lexems.push_back({lex, COMMENT});
                else lexems.push_back({lex, UNKNOWN});
                lex.clear();
            }
        } else {
            if (!lex.empty()) {
                if (isReservedWord(lex)) lexems.push_back({lex, KEYWORD});
                else if (isIdentifier(lex)) lexems.push_back({lex, IDENTIFIER});
                else if (isStringOrCharConstant(lex)) lexems.push_back({lex, STRING_CONSTANT});
                else if (isNumericConstant(lex)) lexems.push_back({lex, NUMERIC_CONSTANT});
                else if (isOperator(lex)) lexems.push_back({lex, OPERATOR});
                else if (isDelimiter(lex)) lexems.push_back({lex, DELIMITER});
                else if (isPreprocessorDirective(lex)) lexems.push_back({lex, PREPROCESSOR_DIRECTIVE});
                else if (isComment(lex)) lexems.push_back({lex, COMMENT});
                else lexems.push_back({lex, UNKNOWN});
                lex.clear();
            }
            lex += c;
            if (isDelimiter(lex)) {
                lexems.push_back({lex, DELIMITER});
                lex.clear();
            } else if (isOperator(lex)) {
                lexems.push_back({lex, OPERATOR});
                lex.clear();
            }
        }
    }

    if (!lex.empty()) {
        if (isReservedWord(lex)) lexems.push_back({lex, KEYWORD});
        else if (isIdentifier(lex)) lexems.push_back({lex, IDENTIFIER});
        else if (isStringOrCharConstant(lex)) lexems.push_back({lex, STRING_CONSTANT});
        else if (isNumericConstant(lex)) lexems.push_back({lex, NUMERIC_CONSTANT});
        else if (isOperator(lex)) lexems.push_back({lex, OPERATOR});
        else if (isDelimiter(lex)) lexems.push_back({lex, DELIMITER});
        else if (isPreprocessorDirective(lex)) lexems.push_back({lex, PREPROCESSOR_DIRECTIVE});
        else if (isComment(lex)) lexems.push_back({lex, COMMENT});
        else lexems.push_back({lex, UNKNOWN});
    }

    return lexems;
}


void display(const vector<Analiz>& lexems) {
    for (const Analiz& part : lexems) {
        string type;
        switch (part.type) {
            case KEYWORD: type = "Keyword"; break;
            case IDENTIFIER: type = "Identifier"; break;
            case STRING_CONSTANT: type = "String Constant"; break;
            case NUMERIC_CONSTANT: type = "Numeric Constant"; break;
            case HEXADECIMAL_NUMBER: type = "Hexadecimal Number"; break;
            case DECIMAL_NUMBER: type = "Decimal Number"; break;
            case OPERATOR: type = "Operator"; break;
            case DELIMITER: type = "Delimiter"; break;
            case PREPROCESSOR_DIRECTIVE: type = "Preprocessor Directive"; break;
            case COMMENT: type = "Comment"; break;
            case UNKNOWN: type = "Unknown"; break;
        }
        cout << part.value << " - Type: " << type << endl;
    }
}
