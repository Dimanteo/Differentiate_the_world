#include <string.h>
#include <cmath>
#include <cctype>
#include "Tree_t\Tree.cpp"
#include "My_Headers\txt_files.h"

class Differentiator {

public:
    const char BEGIN_SEPARATOR = '(';
    const char END_SEPARATOR = ')';
    const static int FUNCTIONS_NUMBER = 4;
    const static size_t VARIABLES_MAX_COUNT = 100;
    const static size_t MATH_OBJECT_MAX_LENGTH = 30;

    enum MATH_OBJECT_TYPE {
        POISON_TYPE = 0, NUMBER_TYPE = 'N', OPERATION_TYPE = 'O', VARIABLE_TYPE = 'V'
    };

    struct MathObject {
        MATH_OBJECT_TYPE type;
        union {
            int code;
            double num;
        };
    };

    struct Function {
        char token[5];
        void (*texPrint)(FILE* Latex, Tree<MathObject>* node);
        //TODO ссылка на функцию дифференцирования.

        //функции печати в tex
        static void sumTex(FILE* Latex, Tree<MathObject>* node);
        static void subTex(FILE* Latex, Tree<MathObject>* node);
        static void mulTex(FILE* Latex, Tree<MathObject>* node);
        static void divTex(FILE* Latex, Tree<MathObject>* node);
    };


    Function functions[FUNCTIONS_NUMBER] = {
            {"+", Function::sumTex},
            {"-", Function::subTex},
            {"*", Function::mulTex},
            {"/", Function::divTex}};


    char *variables[VARIABLES_MAX_COUNT];

    size_t variables_count = 0;
    Tree<MathObject>* tree;

    Differentiator();

    Differentiator(char begin_separator, char end_separator);

    ~Differentiator();

    char *readNode(char *ptr, Tree<MathObject> *node);

    void parse(FILE* in);

    MathObject translateMathObject(char *buffer);

    void dump(const char outFileName[], const char state[], const char message[], const char file[], const char function[], int line);

    void texDump(const char *filename, Tree<MathObject>* node);

};

template<>
void  Tree<Differentiator::MathObject>::valuePrint(FILE *file) {
    switch (value.type) {
        case Differentiator::NUMBER_TYPE:
            fprintf(file, "Number(%d) %lf", value.type, value.num);
            break;
        case Differentiator::OPERATION_TYPE:
            fprintf(file, "Operation(%d) %d", value.type, value.code);
            break;
        case Differentiator::VARIABLE_TYPE:
            fprintf(file, "Variable(%d) [%d]",value.type, value.code);
            break;
        case Differentiator::POISON_TYPE:
            fprintf(file, "void");
            break;
    }
}

template<>
void Tree<Differentiator::MathObject>::valueDestruct() {
    switch (value.type) {
        case Differentiator::NUMBER_TYPE:
            value.num = 0;
            break;
        case Differentiator::OPERATION_TYPE:
            value.code = 0;
            break;
        case Differentiator::VARIABLE_TYPE:
            value.code = 0;
            break;
    }
    value.type = Differentiator::POISON_TYPE;
}

