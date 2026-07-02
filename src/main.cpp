#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <unordered_set>
#include <unordered_map>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define GETCH() _getch()
#else
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
int getch() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#define GETCH() getch()
#endif

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

size_t countSpaces(const std::string& str) {
    return std::count(str.begin(), str.end(), ' ');
}

std::string getFileExtension(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "";
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

bool isCppFile(const std::string& path) {
    std::string ext = getFileExtension(path);
    return ext == "cpp" || ext == "cc" || ext == "cxx" || ext == "h" || ext == "hpp";
}

const std::string RESET = "\033[0m";
const std::string COLOR_KEYWORD = "\033[34m";
const std::string COLOR_TYPE = "\033[92m";
const std::string COLOR_OPERATOR = "\033[31m";
const std::string COLOR_PUNCTUATOR = "\033[35m";
const std::string COLOR_NUMBER = "\033[95m";
const std::string COLOR_STRING = "\033[93m";
const std::string COLOR_COMMENT = "\033[90m";
const std::string COLOR_PREPROCESSOR = "\033[36m";
const std::string COLOR_IDENTIFIER = "\033[37m";

const std::unordered_set<std::string> KEYWORDS = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break", "case",
    "catch", "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const",
    "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await", "co_return",
    "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
    "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int",
    "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or",
    "or_eq", "private", "protected", "public", "register", "reinterpret_cast", "requires",
    "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
    "switch", "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid",
    "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while",
    "xor", "xor_eq"
};

const std::unordered_set<std::string> TYPES = {
    "void", "bool", "char", "wchar_t", "char8_t", "char16_t", "char32_t",
    "short", "int", "long", "long long",
    "float", "double", "long double",
    "signed", "unsigned"
};

struct Token {
    std::string text;
    std::string color;
};

class CppTokenizer {
public:
    explicit CppTokenizer(const std::string& content) : content(content), pos(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < content.length()) {
            char c = content[pos];
            if (std::isspace(static_cast<unsigned char>(c))) {
                tokens.push_back(consumeWhitespace());
            } else if (c == '#') {
                tokens.push_back(consumePreprocessor());
            } else if (c == '/' && pos + 1 < content.length() && (content[pos+1] == '/' || content[pos+1] == '*')) {
                tokens.push_back(consumeComment());
            } else if (c == '"') {
                tokens.push_back(consumeString());
            } else if (c == '\'') {
                tokens.push_back(consumeChar());
            } else if (std::isdigit(static_cast<unsigned char>(c)) || (c == '.' && pos+1 < content.length() && std::isdigit(static_cast<unsigned char>(content[pos+1])))) {
                tokens.push_back(consumeNumber());
            } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                tokens.push_back(consumeIdentifierOrKeyword());
            } else {
                tokens.push_back(consumeOperatorOrPunctuator());
            }
        }
        return tokens;
    }

private:
    std::string content;
    size_t pos;

    Token consumeWhitespace() {
        size_t start = pos;
        while (pos < content.length() && std::isspace(static_cast<unsigned char>(content[pos]))) {
            ++pos;
        }
        return {content.substr(start, pos - start), ""};
    }

    Token consumePreprocessor() {
        size_t start = pos;
        while (pos < content.length() && content[pos] != '\n') {
            ++pos;
        }
        if (pos < content.length() && content[pos] == '\n') ++pos;
        return {content.substr(start, pos - start), COLOR_PREPROCESSOR};
    }

    Token consumeComment() {
        size_t start = pos;
        if (content[pos] == '/' && content[pos+1] == '/') {
            pos += 2;
            while (pos < content.length() && content[pos] != '\n') {
                ++pos;
            }
            if (pos < content.length() && content[pos] == '\n') ++pos;
        } else if (content[pos] == '/' && content[pos+1] == '*') {
            pos += 2;
            while (pos + 1 < content.length() && !(content[pos] == '*' && content[pos+1] == '/')) {
                ++pos;
            }
            pos += 2;
        }
        return {content.substr(start, pos - start), COLOR_COMMENT};
    }

    Token consumeString() {
        size_t start = pos;
        ++pos;
        while (pos < content.length()) {
            if (content[pos] == '\\' && pos + 1 < content.length()) {
                pos += 2;
            } else if (content[pos] == '"') {
                ++pos;
                break;
            } else {
                ++pos;
            }
        }
        return {content.substr(start, pos - start), COLOR_STRING};
    }

    Token consumeChar() {
        size_t start = pos;
        ++pos;
        while (pos < content.length()) {
            if (content[pos] == '\\' && pos + 1 < content.length()) {
                pos += 2;
            } else if (content[pos] == '\'') {
                ++pos;
                break;
            } else {
                ++pos;
            }
        }
        return {content.substr(start, pos - start), COLOR_STRING};
    }

    Token consumeNumber() {
        size_t start = pos;
        bool hex = false;
        bool bin = false;
        if (content[pos] == '0' && pos + 1 < content.length()) {
            char next = content[pos+1];
            if (next == 'x' || next == 'X') {
                hex = true;
                pos += 2;
            } else if (next == 'b' || next == 'B') {
                bin = true;
                pos += 2;
            }
        }
        while (pos < content.length()) {
            char c = content[pos];
            if (std::isdigit(static_cast<unsigned char>(c))) {
                ++pos;
            } else if (hex && ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                ++pos;
            } else if (bin && (c == '0' || c == '1')) {
                ++pos;
            } else if (c == '.') {
                ++pos;
            } else if (c == 'e' || c == 'E' || c == 'p' || c == 'P') {
                ++pos;
                if (pos < content.length() && (content[pos] == '+' || content[pos] == '-')) ++pos;
            } else if (c == 'u' || c == 'U' || c == 'l' || c == 'L' || c == 'f' || c == 'F') {
                ++pos;
            } else {
                break;
            }
        }
        return {content.substr(start, pos - start), COLOR_NUMBER};
    }

    Token consumeIdentifierOrKeyword() {
        size_t start = pos;
        while (pos < content.length() && (std::isalnum(static_cast<unsigned char>(content[pos])) || content[pos] == '_')) {
            ++pos;
        }
        std::string word = content.substr(start, pos - start);
        if (KEYWORDS.find(word) != KEYWORDS.end()) {
            if (TYPES.find(word) != TYPES.end()) {
                return {word, COLOR_TYPE};
            }
            return {word, COLOR_KEYWORD};
        }
        return {word, COLOR_IDENTIFIER};
    }

    Token consumeOperatorOrPunctuator() {
        size_t start = pos;
        char c = content[pos];
        if (pos + 1 < content.length()) {
            std::string two = content.substr(pos, 2);
            if (two == "==" || two == "!=" || two == "<=" || two == ">=" || two == "&&" || two == "||" ||
                two == "++" || two == "--" || two == "+=" || two == "-=" || two == "*=" || two == "/=" ||
                two == "%=" || two == "&=" || two == "|=" || two == "^=" || two == "<<=" || two == ">>=" ||
                two == "<<" || two == ">>" || two == "->" || two == "::" || two == "##" || two == ".*" ||
                two == "->*" || two == "<=>" || two == "or" || two == "and" || two == "xor" ||
                two == "not" || two == "bitand" || two == "bitor" || two == "compl" || two == "and_eq" ||
                two == "or_eq" || two == "xor_eq" || two == "not_eq") {
                pos += 2;
                return {content.substr(start, pos - start), COLOR_OPERATOR};
            }
        }
        if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' ||
            c == ';' || c == ',' || c == '.' || c == '?' || c == ':' || c == '#' || c == '~') {
            ++pos;
            return {content.substr(start, pos - start), COLOR_PUNCTUATOR};
        }
        ++pos;
        return {content.substr(start, pos - start), COLOR_OPERATOR};
    }
};

