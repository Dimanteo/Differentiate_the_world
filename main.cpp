#include <string.h>
#include "Tree_t\Tree.cpp"
#include "My_Headers\txt_files.h"
#include "MathObject.h"
#include "Function.h"
#include "Parser.h"


class Differentiator {

public:

    const static size_t VARIABLES_MAX_COUNT = 100;
    char** variables;
    size_t variables_count = 0;
    int diffVarCode = 0;

    Tree<MathObject>* tree = nullptr;

    Differentiator();
    ~Differentiator();

    void parse(FILE* in);

    void dump(const char outFileName[], const char state[], const char message[], const char file[], const char function[], int line);

    char * texDump(char *buffer, Tree<MathObject>* node);

    Tree<MathObject>* diff(Tree<MathObject>* node);

    void setDiffVar(const char* var);

    Tree<MathObject> *getDiff(const char *diffVar);
};

template<>
void  Tree<MathObject>::valuePrint(FILE *file) {
    switch (value.type) {
        case MathObject::NUMBER_TYPE:
            fprintf(file, "Number(%d) %g", value.type, value.num);
            break;
        case MathObject::OPERATION_TYPE:
            fprintf(file, "Operation(%d) [%d] %s", value.type, value.code, FUNCTIONS[value.code]->token);
            break;
        case MathObject::VARIABLE_TYPE:
            fprintf(file, "Variable(%d) [%d]",value.type, value.code);
            break;
        case MathObject::POISON_TYPE:
            fprintf(file, "void");
            break;
    }
}

template<>
void Tree<MathObject>::valueDestruct() {
    switch (value.type) {
        case MathObject::NUMBER_TYPE:
            value.num = 0;
            break;
        case MathObject::OPERATION_TYPE:
            value.code = 0;
            break;
        case MathObject::VARIABLE_TYPE:
            value.code = 0;
            break;
    }
    value.type = MathObject::POISON_TYPE;
}

template<>
void Tree<MathObject>::genDot(Tree<MathObject> *node, FILE *file) {
#ifndef NDEBUG
    char color[8] = "";
    switch (node->getValue().type) {
        case MathObject::NUMBER_TYPE:
            strcpy(color, "#55B1D2");
            break;
        case MathObject::OPERATION_TYPE:
            strcpy(color, "#B1D255");
            break;
        case MathObject::VARIABLE_TYPE:
            strcpy(color, "#D255B1");
            break;
        case MathObject::POISON_TYPE:
            strcpy(color, "#FFFFFF");
    }
    fprintf(file, "T%p [shape = record, color = \"%s\", style =\"filled\", label = \" {value\\n", node, color);
    node->valuePrint(file);
    fprintf(file, " | this\\n%p | parent\\n%p  | {left\\n%p | right\\n%p}} \"];\n\t", node, node->getParent(), node->getChild(LEFT_CHILD), node->getChild(RIGHT_CHILD));
    for (int i = 0; i < NUMBER_OF_CHILDREN; ++i) {
        if (!node->childIsEmpty(i))
            fprintf(file, "T%p -> T%p[label = \"%s\"];", node, node->getChild(i), i ? "Right" : "Left");
    }
    fprintf(file, "\n\t");
#endif
}

int main() {
    FILE* file = fopen("Input.txt", "rb");
    Differentiator* laba_killer = new Differentiator();
    laba_killer->parse(file);
    fclose(file);

    FILE* log = fopen("../Debug/Diff.txt", "wb");
    fclose(log);
    FILE* latex = fopen("../Debug/Diff.tex", "wb");
    fclose(latex);
    laba_killer->dump("../Debug/Diff", OK_STATE, "SOURCE", __FILE__, __FUNCTION__, __LINE__);

    laba_killer->getDiff("x");

    log = fopen("../Debug/Diff_Result.txt", "wb");
    fclose(log);
    latex = fopen("../Debug/Diff_Result.tex", "wb");
    fclose(latex);
    laba_killer->dump("../Debug/Diff_Result", OK_STATE, "RESULT", __FILE__, __FUNCTION__, __LINE__);

    for (int i = 0; i < FUNCTIONS_COUNT; ++i) {
        delete(FUNCTIONS[i]);
    }
    delete(laba_killer);
    return 0;
}



Differentiator::Differentiator() {
    variables = (char**)calloc(VARIABLES_MAX_COUNT, sizeof(variables[0]));
}

