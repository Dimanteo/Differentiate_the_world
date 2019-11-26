#ifndef DIFFERENTIATOR_FUNCTION_H
#define DIFFERENTIATOR_FUNCTION_H

#include <mem.h>
#include <assert.h>
#include <cstdlib>
#include "MathObject.h"
#include "Tree_t\Tree.h"

struct Function {
    char* token;

    virtual char* texPrint(Tree<MathObject>* node, char* leftString, char* rightString) = 0;
    virtual double calculate(Tree<MathObject>* node) = 0;
    virtual void diff(Tree<MathObject> function, Tree<MathObject> derivative) = 0;
};


//+ summation
struct Sum : public Function {

    Sum() {
        token = strdup("+");
    }
    ~Sum() {
        free(token);
    }

    char* texPrint(Tree<MathObject> *node, char* leftString, char* rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 2, sizeof(buffer[0]));
        sprintf(buffer, "%s+%s", leftString, rightString);
        free(leftString);
        free(rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return node->getChild(LEFT_CHILD)->getValue().num + node->getChild(RIGHT_CHILD)->getValue().num;
    }

    void diff(Tree<MathObject> function, Tree<MathObject> derivative) override {

    }
};

//- subtraction
struct Sub : public Function {
    Sub() {
        token = strdup("-");
    }
    ~Sub() {
        free(token);
    }

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 2, sizeof(buffer[0]));
        sprintf(buffer, "%s-%s", leftString, rightString);
        free(leftString);
        free(rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return node->getChild(LEFT_CHILD)->getValue().num - node->getChild(RIGHT_CHILD)->getValue().num;
    }

    void diff(Tree<MathObject> function, Tree<MathObject> derivative) override {

    }
};

//* multiplication
struct Mul : public Function {
    Mul() {
        token = strdup("*");
    }
    ~Mul() {
        free(token);
    }

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 5 + 1, sizeof(buffer[0]));
        sprintf(buffer, "%s \\cdot %s", leftString, rightString);
        free(leftString);
        free(rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return node->getChild(LEFT_CHILD)->getValue().num * node->getChild(RIGHT_CHILD)->getValue().num;
    }

    void diff(Tree<MathObject> function, Tree<MathObject> derivative) override {

    }
};

//Division
struct Div : Function {
    Div() {
        token = strdup("/");
    }
    ~Div() {
        free(token);
    }

    char *texPrint(Tree<MathObject> *node, char *leftString, char *rightString) override {
        char* buffer = (char*)calloc(strlen(leftString) + strlen(rightString) + 10 + 1, sizeof(buffer[0]));
        sprintf(buffer, "\\dfrac{%s}{%s}", leftString, rightString);
        free(leftString);
        free(rightString);
        return buffer;
    }

    double calculate(Tree<MathObject> *node) override {
        return node->getChild(LEFT_CHILD)->getValue().num / node->getChild(RIGHT_CHILD)->getValue().num;
    }

    void diff(Tree<MathObject> function, Tree<MathObject> derivative) override {

    }
};

Function* FUNCTIONS[] = {
        new Sum(),
        new Sub(),
        new Mul(),
        new Div()
};
const size_t FUNCTIONS_COUNT = sizeof(FUNCTIONS) / sizeof(Function*);
int getFunctionCode(const char* str) {
    for (int i = 0; i < FUNCTIONS_COUNT; ++i) {
        if (strcmp(FUNCTIONS[i]->token, str) == 0) {
            return i;
        }
    }
    return -1;
}

#endif //DIFFERENTIATOR_FUNCTION_H
