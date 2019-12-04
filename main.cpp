#include <string.h>
#include "Tree_t\Tree.cpp"
#include "My_Headers\txt_files.h"
#include "MathObject.h"
#include "Function.h"
#include "Parser.h"
#include <math.h>
#include <windows.h>

#include "Ochevidno.h"

#define VERIFY_CONTEXT __FILE__, __PRETTY_FUNCTION__, __LINE__

class Differentiator {

public:

    const static size_t VARIABLES_MAX_COUNT = 100;
    char** variables;
    size_t variables_count = 0;
    int diffVarCode = -1;

    Tree<MathObject>* tree = nullptr;

    Differentiator();
    ~Differentiator();

    void parse(FILE* in);

    void dump(const char outFileName[], const char state[], const char message[], const char file[], const char function[], int line);

    char * texDump(char *buffer, Tree<MathObject>* node);

    Tree<MathObject>* diff(Tree<MathObject>* node);

    void setDiffVar(const char* var);

    Tree<MathObject> *getDiff(const char *diffVar, const char *filename, int order);

    Tree<MathObject>* copySubtree(Tree<MathObject>* node);

    void replaceBy(size_t child, Tree<MathObject>* node);

    void makeNumber(double num, Tree<MathObject>* node);

    bool isVariable(Tree<MathObject>* node);

    bool optimizationZero(Tree<MathObject> *node);

    bool optimizationOne(Tree<MathObject> *node);

    bool optimizationCalc(Tree<MathObject> *node);

    void optimization();

    FILE* texPreprocess(const char* filename, const char* diffVar, int order);

//DSL functions. Defined in #define overrideUnaryOperator
    Tree<MathObject>* cos (Tree<MathObject>* node);

    Tree<MathObject>* sin (Tree<MathObject>* node);

    Tree<MathObject>* ln (Tree<MathObject>* node);
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
    SetConsoleOutputCP(CP_UTF8);
    FILE* file = fopen("Input.txt", "rb");
    Differentiator* laba_killer = new Differentiator();
    laba_killer->parse(file);
    fclose(file);

    FILE* log = fopen("../Debug/Diff.txt", "wb");
    fclose(log);

    char var[20] = "";
    int order = 0;
    printf("Введите переменную дифференцирования и порядок производной\n");
    scanf("%s %d", var, &order);

    laba_killer->getDiff(var, "../Debug/LATEX.tex", order);

    system("pdflatex ../Debug/LATEX.tex");
    system("LATEX.pdf");

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
#ifndef NDEBUG
    Tree<MathObject>** seq = tree->allocTree();
    tree->inorder(seq);
    FILE* log = fopen(outFileName, "ab");

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
#endif
}

char* prefixMinusTex(Tree<MathObject>* node, char* rightString) {
    if( node->getValue().code == getFunctionCode("--") &&
        (node->getChild(RIGHT_CHILD)->getValue().code == getFunctionCode("+") ||
         node->getChild(RIGHT_CHILD)->getValue().code == getFunctionCode("-"))) {
        char* surrounded = (char*)calloc(strlen(rightString) + 3, sizeof(surrounded[0]));
        strcpy(surrounded, "(");
        strcat(surrounded, rightString);
        strcat(surrounded, ")");
        free(rightString);
        rightString = surrounded;
    }
    return rightString;
}

char * Differentiator::texDump(char *buffer, Tree<MathObject>* node) {
    assert(node);
    switch (node->getValue().type) {
        case MathObject::NUMBER_TYPE: {
            char num_string[MathObject::MAX_LENGTH] = "";
            if  (node->getValue().num < 0) {
                sprintf(num_string, "(%g)", node->getValue().num);
            } else {
                sprintf(num_string, "%g", node->getValue().num);
            }
            buffer = (char*)realloc(buffer, strlen(buffer) + strlen(num_string) + 1);
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
                rightString = prefixMinusTex(node, rightString);
            }
            char *output = FUNCTIONS[node->getValue().code]->texPrint(leftString, rightString);
            if (!node->isRoot() && !node->getParent()->childIsEmpty(LEFT_CHILD) &&
            FUNCTIONS[node->getValue().code]->priority < FUNCTIONS[node->getParent()->getValue().code]->priority) {
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
            return;
        }
    }
    variables[variables_count] = strdup(var);
    diffVarCode = variables_count++;
}

