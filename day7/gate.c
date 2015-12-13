#include "gate.h"
#include "common.h"
#include <stdarg.h>
#include <string.h>

GateOp Op_Undef  = { .type = UNDEF,   .num_inputs = 0, .name = "undef"   };
GateOp Op_Const  = { .type = CONST,  .num_inputs = 0, .name = "const"  };
GateOp Op_Set    = { .type = SET,    .num_inputs = 1, .name = "set"    };
GateOp Op_Not    = { .type = NOT,    .num_inputs = 1, .name = "not"    };
GateOp Op_And    = { .type = AND,    .num_inputs = 2, .name = "and"    };
GateOp Op_Or     = { .type = OR,     .num_inputs = 2, .name = "or"     };
GateOp Op_LShift = { .type = LSHIFT, .num_inputs = 2, .name = "lshift" };
GateOp Op_RShift = { .type = RSHIFT, .num_inputs = 2, .name = "rshift" };

static void Gate_set_input(Gate *self, const int position, Gate *input) {
    if( position > self->proto->op->num_inputs ) {
        die(
            "Too many inputs for %s (%s), position %d, but %d max",
            self->name, self->proto->op->name, position, self->proto->op->num_inputs
        );
    }

    self->inputs[position] = input;
}

static void Gate_set_value(Gate *self, GateVal value) {
    self->value = value;
}

static void Gate_init(Gate *self, char *name) {
    self->name = strdup(name);
    
    int num_inputs = self->proto->op->num_inputs;
    self->inputs = calloc(num_inputs, sizeof(Gate));
}

static void Gate_destroy(Gate *self) {
    free(self->name);
    free(self->inputs);
    free(self);
}


static void ConstGate_init(Gate *self, char *name) {
    self->name = strdup(name);
}

static GateVal ConstGate_get(Gate *self) {
    return self->value;
}

static void ConstGate_set_input(Gate *self, const int position, Gate *input) {
    die("Const gate %s does not take an input", self->name);
}

static void ConstGate_destroy(Gate *self) {
    free(self->name);
    free(self);
}

struct GateProto ConstGateProto = {
    .get        = ConstGate_get,
    .init       = ConstGate_init,
    .destroy    = ConstGate_destroy,
    .op         = &Op_Const,
    .set_input  = ConstGate_set_input,
    .set_value  = Gate_set_value
};


#define DeclareGate(OP) \
    struct GateProto OP##GateProto = {  \
        .init    = Gate_init,           \
        .destroy = Gate_destroy,        \
        .get     = OP##Gate_get,        \
        .op      = &Op_##OP,            \
        .set_input = Gate_set_input,    \
        .set_value = Gate_set_value     \
    }

GateVal UndefGate_get(Gate *self) {
    die("Undef gate %s called", self->name);
    return -1;
}

DeclareGate(Undef);

GateVal SetGate_get(Gate *self) {
    return __(self->inputs[0], get);
}

DeclareGate(Set);

GateVal NotGate_get(Gate *self) {
    return ~__(self->inputs[0], get);
}

DeclareGate(Not);

GateVal AndGate_get(Gate *self) {
    return __(self->inputs[0], get) & __(self->inputs[1], get);
}

DeclareGate(And);

GateVal OrGate_get(Gate *self) {
    return __(self->inputs[0], get) | __(self->inputs[1], get);
}

DeclareGate(Or);

GateVal LShiftGate_get(Gate *self) {
    return __(self->inputs[0], get) << __(self->inputs[1], get);
}

DeclareGate(LShift);

GateVal RShiftGate_get(Gate *self) {
    return __(self->inputs[0], get) >> __(self->inputs[1], get);
}

DeclareGate(RShift);

Gate *Gate_factory(GateOp *op, char *name) {
    Gate *gate = malloc(sizeof(Gate));
    
    switch(op->type) {
        case UNDEF:
            gate->proto = &UndefGateProto;
            break;
        case CONST:
            gate->proto = &ConstGateProto;
            break;
        case SET:
            gate->proto = &SetGateProto;
            break;
        case NOT:
            gate->proto = &NotGateProto;
            break;
        case AND:
            gate->proto = &AndGateProto;
            break;
        case OR:
            gate->proto = &OrGateProto;
            break;
        case LSHIFT:
            gate->proto = &LShiftGateProto;
            break;
        case RSHIFT:
            gate->proto = &RShiftGateProto;
            break;
        default:
            die("Unknown op type %s", op->name);
            break;
    }
    
    gate->proto->init(gate, name);

    return gate;
}
