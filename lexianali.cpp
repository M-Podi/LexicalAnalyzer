#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>

enum class State
{
    None,
    Identifier,
    Keyword,
    Literal,
    Operator,
    Separator,
    Comment,
    Invalid,
    PreprocessorDirective
};

bool isOperator(const std::string &str)
{
    static const std::vector<std::string> operators{
        "+", "-", "*", "/", "%",           // Arithmetic operators
        "++", "--",                        // Increment and decrement operators
        "=", "+=", "-=", "*=", "/=", "%=", // Assignment operators
        "==", "!=", ">", "<", ">=", "<=",  // Comparison operators
        "&&", "||", "!",                   // Logical operators
        "&", "|", "^", "~", "<<", ">>",    // Bitwise operators
        "?:",                              // Ternary conditional operator
        "::", ".", "->",                   // Access operators
    };

    return std::find(operators.begin(), operators.end(), str) != operators.end();
}

bool isSeparator(const std::string &str)
{
    static const std::vector<std::string> separators{
        ";", ",", ":",                // Common separators
        "(", ")", "[", "]", "{", "}", // Parentheses, brackets, and braces
        ".", "->", "::",              // Access operators which can also act as separators
        "#",                          // Preprocessor directive in some languages like C++
    };

    return std::find(separators.begin(), separators.end(), str) != separators.end();
}

bool isKeyword(const std::string &str)
{
    static const std::vector<std::string> keywords{
        "if", "else", "while", "do", "for", "switch", "case", "default",      // Control structures
        "int", "char", "double", "float", "long", "short", "bool", "void",    // Primitive data types
        "class", "struct", "union", "enum", "typedef", "template",            // User-defined data types
        "public", "private", "protected", "friend",                           // Access specifiers
        "const", "static", "volatile", "extern",                              // Type qualifiers and storage class specifiers
        "return", "break", "continue", "goto",                                // Jump statements
        "try", "catch", "throw", "finally",                                   // Exception handling
        "new", "delete", "this", "operator", "sizeof", "typeof", "constexpr", // Operators and size specifiers
        "auto", "register", "using", "namespace", "include",                  // Other keywords in languages like C++
    };

    return std::find(keywords.begin(), keywords.end(), str) != keywords.end();
}

