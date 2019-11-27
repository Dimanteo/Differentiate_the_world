#include <string.h>
#include "Tree_t\Tree.cpp"
#include "My_Headers\txt_files.h"
#include "MathObject.h"
#include "Function.h"
#include "Parser.h"


class Differentiator {

public:

    const static size_t VARIABLES_MAX_COUNT = 100;
    //char *variables[VARIABLES_MAX_COUNT];
    char** variables;
    size_t variables_count = 0;

    Tree<MathObject>* tree = nullptr;

    Differentiator();
    ~Differentiator();

    void parse(FILE* in);

    void dump(const char outFileName[], const char state[], const char message[], const char file[], const char function[], int line);

    char * texDump(char *buffer, Tree<MathObject>* node);

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



int main() {
    FILE* file = fopen("Input.txt", "rb");
    Differentiator* laba_killer = new Differentiator();
    laba_killer->parse(file);
    fclose(file);
    FILE* log = fopen("../Debug/Diff.txt", "wb");
    fclose(log);
    FILE* latex = fopen("../Debug/Diff.tex", "wb");
    fclose(latex);
    laba_killer->dump("../Debug/Diff", OK_STATE, "call from main", __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
    switch (node->getValue().type) {
        case MathObject::NUMBER_TYPE: {
            char num_string[MathObject::MAX_LENGTH] = "";
            sprintf(num_string, "%g", node->getValue().num);
            buffer = (char*)realloc(buffer, sizeof(buffer) + strlen(num_string));
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
                buffer = (char*)realloc(buffer, sizeof(buffer) + strlen(output) + 2);
                strcat(buffer, "(");
                strcat(buffer, output);
                strcat(buffer, ")");
            } else {
                buffer = (char*)realloc(buffer, sizeof(buffer) + strlen(output));
                strcat(buffer, output);
            }
            free(output);
            free(rightString);
            free(leftString);
            break;
        }
        case MathObject::VARIABLE_TYPE: {
            buffer = (char*)realloc(buffer, sizeof(buffer) + strlen(variables[node->getValue().code]));
            strcat(buffer, variables[node->getValue().code]);
            break;
        }
    }
    return buffer;
}

