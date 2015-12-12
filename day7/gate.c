#include "gate.h"
#include "common.h"
#include <stdarg.h>
#include <string.h>

GateOp Op_Const  = { .type = CONST,  .num_inputs = 0, .name = "const"  };
GateOp Op_Set    = { .type = SET,    .num_inputs = 1, .name = "set"    };
GateOp Op_Not    = { .type = NOT,    .num_inputs = 1, .name = "not"    };
GateOp Op_And    = { .type = AND,    .num_inputs = 2, .name = "and"    };
GateOp Op_Or     = { .type = OR,     .num_inputs = 2, .name = "or"     };
GateOp Op_LShift = { .type = LSHIFT, .num_inputs = 2, .name = "lshift" };
GateOp Op_RShift = { .type = RSHIFT, .num_inputs = 2, .name = "rshift" };

void Gate_set_input(Gate *self, const int position, Gate *input) {
    if( position > self->proto->op->num_inputs ) {
        die(
            "Too many inputs for %s (%s), position %d, but %d max",
            self->name, self->proto->op->name, position, self->proto->op->num_inputs
        );
    }

    self->inputs[position] = input;
}

static void Gate_init(Gate *self, char *name, va_list inputs) {
    self->name = strdup(name);
    
    int num_inputs = self->proto->op->num_inputs;
    self->inputs = malloc(sizeof(Gate) * num_inputs);
    
    for( int i = 0; i < num_inputs; i++ ) {
        Gate *gate = va_arg(inputs, Gate*);
        self->inputs[i] = gate;
    }
}

static void Gate_destroy(Gate *self) {
    free(self->name);
    free(self->inputs);
    free(self);
}


static void ConstGate_init(Gate *self, char *name, va_list inputs) {
    self->name = strdup(name);
    self->value = (GateVal)va_arg(inputs, int);
}

static GateVal ConstGate_get(Gate *self) {
    return self->value;
}

static void ConstGate_destroy(Gate *self) {
    free(self->name);
    free(self);
}

struct GateProto ConstGateProto = {
    .get = ConstGate_get,
    .init = ConstGate_init,
    .destroy = ConstGate_destroy
};


#define DeclareGate(OP) \
    struct GateProto OP##GateProto = {  \
        .init    = Gate_init,           \
        .destroy = Gate_destroy,        \
        .get     = OP##Gate_get,        \
        .op      = &Op_##OP             \
    }

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

Gate *Gate_factory(GateOp *op, char *name, ...) {
    Gate *gate = malloc(sizeof(Gate));
    
    switch(op->type) {
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
    
    va_list inputs;
    va_start(inputs, name);

    gate->proto->init(gate, name, inputs);
    
    va_end(inputs);

    return gate;
}