int main(int argc, char* argv[]) {
    std::string nombreArchivo;
    std::string modo = "word";

    if (argc >= 2) {
        nombreArchivo = argv[1];
        if (argc >= 3) {
            modo = argv[2];
            std::transform(modo.begin(), modo.end(), modo.begin(), ::tolower);
        }
    } else {
        std::cout << "Introduce la ruta del archivo de texto: ";
        std::getline(std::cin, nombreArchivo);
        std::cout << "Modo (char / word) [default word]: ";
        std::string entradaModo;
        std::getline(std::cin, entradaModo);
        if (!entradaModo.empty()) {
            modo = entradaModo;
            std::transform(modo.begin(), modo.end(), modo.begin(), ::tolower);
        }
    }

    if (modo != "char" && modo != "word") {
        std::cerr << "Modo inválido. Usa 'char' o 'word'.\n";
        return 1;
    }

    std::ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        std::cerr << "Error: no se pudo abrir el archivo \"" << nombreArchivo << "\"\n";
        return 1;
    }

    std::string contenido;
    std::string linea;
    while (std::getline(archivo, linea)) {
        contenido += linea;
        contenido.push_back('\n');
    }
    archivo.close();

    if (contenido.empty()) {
        std::cerr << "El archivo está vacío.\n";
        return 1;
    }

    size_t totalEspacios = countSpaces(contenido);

#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif

    std::vector<std::string> segmentosTexto;
    std::vector<std::string> segmentosColor;

    bool highlight = isCppFile(nombreArchivo);

    if (highlight) {
        CppTokenizer tokenizer(contenido);
        std::vector<Token> tokens = tokenizer.tokenize();
        for (const Token& tok : tokens) {
            segmentosTexto.push_back(tok.text);
            segmentosColor.push_back(tok.color);
        }
    } else {
        if (modo == "char") {
            for (char c : contenido) {
                segmentosTexto.push_back(std::string(1, c));
                segmentosColor.push_back("");
            }
        } else {
            bool esEspacio = std::isspace(static_cast<unsigned char>(contenido[0]));
            std::string acumulado;
            for (char c : contenido) {
                bool actualEsEspacio = std::isspace(static_cast<unsigned char>(c));
                if (actualEsEspacio != esEspacio) {
                    if (!acumulado.empty()) {
                        segmentosTexto.push_back(acumulado);
                        segmentosColor.push_back("");
                        acumulado.clear();
                    }
                    esEspacio = actualEsEspacio;
                }
                acumulado.push_back(c);
            }
            if (!acumulado.empty()) {
                segmentosTexto.push_back(acumulado);
                segmentosColor.push_back("");
            }
        }
    }

    clearScreen();
    std::cout << "Presiona cualquier tecla para comenzar a revelar el texto...\n";
    GETCH();

    clearScreen();

    for (size_t i = 0; i < segmentosTexto.size(); ++i) {
        GETCH();
        if (!segmentosColor[i].empty()) {
            std::cout << segmentosColor[i] << segmentosTexto[i] << RESET;
        } else {
            std::cout << segmentosTexto[i];
        }
        std::cout.flush();
    }

    std::cout << "\n\n--- Fin del texto ---\n";
    std::cout << "Número total de espacios en el archivo: " << totalEspacios << '\n';
    std::cout << "Modo de revelado: " << modo << '\n';
    if (highlight) {
        std::cout << "Resaltado de sintaxis C++ activado.\n";
    }

    std::cout << "\nPresiona cualquier tecla para salir...\n";
    GETCH();

    return 0;
}