int counter = 0;

void drawGraph(const char filename[], int number, Tree<MathObject>* tree) {
    char Name[30] = "";
    sprintf(Name, "%s[%d].png", filename, number);
    Tree<MathObject>** sequ = tree->allocTree();
    tree->inorder(sequ);
    tree->graphDump(Name, sequ);
    free(sequ);
    counter++;
}

void printMemes(FILE* file, int complexity) {
    if (complexity >= 10) {
        int i = rand() % MEMS_SIZE;
        fprintf(file, "%s\n", MEMES[i]);
    }
}

FILE*  Differentiator::texPreprocess(const char* filename, const char* diffVar, int order) {
    FILE* latex = fopen(filename, "wb");
    fprintf(latex, "\\documentclass[a4paper,12pt]{article}\n"
                   "\\usepackage[T2A]{fontenc}\n"
                   "\\usepackage[utf8]{inputenc}\n"
                   "\\usepackage[english,russian]{babel}\n"
                   "\\usepackage{amsmath,amsfonts,amssymb,amsthm,mathtools} \n"
                   "\\usepackage{float}\n"
                   "\\usepackage{wasysym}\n"
                   "\\begin{document}\n"
                   "Рассмотрим следующее выражение. Найдем производную %d-го порядка по $%s$\n", order, diffVar);

    fprintf(latex, "$$");
    char* equation = (char*)calloc(1, sizeof(equation[0]));
    equation = texDump(equation, tree);
    fwrite(equation, sizeof(equation[0]), strlen(equation), latex);
    free(equation);
    fprintf(latex, "$$\n");
    return latex;
}

Tree<MathObject> *Differentiator::getDiff(const char *diffVar, const char *filename, int order) {
    setDiffVar(diffVar);
    FILE* latex = texPreprocess(filename, diffVar, order);

#ifdef DIFF_GRAPH
    drawGraph("../Debug/Diff", counter, tree);
#endif

    for (int i = 0; i < order; ++i) {
        Tree<MathObject>* diffTree = diff(tree);
        int complexity = abs((int)tree->getSize() - (int)diffTree->getSize());
        delete(tree);
        tree = diffTree;

        optimization();

        printMemes(latex, complexity);

        fprintf(latex, "\\begin{equation}\n");
        char* buffer = (char*)calloc(1, sizeof(buffer[0]));
        buffer = texDump(buffer, tree);
        fwrite(buffer, sizeof(buffer[0]), strlen(buffer), latex);
        free(buffer);
        fprintf(latex, "\n\\end{equation}\n");

#ifdef DIFF_GRAPH
        drawGraph("../Debug/Diff", counter, tree);
#endif
    }
    fprintf(latex, "Дальнейшие рассуждения, ввиду очевидности, предоставляются читателю в качестве упражнения.\n"
                   "\\end{document}");
    fclose(latex);
}

Tree<MathObject>* Differentiator::copySubtree(Tree<MathObject>* node) {
    assert(node);
    Tree<MathObject>* newNode = new Tree<MathObject>(node->getValue());
    if (!node->childIsEmpty(LEFT_CHILD)) {
        newNode->connectSubtree(LEFT_CHILD, copySubtree(node->getChild(LEFT_CHILD)));
    }
    if (!node->childIsEmpty(RIGHT_CHILD)) {
        newNode->connectSubtree(RIGHT_CHILD, copySubtree(node->getChild(RIGHT_CHILD)));
    }
    return newNode;
}

