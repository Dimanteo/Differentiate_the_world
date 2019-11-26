#ifndef DIFFERENTIATOR_PARSER_H
#define DIFFERENTIATOR_PARSER_H

#include "Tree_t\Tree.h"
#include "MathObject.h"
#include "Function.h"
#define syntax_assert(condition, exp) if(!(condition)) {printf("syntax error: expected %c, got %c",exp, *str);abort();}

struct Parser {
    char* str;

    Tree<MathObject>* parseLine(const char* input);
    Tree<MathObject>* getG();
    Tree<MathObject>* getE();
    Tree<MathObject>* getT();
    Tree<MathObject>* getP();
    Tree<MathObject>* getN();
    Tree<MathObject>* makeVal(const char* string, Tree<MathObject>* val, Tree<MathObject>* val1);
};

Tree<MathObject>* Parser::parseLine(const char* input) {
    str = (char*)input;
    return getG();
}


Tree<MathObject>* Parser::makeVal(const char* string, Tree<MathObject>* val, Tree<MathObject>* val1) {
    Tree<MathObject>* ret_val = new Tree<MathObject>(MathObject(MathObject::OPERATION_TYPE, getFunctionCode(string)));
    ret_val->connectSubtree(LEFT_CHILD, val);
    ret_val->connectSubtree(RIGHT_CHILD, val1);
    return ret_val;
}

Tree<MathObject>* Parser::getG() {
    Tree<MathObject>* val = getE();
    syntax_assert(*str == '\0', '\0')
    return val;
}

Tree<MathObject>* Parser::getE() {
    Tree<MathObject>* val = getT();
    while ('+' == *str || *str == '-') {
        char op = *str;
        str++;
        Tree<MathObject>* val1 = getT();
        if (op == '+') {
            val = makeVal("+", val, val1);
        } else if (op == '-') {
            val = makeVal("-", val, val1);
        }
    }
    return val;
}

Tree<MathObject> *Parser::getT() {
    Tree<MathObject>* val = getP();
    while  (*str == '*' || *str == '/') {
        char op = *str;
        str++;
        Tree<MathObject>* val1 = getP();
        if (op == '*') {
            val = makeVal("*", val, val1);
        } else if (op == '/') {
            val = makeVal("/", val, val1);
        }
    }
    return val;
}

Tree<MathObject> *Parser::getP() {
    if (*str == '(') {
        str++;
        Tree<MathObject>* val = getE();
        syntax_assert(*str == ')', ')');
        str++;
        return val;
    } else {
        Tree<MathObject>* val = getN();
        return val;
    }
}

Tree<MathObject> *Parser::getN() {
    double value = 0;
    do {
        value = value * 10 + (double)(*str - '0');
        str++;
    } while ('0' <= *str && *str <= '9');
    return new Tree<MathObject>(MathObject(value));
}


#endif //DIFFERENTIATOR_PARSER_H
