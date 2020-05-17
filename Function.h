#ifndef DIFFERENTIATOR_FUNCTION_H
#define DIFFERENTIATOR_FUNCTION_H

#include <assert.h>
#include <cstdlib>
#include "MathObject.h"
#include "Tree_t/Tree.h"

struct Function {
    char* token;
    int priority;

    virtual ~Function() {}
    virtual char* texPrint(char* leftString, char* rightString) = 0;
    virtual double calculate(Tree<MathObject>* node) = 0;
};

#define LEFT_IS_NUMBER node->getChild(LEFT_CHILD)->getValue().type == MathObject::NUMBER_TYPE
#define RIGHT_IS_NUMBER node->getChild(RIGHT_CHILD)->getValue().type == MathObject::NUMBER_TYPE

//+ summation
struct Sum : public Function {
    Sum() {
        token = strdup("+");
        priority = 0;
    }
    ~Sum() {
        free(token);
    }

    char* texPrint(char* leftString, char* rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 2, sizeof(buffer[0]));
        sprintf(buffer, "%s+%s", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (LEFT_IS_NUMBER && RIGHT_IS_NUMBER)
            return node->getChild(LEFT_CHILD)->getValue().num + node->getChild(RIGHT_CHILD)->getValue().num;
        else
            return NAN;
    }
};

//- subtraction
struct Sub : public Function {
    Sub() {
        token = strdup("-");
        priority = 0;
    }
    ~Sub() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 2, sizeof(buffer[0]));
        sprintf(buffer, "%s-%s", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (LEFT_IS_NUMBER && RIGHT_IS_NUMBER)
            return node->getChild(LEFT_CHILD)->getValue().num - node->getChild(RIGHT_CHILD)->getValue().num;
        else
            return NAN;
    }
};

//* multiplication
struct Mul : public Function {
    Mul() {
        token = strdup("*");
        priority = 1;
    }
    ~Mul() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 8 + 1, sizeof(buffer[0]));
        sprintf(buffer, "%s \\cdot %s", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (LEFT_IS_NUMBER && RIGHT_IS_NUMBER)
            return node->getChild(LEFT_CHILD)->getValue().num * node->getChild(RIGHT_CHILD)->getValue().num;
        else
            return NAN;
    }
};

//Division
struct Div : Function {
    Div() {
        token = strdup("/");
        priority = 1;
    }
    ~Div() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 10 + 1, sizeof(buffer[0]));
        sprintf(buffer, "\\dfrac{%s}{%s}", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (LEFT_IS_NUMBER && RIGHT_IS_NUMBER)
            return node->getChild(LEFT_CHILD)->getValue().num / node->getChild(RIGHT_CHILD)->getValue().num;
        else
            return NAN;
    }
};

struct Pow : Function {
    Pow() {
        token = strdup("^");
        priority = 2;
    }
    ~Pow() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 6, sizeof(buffer[0]));
        sprintf(buffer, "{%s}^{%s}", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (LEFT_IS_NUMBER && RIGHT_IS_NUMBER)
            return pow(node->getChild(LEFT_CHILD)->getValue().num, node->getChild(RIGHT_CHILD)->getValue().num);
        else
            return NAN;
    }
};

struct Sin : Function {
    Sin() {
        token = strdup("sin");
        priority = 2;
    }
    ~Sin() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 7, sizeof(buffer[0]));
        sprintf(buffer, "\\sin(%s)", rightString);
        return  buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (RIGHT_IS_NUMBER)
            return sin(node->getChild(RIGHT_CHILD)->getValue().num);
        else
            return NAN;
    }
};

struct Cos : Function {
    Cos() {
        token = strdup("cos");
        priority = 2;
    }
    ~Cos() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 7, sizeof(buffer[0]));
        sprintf(buffer, "\\cos(%s)", rightString);
        return  buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (RIGHT_IS_NUMBER)
            return cos(node->getChild(RIGHT_CHILD)->getValue().num);
        else
            return NAN;
    }
};

struct Tan : Function {
    Tan() {
        token = strdup("tg");
        priority = 2;
    }
    ~Tan() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 6 + 1, sizeof(buffer[0]));
        sprintf(buffer, "\\tan(%s)", rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (RIGHT_IS_NUMBER)
            return tan(node->getChild(RIGHT_CHILD)->getValue().num);
        else
            return NAN;
    }
};

struct Ctg : Function {
    Ctg() {
        token = strdup("ctg");
        priority = 2;
    }
    ~Ctg() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 6 + 1, sizeof(buffer[0]));
        sprintf(buffer, "\\ctg(%s)", rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (RIGHT_IS_NUMBER)
            return cos(node->getChild(RIGHT_CHILD)->getValue().num) / sin(node->getChild(RIGHT_CHILD)->getValue().num);
        else
            return NAN;
    }
};

struct Log_e : Function {
    Log_e() {
        token = strdup("ln");
        priority = 2;
    }
    ~Log_e() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 6, sizeof(buffer[0]));
        sprintf(buffer, "\\ln(%s)", rightString);
        return  buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (RIGHT_IS_NUMBER)
            return log(node->getChild(RIGHT_CHILD)->getValue().num);
        else
            return NAN;
    }
};

struct Minus : Function {
    Minus (){
        token = strdup("--");
        priority = -1;
    }
    ~Minus() {
        free(token);
    }

    char *texPrint(char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 2, sizeof(buffer[0]));
        sprintf(buffer, "-%s", rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        if (RIGHT_IS_NUMBER)
            if (node->getChild(RIGHT_CHILD)->getValue().num != 0)
                return -(node->getChild(RIGHT_CHILD)->getValue().num);
            else
                return 0;
        else
            return NAN;
    }
};

Function* FUNCTIONS[] = {
        new Sum(),
        new Sub(),
        new Mul(),
        new Div(),
        new Pow(),
        new Sin(),
        new Cos(),
        new Log_e(),
        new Tan(),
        new Ctg(),
        new Minus()
};

const int NO_FUNCTION_CODE = -1;
const size_t FUNCTIONS_COUNT = sizeof(FUNCTIONS) / sizeof(FUNCTIONS[0]);

int getFunctionCode(const char* str) {
    for (int i = 0; i < FUNCTIONS_COUNT; ++i) {
        if (strcmp(FUNCTIONS[i]->token, str) == 0) {
            return i;
        }
    }
    return NO_FUNCTION_CODE;
}

#undef LEFT_IS_NUMBER
#undef RIGHT_IS_NUMBER

#endif //DIFFERENTIATOR_FUNCTION_H
