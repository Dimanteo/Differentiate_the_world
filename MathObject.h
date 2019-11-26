#ifndef DIFFERENTIATOR_MATHOBJECT_H
#define DIFFERENTIATOR_MATHOBJECT_H

#include <cmath>

struct MathObject {

    const static size_t MAX_LENGTH = 30;

    enum TYPE {
        POISON_TYPE = 0, NUMBER_TYPE = 'N', OPERATION_TYPE = 'O', VARIABLE_TYPE = 'V'
    };

    TYPE type;
    union {
        int code;
        double num;
    };

    MathObject();
    MathObject(double number);
    MathObject(TYPE objType, int objCode);
    ~MathObject();
};

MathObject::MathObject() : type(POISON_TYPE), num(NAN) {}

MathObject::

MathObject::~MathObject() {
    type = POISON_TYPE;
    code = 0;
    num = 0;
}

MathObject::MathObject(double number) {
    type = NUMBER_TYPE;
    num = number;
}

MathObject::MathObject(TYPE objType, int objCode) : type(objType), code(objCode) {}

#endif //DIFFERENTIATOR_MATHOBJECT_H