Differentiator::~Differentiator() {
    delete tree;
    for (int i = 0; i < variables_count; ++i) {
        free(variables[i]);
    }
    free(variables);
    variables = nullptr;
    variables_count = 0;
}

void Differentiator::parse(FILE *in) {
    size_t buffer_size = 0;
    char* buffer = read_file_to_buffer_alloc(in, "rb", &buffer_size);
    Parser parser = Parser();
    tree = parser.parseLine(buffer, variables);
    variables_count = parser.getVariablesCount();
    free(buffer);
}

void Differentiator::dump(const char outFileName[], const char state[], const char message[], const char file[], const char function[], int line) {
    Tree<MathObject>** seq = tree->allocTree();
    tree->inorder(seq);
    size_t outFileLength = strlen(outFileName);
    char* extensionName = (char*)calloc(outFileLength + 5, sizeof(extensionName[0]));
    strcpy(extensionName, outFileName);
    strcat(extensionName, ".txt");
    FILE* log = fopen(extensionName, "ab");

    fprintf(log, "\nBEGIN <Differentiator dump>\n{\n"
                 "\tVariables list\n"
                 "\t{\n");
    for (int i = 0; i < variables_count; ++i) {
        fprintf(log, "\t\t[%d] = %s\n", i, variables[i]);
    }
    fprintf(log, "\t}\n");
    tree->treeDump(log, state, message,file, function, line, seq);
    fprintf(log, "END </Differentiator dump>\n");
    fclose(log);

    extensionName[outFileLength] = '\0';
    strcat(extensionName, ".png");
    tree->graphDump(extensionName, seq);

    extensionName[outFileLength] = '\0';
    strcat(extensionName, ".tex");
    char* buffer = (char*)calloc(1, sizeof(buffer[0]));
    buffer = texDump(buffer, tree);
    FILE* latex = fopen(extensionName, "ab");
    fprintf(latex, "\\documentclass[a4paper,12pt]{article}\n"
                   "\\usepackage[T2A]{fontenc}\n"
                   "\\usepackage[utf8]{inputenc}\n"
                   "\\usepackage[english,russian]{babel}\n"
                   "\\usepackage{amsmath,amsfonts,amssymb,amsthm,mathtools} \n"
                   "\\usepackage{float}\n"
                   "\\usepackage{wasysym}\n"
                   "\\begin{document}\n");
    fprintf(latex, "\\begin{equation}\n");
    fwrite(buffer, sizeof(buffer[0]), strlen(buffer), latex);
    fprintf(latex, "\n\\end{equation}\n"
                   "\\end{document}");
    fclose(latex);

    free(extensionName);
}

char * Differentiator::texDump(char *buffer, Tree<MathObject>* node) {
    assert(node);
    switch (node->getValue().type) {
        case MathObject::NUMBER_TYPE: {
            char num_string[MathObject::MAX_LENGTH] = "";
            sprintf(num_string, "%g", node->getValue().num);
            buffer = (char*)realloc(buffer, strlen(buffer) + strlen(num_string) + 1);
            assert(buffer);
            strcat(buffer, num_string);
            break;
        }
        case MathObject::OPERATION_TYPE: {
            char *leftString = (char*)calloc(1, sizeof(leftString[0]));
            if (!node->childIsEmpty(LEFT_CHILD)) {
                leftString = texDump(leftString, node->getChild(LEFT_CHILD));
            }
            char *rightString = (char*)calloc(1, sizeof(rightString[0]));
            if (!node->childIsEmpty(RIGHT_CHILD)) {
                rightString = texDump(rightString, node->getChild(RIGHT_CHILD));
            }
            char *output = FUNCTIONS[node->getValue().code]->texPrint(node, leftString, rightString);
            if (!node->isRoot() && !node->getParent()->childIsEmpty(LEFT_CHILD) && FUNCTIONS[node->getValue().code]->priority < FUNCTIONS[node->getParent()->getValue().code]->priority) {
                buffer = (char*)realloc(buffer, strlen(buffer) + strlen(output) + 3);
                strcat(buffer, "(");
                strcat(buffer, output);
                strcat(buffer, ")");
            } else {
                buffer = (char*)realloc(buffer, strlen(buffer) + strlen(output) + 1);
                strcat(buffer, output);
            }
            free(output);
            free(rightString);
            free(leftString);
            break;
        }
        case MathObject::VARIABLE_TYPE: {
            buffer = (char*)realloc(buffer, strlen(buffer) + strlen(variables[node->getValue().code]) + 1);
            strcat(buffer, variables[node->getValue().code]);
            break;
        }
    }
    return buffer;
}