bool Differentiator::isVariable(Tree<MathObject> *node) {
    if (node->getValue().type == MathObject::VARIABLE_TYPE && node->getValue().code == diffVarCode) {
        return true;
    }
    if (!node->childIsEmpty(LEFT_CHILD)) {
        return isVariable(node->getChild(LEFT_CHILD));
    }
    if (!node->childIsEmpty(RIGHT_CHILD)) {
        return isVariable(node->getChild(RIGHT_CHILD));
    }
    return false;
}

void Differentiator::replaceBy(size_t child, Tree<MathObject>* node) {
    node->setValue(node->getChild(child)->getValue());
    size_t not_child = (child == RIGHT_CHILD) ? LEFT_CHILD : RIGHT_CHILD;
    Tree<MathObject>* subtreeCopy = nullptr;

    node->removeSubTree(not_child);
    if (!node->getChild(child)->childIsEmpty(not_child)) {
        subtreeCopy = copySubtree(node->getChild(child)->getChild(not_child));
        node->connectSubtree(not_child, subtreeCopy);
    }
    if (!node->getChild(child)->childIsEmpty(child)) {
        subtreeCopy = copySubtree(node->getChild(child)->getChild(child));
        node->removeSubTree(child);
        node->connectSubtree(child, subtreeCopy);
    } else {
        node->removeSubTree(child);
    }
}

void Differentiator::makeNumber(double num, Tree<MathObject> *node) {
    node->removeSubTree(LEFT_CHILD );
    node->removeSubTree(RIGHT_CHILD);
    node->setValue(MathObject(num));
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
                                    }\
                                    Tree<MathObject>* operator op (Tree<MathObject>& l, double num) {\
                                        Tree<MathObject>* res = newOperation(op);\
                                        res->connectSubtree(LEFT_CHILD , &l);\
                                        res->connectSubtree(RIGHT_CHILD, new Tree<MathObject>(MathObject(num)));\
                                        return res;\
                                    }\
                                    Tree<MathObject>* operator op (double num, Tree<MathObject>& r) {\
                                        Tree<MathObject>* res = newOperation(op);\
                                        res->connectSubtree(RIGHT_CHILD , &r);\
                                        res->connectSubtree(LEFT_CHILD, new Tree<MathObject>(MathObject(num)));\
                                        return res;\
                                    }

overrideBinaryOperator(+)

overrideBinaryOperator(-)

overrideBinaryOperator(*)

overrideBinaryOperator(/)

overrideBinaryOperator(^)

Tree<MathObject>* operator - (Tree<MathObject>& node) {
    Tree<MathObject>* minus = newOperation(--);
    minus->connectSubtree(RIGHT_CHILD, &node);
    return minus;
}

#define overrideUnaryOperator(op) Tree<MathObject>* Differentiator::op (Tree<MathObject>* node) {\
                                      Tree<MathObject>* res = newOperation(op);\
                                      res->connectSubtree(RIGHT_CHILD, node);\
                                      return res;\
                                  }

overrideUnaryOperator(cos)

overrideUnaryOperator(sin)

overrideUnaryOperator(ln)

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
                return *cos(R) * *dR;
            }
            if (it_is(cos)) {
                return *(-*sin(R)) * *dR;
            }
            if (it_is(tg)) {
                return *(1 / *(*cos(R) ^ 2)) * *dR;
            }
            if (it_is(ctg)) {
                return -*(1 / *(*sin(R) ^ 2));
            }
            if (it_is(^)) { //L^R
                bool baseIsVar  = isVariable(node->getChild(LEFT_CHILD));
                bool powerIsVar = isVariable(node->getChild(RIGHT_CHILD));
                if (baseIsVar && powerIsVar) {
                    return *(*L ^ *(*R - 1)) * *(*(*R * *dL) + *(*(*L * *ln(L) )* *dR));
                }
                if (baseIsVar) {
                    return *(*R * *(*L ^ *(*R - 1))) * *dL;
                }
                if (powerIsVar) {
                    return *(*(*L ^ *R) * *ln(L)) * *dR;
                }
                //if (!baseIsVar && !powerIsVar)
                return new Tree<MathObject>(MathObject(0));
            }
            if (it_is(ln)) {
                return *(1 / *R) * *dR;
            }
            if (it_is(--)) {
                Tree<MathObject>* minus = newOperation(--);
                minus->connectSubtree(RIGHT_CHILD, dR);
                return minus;
            }
            break;
        }
    }
}