int main() {
    FILE* file = fopen("Input.txt", "rb");
    Differentiator* laba_killer = new Differentiator();
    laba_killer->parse(file);
    fclose(file);
    FILE* log = fopen("../Debug/Diff.txt", "wb");
    fclose(log);
    laba_killer->dump("../Debug/Diff", OK_STATE, "call from main", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    return 0;
}

Differentiator::Differentiator() : tree(nullptr), variables_count(0) {}

Differentiator::Differentiator(const char begin_separator, const char end_separator) :
                    tree(nullptr), variables_count(0), BEGIN_SEPARATOR(begin_separator), END_SEPARATOR(end_separator) {}

Differentiator::~Differentiator() {
    delete tree;
    for (int i = 0; i < variables_count; ++i) {
        free(variables[i]);
    }
    variables_count = 0;
}

char* Differentiator::readNode(char *ptr, Tree<Differentiator::MathObject> *node) {
    assert(node);
    assert(ptr);

    ptr++;
    while (isspace(*ptr))
        ptr++;
    if (*ptr == BEGIN_SEPARATOR) {
        node->growChild(LEFT_CHILD, {});
        ptr = readNode(ptr, node->getChild(LEFT_CHILD));
    }

    while (isspace(*ptr))
        ptr++;
    node->setValue(translateMathObject(ptr));

    if (strchr(ptr, BEGIN_SEPARATOR) != nullptr && strchr(ptr, BEGIN_SEPARATOR) < strchr(ptr, END_SEPARATOR)) {
        node->growChild(RIGHT_CHILD, {});
        ptr = strchr(ptr, BEGIN_SEPARATOR);
        ptr = readNode(ptr, node->getChild(RIGHT_CHILD));
    }
    ptr = strchr(ptr, END_SEPARATOR) + 1;
    return ptr;
}


 Differentiator::MathObject Differentiator::translateMathObject(char* buffer) {
    assert(buffer);

     while (isspace(*buffer) || *buffer == BEGIN_SEPARATOR || *buffer == END_SEPARATOR)
         buffer++;

    double num = NAN;
    int is_num = 0;
    is_num = sscanf(buffer, "%lf", &num);
    if (is_num) {
        return MathObject{NUMBER_TYPE, {.num = num}};
    }

    char input[Differentiator::MATH_OBJECT_MAX_LENGTH] = {};
    char format[8] = "%[^ ";
    strcat(format, &BEGIN_SEPARATOR);
    strcat(format, &END_SEPARATOR);
    strcat(format, "]");
    sscanf(buffer, format, input);
    assert(strlen(input) != 0);

    for (int i = 0; i < FUNCTIONS_NUMBER; ++i) {
        if (strcmp(functions[i].token, input) == 0) {
            return MathObject{OPERATION_TYPE, {.code = i}};
        }
    }
    //if it's variable
    for (int i = 0; i < variables_count; ++i) {
        if(strcmp(variables[i], input) == 0) {
            variables[i] = strdup(input);
            return MathObject{VARIABLE_TYPE, {.code = i}};
        }
    }
    //if it's new variable
    variables[variables_count] = strdup(input);
    return MathObject{VARIABLE_TYPE, {.code = (int)variables_count++}};
}

void Differentiator::parse(FILE *in) {
    tree = new Tree<MathObject>(MathObject{POISON_TYPE , {.num = NAN}});
    size_t buffer_size = 0;
    char* buffer = read_file_to_buffer_alloc(in, "rb", &buffer_size);
    readNode(buffer, tree);
    free(buffer);
}

void Differentiator::dump(const char outFileName[], const char state[], const char message[], const char file[], const char function[], int line) {
    Tree<Differentiator::MathObject>** seq = tree->allocTree();
    tree->inorder(seq);
    char* extensionName = (char*)calloc(strlen(outFileName) + 5, sizeof(extensionName[0]));
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

    strcpy(extensionName, outFileName);
    strcat(extensionName, ".png");
    tree->graphDump(extensionName, seq);

    free(extensionName);
}

void Differentiator::texDump(const char filename[], Tree<MathObject>* node) {
    FILE* LaTex = fopen(filename, "ab");
    fprintf(LaTex, "\\begin{equation}\n");
    switch (node->getValue().type) {
        case NUMBER_TYPE:
            fprintf(LaTex, "(%g)", node->getValue().num);
            break;
        case OPERATION_TYPE:
            functions[node->getValue().code].texPrint(LaTex, node);
            break;
        case VARIABLE_TYPE:
            fprintf(LaTex, "(%s)", variables[node->getValue().code]);
            break;
    }
    fprintf(LaTex, "\\end{equation}\n");
}
