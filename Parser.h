#ifndef DIFFERENTIATOR_PARSER_H
#define DIFFERENTIATOR_PARSER_H

#include "Tree_t\Tree.h"
#include "MathObject.h"
#include "Function.h"
#define syntax_assert(condition, exp) if(!(condition)) {fprintf(stderr, "syntax error: expected %c, got %c\n",exp, *str);assert(condition);}

struct Parser {
    char* str;
    size_t variables_count = 0;
    char** variables;
    Tree<MathObject>* parseLine(const char* input, char** var_storage);
    size_t getVariablesCount();
    Tree<MathObject>* makeVal(const char* string, Tree<MathObject>* val, Tree<MathObject>* val1);
    void skipSpaces();


    Tree<MathObject>* getG();
    Tree<MathObject>* getE();
    Tree<MathObject>* getSign();
    Tree<MathObject>* getAbs();
    Tree<MathObject>* getT();
    Tree<MathObject>* getPow();
    Tree<MathObject>* getP();
    Tree<MathObject>* getN();
    Tree<MathObject>* getId();
    Tree<MathObject>* getFun();
};

Tree<MathObject>* Parser::parseLine(const char* input, char** var_storage) {
    str = (char*)input;
    variables = var_storage;
    return getG();
}


Tree<MathObject>* Parser::makeVal(const char* string, Tree<MathObject>* val, Tree<MathObject>* val1) {
    Tree<MathObject>* ret_val = new Tree<MathObject>(MathObject(MathObject::OPERATION_TYPE, getFunctionCode(string)));
    ret_val->connectSubtree(LEFT_CHILD, val);
    ret_val->connectSubtree(RIGHT_CHILD, val1);
    return ret_val;
}

Tree<MathObject>* Parser::getG() {
    skipSpaces();
    Tree<MathObject>* val = getE();
    skipSpaces();
    syntax_assert(*str == '\0', '\0')
    return val;
}

Tree<MathObject>* Parser::getE() {
    skipSpaces();
    return getSign();
}

Tree<MathObject>* Parser::getSign() {
    skipSpaces();
    Tree<MathObject>* minus = nullptr;
    if (*str == '-') {
        minus = new Tree<MathObject>(MathObject(MathObject::OPERATION_TYPE, getFunctionCode("--")));
        str++;
        skipSpaces();
    }
    Tree<MathObject>* val = getAbs();
    if (minus != nullptr) {
        minus->connectSubtree(RIGHT_CHILD, val);
        return minus;
    } else {
        return val;
    }
}

Tree<MathObject>* Parser::getAbs() {
    skipSpaces();
    Tree<MathObject>* val = getT();
    skipSpaces();
    while ('+' == *str || *str == '-') {
        char op = *str;
        str++;
        skipSpaces();
        Tree<MathObject>* val1 = getT();
        skipSpaces();
        if (op == '+') {
            val = makeVal("+", val, val1);
        } else if (op == '-') {
            val = makeVal("-", val, val1);
        }
    }
    return val;
}

Tree<MathObject> *Parser::getT() {
    skipSpaces();
    Tree<MathObject>* val = getPow();
    skipSpaces();
    while  (*str == '*' || *str == '/') {
        char op = *str;
        str++;
        skipSpaces();
        Tree<MathObject>* val1 = getPow();
        skipSpaces();
        if (op == '*') {
            val = makeVal("*", val, val1);
        } else if (op == '/') {
            val = makeVal("/", val, val1);
        }
    }
    return val;
}

Tree<MathObject> *Parser::getPow() {
    skipSpaces();
    Tree<MathObject>* val = getP();
    skipSpaces();
    if (*str == '^') {
        str++;
        skipSpaces();
        Tree<MathObject>* val1 = getP();
        skipSpaces();
        val = makeVal("^", val, val1);
    }
    return val;
}

Tree<MathObject> *Parser::getP() {
    skipSpaces();
    Tree<MathObject>* minus = nullptr;
    if (*str == '(') {
        str++;
        skipSpaces();
        Tree<MathObject>* val = getE();
        skipSpaces();
        syntax_assert(*str == ')', ')')
        str++;
        skipSpaces();
        return val;
    } else if ('0' <= *str && *str <= '9') {
        Tree<MathObject>* val = getN();
        skipSpaces();
        return val;
    } else {
        Tree<MathObject>* val =getFun();
        skipSpaces();
        return val;
    }
}

Tree<MathObject> *Parser::getN() {
    double value = 0;
    do {
        value = value * 10 + (double)(*str - '0');
        str++;
    } while ('0' <= *str && *str <= '9');
    if (*str == '.') {
        str++;
        int i = 1;
        while ('0' <= *str && *str <= '9') {
            value = value + (double) (*str - '0') / (i * 10);
            i *= 10;
            str++;
        }
    }
    return new Tree<MathObject>(MathObject(value));
}

Tree<MathObject> *Parser::getFun() {
    skipSpaces();
    char* start = str;
    while ('a' <= *str && *str <= 'z') {
        str++;
    }
    char* name = (char*)calloc(str - start + 1, sizeof(name[0]));
    strncpy(name, start, str - start);
    int code = getFunctionCode(name);
    free(name);
    skipSpaces();
    if (code == NO_FUNCTION_CODE || *str != '(') {
        str = start;
        Tree<MathObject>* val = getId();
        return val;
    }
    str++;
    skipSpaces();
    Tree<MathObject>* val = getE();
    skipSpaces();
    syntax_assert(*str == ')', ')')
    str++;
    skipSpaces();
    Tree<MathObject>* val1 = new Tree<MathObject>(MathObject(MathObject::OPERATION_TYPE, code));
    val1->connectSubtree(RIGHT_CHILD, val);
    return val1;
}

Tree<MathObject> *Parser::getId() {
    skipSpaces();
    char* start = str;
    if (isalpha(*str) || *str == '_' || *str == '\\' || *str == '{' || *str == '}') {
        str++;
    }
    while (isalnum(*str) || *str == '_' || *str == '\\' || *str == '{' || *str == '}') {
        str++;
    }
    syntax_assert(str != start, 'V')
    char* name = (char *)calloc(str - start + 1, sizeof(name[0]));
    strncpy(name, start, str - start);
    int code = variables_count;
    for (int i = 0; i < variables_count; ++i) {
        if (strcmp(variables[i], name) == 0) {
            code = i;
            break;
        }
    }
    if (code == variables_count) {
        variables[code] = name;
        variables_count++;
    }
    Tree<MathObject>* val = new Tree<MathObject>(MathObject(MathObject::VARIABLE_TYPE, code));
    return val;
}

size_t Parser::getVariablesCount() {
    return variables_count;
}

inline void Parser::skipSpaces() {
    while (isspace(*str)) {
        str++;
    }
}

#endif //DIFFERENTIATOR_PARSER_H