bool isDigit(const std::string &str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

bool isInteger(const std::string &str)
{
    if (str.empty())
        return false;
    size_t startPos = (str[0] == '-' || str[0] == '+') ? 1 : 0;
    return isDigit(str.substr(startPos));
}

bool isFloatingPoint(const std::string &str)
{
    if (str.empty())
        return false;
    size_t dotPos = str.find('.');
    if (dotPos == std::string::npos || dotPos == 0 || dotPos == str.size() - 1)
        return false;
    return isInteger(str.substr(0, dotPos)) && isDigit(str.substr(dotPos + 1));
}

bool isCharacter(const std::string &str)
{
    return str.size() == 3 && str[0] == '\'' && str[2] == '\'';
}

bool isString(const std::string &str)
{
    return str.size() >= 2 && str.front() == '"' && str.back() == '"';
}

bool isBool(const std::string &str)
{
    return str == "true" || str == "false";
}

bool isLiteral(const std::string &str)
{
    return isInteger(str) || isFloatingPoint(str) || isCharacter(str) || isString(str) || isBool(str);
}

void printToken(const std::string &token, State state)
{
    switch (state)
    {
    case State::Identifier:
        std::cout << "(identifier, " << token << ")\n";
        break;
    case State::Keyword:
        std::cout << "(keyword, " << token << ")\n";
        break;
    case State::Literal:
        std::cout << "(literal, " << token << ")\n";
        break;
    case State::Operator:
        std::cout << "(operator, " << token << ")\n";
        break;
    case State::Separator:
        std::cout << "(separator, " << token << ")\n";
        break;
    case State::Comment:
        std::cout << "(comment, " << token << ")\n";
        break;
    case State::Invalid:
        std::cout << "(invalid, " << token << ")\n";
        break;
    case State::PreprocessorDirective:
        std::cout << "(preprocessor directive, " << token << ")\n";
        break;
    default:
        std::cout << "(unknown, " << token << ")\n";
        break;
    }
}

void lexicalAnalyze(const std::string &nameOfFile)
{
    std::fstream file(nameOfFile, std::fstream::in);
    if (!file.is_open())
    {
        std::cout << "Error opening file\n";
        return;
    }

    char ch;
    std::string buffer;
    State currentState = State::None;
    bool insideStringLiteral = false;

    while (file >> std::noskipws >> ch)
    {
        if (insideStringLiteral)
        {
            buffer += ch;
            if (ch == '"')
            {
                printToken(buffer, State::Literal);
                buffer.clear();
                insideStringLiteral = false;
                currentState = State::None;
            }
            continue;
        }

        switch (currentState)
        {
        case State::None:
            if (std::isspace(ch))
            {
                // Skip whitespace
                continue;
            }
            if (ch == '#')
            {
                currentState = State::PreprocessorDirective;
                //buffer += ch;
            }
            else if (std::isalpha(ch) || ch == '_')
            {
                currentState = State::Identifier;
            }
            else if (std::isdigit(ch))
            {
                currentState = State::Literal;
            }
            else if (ch == '"')
            {
                insideStringLiteral = true;
                currentState = State::Literal;
            }
            else if (isOperator(std::string(1, ch)) || isSeparator(std::string(1, ch)))
            {
                currentState = isOperator(std::string(1, ch)) ? State::Operator : State::Separator;
            }
            else
            {
                currentState = State::Invalid;
            }
            buffer += ch;
            break;
        case State::PreprocessorDirective:
            if (ch == '\n')
            {
                printToken(buffer, State::PreprocessorDirective);
                buffer.clear();
                currentState = State::None;
            }
            else
            {
                buffer += ch;
            }
            break;
        case State::Identifier:
            if (!(std::isalnum(ch) || ch == '_'))
            {
                // Handle potential namespace qualifier
                if (ch == ':' && file.peek() == ':')
                {
                    buffer += ch; // Add the first colon
                    file >> ch;   // Read the second colon
                    buffer += ch; // Add the second colon
                    continue;     // Continue building the namespace qualified identifier
                }

                // Handle normal identifier
                if (buffer == "std::cout" || buffer == "std::endl")
                {
                    printToken(buffer, State::Identifier); // Treat std::cout and std::endl as single identifiers
                }
                else if (isKeyword(buffer))
                {
                    printToken(buffer, State::Keyword);
                }
                else
                {
                    printToken(buffer, State::Identifier);
                }
                buffer.clear();
                currentState = State::None;
                continue;
            }
            buffer += ch;
            break;
        case State::Literal:
            if (!std::isdigit(ch) && ch != '.' && ch != '\'')
            {
                printToken(buffer, State::Literal);
                buffer.clear();
                currentState = State::None;
                continue;
            }
            buffer += ch;
            break;
        case State::Operator:
        case State::Separator:
            if (!isOperator(buffer + ch) && !isSeparator(buffer + ch))
            {
                printToken(buffer, currentState);
                buffer.clear();
                currentState = State::None;
                continue;
            }
            buffer += ch;
            break;
        case State::Comment:
            if (ch == '\n')
            {
                printToken(buffer, State::Comment);
                buffer.clear();
                currentState = State::None;
            }
            else
            {
                buffer += ch;
            }
            break;
        case State::Invalid:
            printToken(buffer, State::Invalid);
            buffer.clear();
            currentState = State::None;
            continue;
        default:
            break;
        }
    }

    if (!buffer.empty())
    {
        // Handle the final token
        switch (currentState)
        {
        case State::Identifier:
            printToken(buffer, isKeyword(buffer) ? State::Keyword : State::Identifier);
            break;
        case State::Literal:
        case State::Operator:
        case State::Separator:
        case State::Comment:
        case State::Invalid:
            printToken(buffer, currentState);
            break;
        default:
            break;
        }
    }

    file.close();
}

int main()
{
    std::string testFileName = "test_code.txt"; // The name of the test file
    lexicalAnalyze(testFileName);               // Call the lexical analyzer with the test file
    return 0;
}