void Differentiator::optimization() {
    bool modified = true;
    while(modified) {
        modified = false;
        Tree<MathObject>** sequence = tree->allocTree();
        tree->postorder(sequence);
        for (int i = 0; i < tree->getSize(); ++i) {
            if (sequence[i]->getValue().type == MathObject::OPERATION_TYPE) {
                modified = modified || optimizationCalc(sequence[i]);
                modified = modified || optimizationZero(sequence[i]);
                modified = modified || optimizationOne (sequence[i]);
            }
        }
        free(sequence);
    }
}

#define LEFT_IS(arg)  node->getChild(LEFT_CHILD )->getValue().type == MathObject::NUMBER_TYPE && node->getChild(LEFT_CHILD )->getValue().num == arg
#define RIGHT_IS(arg) node->getChild(RIGHT_CHILD)->getValue().type == MathObject::NUMBER_TYPE && node->getChild(RIGHT_CHILD)->getValue().num == arg

bool Differentiator::optimizationZero(Tree<MathObject> *node) {
    if (it_is(+)) {
        if (LEFT_IS(0)) {
            replaceBy(RIGHT_CHILD, node);
            return true;
        } else if (RIGHT_IS(0)) {
            replaceBy(LEFT_CHILD, node);
            return true;
        }
    }
    if (it_is(*)) {
        if ((LEFT_IS(0)) || (RIGHT_IS(0))) {
            makeNumber(0, node);
            return true;
        }
    }
    if (it_is(-)) {
        if (RIGHT_IS(0)) {
            replaceBy(LEFT_CHILD, node);
            return true;
        }
        if (LEFT_IS(0)) {
            node->removeSubTree(LEFT_CHILD);
            node->setValue(MathObject(MathObject::OPERATION_TYPE, getFunctionCode("--")));
        }
    }
    if (it_is(^)) {
        if (RIGHT_IS(0)) {
            makeNumber(1, node);
            return true;
        }
        if (LEFT_IS(0)) {
            makeNumber(0, node);
            return true;
        }
    }
    if (it_is(/)) {
        if (LEFT_IS(0)) {
            makeNumber(0, node);
            return true;
        }
    }
    return false;
}

bool Differentiator::optimizationOne(Tree<MathObject> *node) {
    if (it_is(*)) {
        if (RIGHT_IS(1)) {
            replaceBy(LEFT_CHILD, node);
            return true;
        }
        if (LEFT_IS(1)) {
            replaceBy(RIGHT_CHILD, node);
            return true;
        }
    }
    if (it_is(/)) {
        if (RIGHT_IS(1)) {
            replaceBy(LEFT_CHILD, node);
            return true;
        }
    }
    if (it_is(^)) {
        if (RIGHT_IS(1)) {
            replaceBy(LEFT_CHILD, node);
            return true;
        }
        if (LEFT_IS(1)) {
            node->removeSubTree(LEFT_CHILD );
            node->removeSubTree(RIGHT_CHILD);
            node->setValue(MathObject(1));
            return true;
        }
    }
    return false;
}

bool Differentiator::optimizationCalc(Tree<MathObject> *node) {
    double value = FUNCTIONS[node->getValue().code]->calculate(node);
    if (!isnan(value)) {
        makeNumber(value, node);
        return true;
    }
    return false;
}

#undef dL
#undef dR
#undef L
#undef R
#undef overrideBinaryOperator
#undef overrideUnaryOperator
#undef it_is
#undef newOperation
#undef LEFT_IS
#undef RIGHT_IS
#undef VERIFY_CONTEXT
