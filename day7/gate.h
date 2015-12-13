#ifndef _gate_h
#define _gate_h

#include <stdarg.h>

#define __(o, m, ...) o->proto->m(o, ##__VA_ARGS__)

#include <stdint.h>

typedef uint16_t GateVal;

typedef enum {
    CONST, SET, NOT, AND, OR, LSHIFT, RSHIFT, UNDEF
} GateOpType;

typedef struct GateOp {
    GateOpType type;
    int num_inputs;
    char *name;
} GateOp;

GateOp Op_Undef;
GateOp Op_Const;
GateOp Op_Set;
GateOp Op_Not;
GateOp Op_And;
GateOp Op_Or;
GateOp Op_LShift;
GateOp Op_RShift;

GateOp *Op_lookup(char *_opname);

typedef struct Gate {
    struct GateProto *proto;

    char *name;
    GateVal value;          // for caching or setting directly
    struct Gate **inputs;
} Gate;

struct GateProto {
    GateOp *op;
    
    GateVal (*get)(Gate *self);
    void (*init)(Gate *self, char *name);
    void (*destroy)(Gate *self);
    void (*set_input)(Gate *self, const int position, Gate *input);
    void (*set_value)(Gate *self, GateVal value);
    void (*set_op)(Gate *self, GateOp *op);
} GateProto;

Gate *Gate_factory(GateOp *op, char *name);
GateVal Gate_get(Gate *self);

#endif
