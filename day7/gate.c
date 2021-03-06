#include "gate.h"
#include "common.h"
#include <stdarg.h>
#include <string.h>

GateOp Op_Undef  = { .type = UNDEF,  .num_inputs = 0, .name = "undef"   };
GateOp Op_Const  = { .type = CONST,  .num_inputs = 0, .name = "const"  };
GateOp Op_Set    = { .type = SET,    .num_inputs = 1, .name = "set"    };
GateOp Op_Not    = { .type = NOT,    .num_inputs = 1, .name = "not"    };
GateOp Op_And    = { .type = AND,    .num_inputs = 2, .name = "and"    };
GateOp Op_Or     = { .type = OR,     .num_inputs = 2, .name = "or"     };
GateOp Op_LShift = { .type = LSHIFT, .num_inputs = 2, .name = "lshift" };
GateOp Op_RShift = { .type = RSHIFT, .num_inputs = 2, .name = "rshift" };

static void Gate_set_op(Gate *self, GateOp *op);

static void Gate_set_input(Gate *self, const int position, Gate *input) {
    if( position > self->proto->op->num_inputs ) {
        die(
            "Too many inputs for %s (%s), position %d, but %d max",
            self->name, self->proto->op->name, position, self->proto->op->num_inputs
        );
    }

    self->inputs[position] = input;

    __(self, clear_cache);
}

static void Gate_set_cache(Gate *self, GateVal value) {
    self->cache = value;
}

static void Gate_clear_cache(Gate *self) {
    self->cache = 0;
}

static void Gate_init(Gate *self, char *name) {
    self->name = strdup(name);
}

static void Gate_destroy(Gate *self) {
    free(self->name);
    free(self->inputs);
    free(self);
}

static GateVal ConstGate_get(Gate *self) {
    return self->cache;
}

static void ConstGate_clear_cache(Gate *self) {
    return;
}

static void ConstGate_set_input(Gate *self, const int position, Gate *input) {
    die("Const gate %s does not take an input", self->name);
}

struct GateProto ConstGateProto = {
    .get        = ConstGate_get,
    .init       = Gate_init,
    .destroy    = Gate_destroy,
    .op         = &Op_Const,
    .set_input  = ConstGate_set_input,
    .set_cache  = Gate_set_cache,
    .clear_cache= ConstGate_clear_cache,
    .set_op     = Gate_set_op
};


#define DeclareGate(OP) \
    struct GateProto OP##GateProto = {  \
        .init    = Gate_init,           \
        .destroy = Gate_destroy,        \
        .get     = OP##Gate_get,        \
        .op      = &Op_##OP,            \
        .set_input   = Gate_set_input,    \
        .set_cache   = Gate_set_cache,    \
        .clear_cache = Gate_clear_cache,  \
        .set_op      = Gate_set_op        \
    }

GateVal UndefGate_get(Gate *self) {
    die("Undef gate %s called", self->name);
    return -1;
}

DeclareGate(Undef);

GateVal SetGate_get(Gate *self) {
    return Gate_get(self->inputs[0]);
}

DeclareGate(Set);

GateVal NotGate_get(Gate *self) {
    return ~Gate_get(self->inputs[0]);
}

DeclareGate(Not);

GateVal AndGate_get(Gate *self) {
    return Gate_get(self->inputs[0]) & Gate_get(self->inputs[1]);
}

DeclareGate(And);

GateVal OrGate_get(Gate *self) {
    return Gate_get(self->inputs[0]) | Gate_get(self->inputs[1]);
}

DeclareGate(Or);

GateVal LShiftGate_get(Gate *self) {
    return Gate_get(self->inputs[0]) << Gate_get(self->inputs[1]);
}

DeclareGate(LShift);

GateVal RShiftGate_get(Gate *self) {
    return Gate_get(self->inputs[0]) >> Gate_get(self->inputs[1]);
}

DeclareGate(RShift);

static void Gate_set_op(Gate *self, GateOp *op) {
    switch(op->type) {
        case UNDEF:
            self->proto = &UndefGateProto;
            break;
        case CONST:
            self->proto = &ConstGateProto;
            break;
        case SET:
            self->proto = &SetGateProto;
            break;
        case NOT:
            self->proto = &NotGateProto;
            break;
        case AND:
            self->proto = &AndGateProto;
            break;
        case OR:
            self->proto = &OrGateProto;
            break;
        case LSHIFT:
            self->proto = &LShiftGateProto;
            break;
        case RSHIFT:
            self->proto = &RShiftGateProto;
            break;
        default:
            die("Unknown op type %s", op->name);
            break;
    }

    int num_inputs = self->proto->op->num_inputs;
    self->inputs = realloc(self->inputs, sizeof(Gate *) * num_inputs);

    __(self, clear_cache);
}

GateOp *Op_lookup(char *_opname) {
    char *opname = g_ascii_strdown(_opname, -1);
    GateOp *op = NULL;
    
    /* CHEATING! */
    switch(opname[0]) {
        case 'u':
            op = &Op_Undef;
            break;
        case 'c':
            op = &Op_Const;
            break;
        case 's':
            op = &Op_Set;
            break;
        case 'n':
            op = &Op_Not;
            break;
        case 'a':
            op = &Op_And;
            break;
        case 'o':
            op = &Op_Or;
            break;
        case 'l':
            op = &Op_LShift;
            break;
        case 'r':
            op = &Op_RShift;
            break;
        default:
            die("Unknown op %s", _opname);
    }

    free(opname);
    
    return op;
}

GateVal Gate_get(Gate *self) {
    GateVal val = self->cache;
    
    if( !val ) {
        val = __(self, get);
        __(self, set_cache, val);
    }

    return val;
}

Gate *Gate_factory(GateOp *op, char *name) {
    Gate *gate = calloc(1, sizeof(Gate));

    Gate_set_op(gate, op);
    __(gate, init, name);

    return gate;
}
