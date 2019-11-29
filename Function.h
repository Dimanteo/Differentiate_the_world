#ifndef DIFFERENTIATOR_FUNCTION_H
#define DIFFERENTIATOR_FUNCTION_H

#include <mem.h>
#include <assert.h>
#include <cstdlib>
#include "MathObject.h"
#include "Tree_t\Tree.h"

struct Function {
    char* token;
    int priority;

    virtual ~Function() {}
    virtual char* texPrint(Tree<MathObject>* node, char* leftString, char* rightString) = 0;
    virtual double calculate(Tree<MathObject>* node) = 0;
};


//+ summation
struct Sum : public Function {
    Sum() {
        token = strdup("+");
        priority = 0;
    }
    ~Sum() {
        free(token);
    }

    char* texPrint(Tree<MathObject> *node, char* leftString, char* rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 2, sizeof(buffer[0]));
        sprintf(buffer, "%s+%s", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return node->getChild(LEFT_CHILD)->getValue().num + node->getChild(RIGHT_CHILD)->getValue().num;
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

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 2, sizeof(buffer[0]));
        sprintf(buffer, "%s-%s", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return node->getChild(LEFT_CHILD)->getValue().num - node->getChild(RIGHT_CHILD)->getValue().num;
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

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 8 + 1, sizeof(buffer[0]));
        sprintf(buffer, "%s \\cdot %s", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return node->getChild(LEFT_CHILD)->getValue().num * node->getChild(RIGHT_CHILD)->getValue().num;
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

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 10 + 1, sizeof(buffer[0]));
        sprintf(buffer, "\\dfrac{%s}{%s}", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return node->getChild(LEFT_CHILD)->getValue().num / node->getChild(RIGHT_CHILD)->getValue().num;
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

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 4, sizeof(buffer[0]));
        sprintf(buffer, "%s^{%s}", leftString, rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return pow(node->getChild(LEFT_CHILD)->getValue().num, node->getChild(RIGHT_CHILD)->getValue().num);
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

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 7, sizeof(buffer[0]));
        sprintf(buffer, "\\sin(%s)", rightString);
        return  buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return sin(node->getChild(RIGHT_CHILD)->getValue().num);
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

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 7, sizeof(buffer[0]));
        sprintf(buffer, "\\cos(%s)", rightString);
        return  buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return cos(node->getChild(RIGHT_CHILD)->getValue().num);
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

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 6 + 1, sizeof(buffer[0]));
        sprintf(buffer, "\\tan(%s)", rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return tan(node->getValue().num);
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

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 6 + 1, sizeof(buffer[0]));
        sprintf(buffer, "\\ctg(%s)", rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return cos(node->getValue().num) / sin(node->getValue().num);
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

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(rightString) + 6, sizeof(buffer[0]));
        sprintf(buffer, "\\ln(%s)", rightString);
        return  buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return log(node->getChild(RIGHT_CHILD)->getValue().num);
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
        new Ctg()
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

#endif //DIFFERENTIATOR_FUNCTION_H