void Differentiator::setDiffVar(const char *var) {
    for (int i = 0; i < variables_count; ++i) {
        if (strcmp(variables[i], var) == 0) {
            diffVarCode = i;
            break;
        }
    }
}

Tree<MathObject> *Differentiator::getDiff(const char *diffVar) {
    setDiffVar(diffVar);
    Tree<MathObject>* diffTree = diff(tree);
    delete(tree);
    tree = diffTree;
}

Tree<MathObject>* copySubtree(Tree<MathObject>* node) {
    Tree<MathObject>* newNode = new Tree<MathObject>(node->getValue());
    if (!node->childIsEmpty(LEFT_CHILD)) {
        newNode->connectSubtree(LEFT_CHILD, copySubtree(node->getChild(LEFT_CHILD)));
    }
    if (!node->childIsEmpty(RIGHT_CHILD)) {
        newNode->connectSubtree(RIGHT_CHILD, copySubtree(node->getChild(RIGHT_CHILD)));
    }
    return newNode;
}

//DSL
#define dL diff(node->getChild(LEFT_CHILD))
#define dR diff(node->getChild(RIGHT_CHILD))
#define L copySubtree(node->getChild(LEFT_CHILD))
#define R copySubtree(node->getChild(RIGHT_CHILD))
#define it_is(tok) strcmp(FUNCTIONS[node->getValue().code]->token, #tok) == 0
#define newOperation(op) new Tree<MathObject>(MathObject(MathObject::OPERATION_TYPE, getFunctionCode(#op)))

#define overrideBinaryOperator(op) Tree<MathObject>* operator op (Tree<MathObject>& l, Tree<MathObject>& r) {\
                                        Tree<MathObject>* res = newOperation(op);\
                                        res->connectSubtree(LEFT_CHILD , &l);\
                                        res->connectSubtree(RIGHT_CHILD, &r);\
                                        return res;\
                                    }

overrideBinaryOperator(+)

overrideBinaryOperator(-)

overrideBinaryOperator(*)

overrideBinaryOperator(/)

overrideBinaryOperator(^)

#define overrideOperatorNumRight(op) Tree<MathObject>* operator op (Tree<MathObject>& l, double num) {\
                                        Tree<MathObject>* res = newOperation(op);\
                                        res->connectSubtree(LEFT_CHILD , &l);\
                                        res->connectSubtree(RIGHT_CHILD, new Tree<MathObject>(MathObject(num)));\
                                        return res;\
                                    }

overrideOperatorNumRight(^)
overrideOperatorNumRight(*)

Tree<MathObject>* Differentiator::diff(Tree<MathObject>* node) {
    switch (node->getValue().type) {
        case MathObject::NUMBER_TYPE: {
            return new Tree<MathObject>(MathObject(0));
        }
        case MathObject::VARIABLE_TYPE: {
            if (node->getValue().code == diffVarCode) {
                return new Tree<MathObject>(MathObject(1));
            } else {
                return new Tree<MathObject>(MathObject(0));
            }
        }
        case MathObject::OPERATION_TYPE: {
            if (it_is(+)) {
                return *dL + *dR;
            }
            if (it_is(-)) {
                return *dL - *dR;
            }
            if (it_is(*)) {
                return *(*dL * *R) + *(*L * *dR);
            }
            if (it_is(/)) {
                return *(*(*dL * *R) - *(*L * *dR)) / *(*R ^ 2);
            }
            if (it_is(sin)) {
                Tree<MathObject>* res = newOperation(cos);
                res->connectSubtree(RIGHT_CHILD, copySubtree(node->getChild(RIGHT_CHILD)));
                return *res * *dR;
            }
            if (it_is(cos)) {
                Tree<MathObject>* res = newOperation(sin);
                res->connectSubtree(RIGHT_CHILD, copySubtree(node->getChild(RIGHT_CHILD)));
                return *(*res * (-1)) * *dR;
            }
            break;
        }
    }
}

#undef dL
#undef dR
#undef L
#undef R
#undef overrideOperatorNumRight
#undef overrideBinaryOperator
#undef it_is
#undef newOperation
