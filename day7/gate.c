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

GateVal GateDoSet(Gate *self) {
    return __(self->inputs[0], get);
}

GateVal GateDoNot(Gate *self) {
    return ~__(self->inputs[0], get);
}

GateVal GateDoAnd(Gate *self) {
    return __(self->inputs[0], get) & __(self->inputs[1], get);
}

GateVal GateDoOr(Gate *self) {
    return __(self->inputs[0], get) | __(self->inputs[1], get);
}

GateVal GateDoLShift(Gate *self) {
    return __(self->inputs[0], get) << __(self->inputs[1], get);
}

GateVal GateDoRShift(Gate *self) {
    return __(self->inputs[0], get) >> __(self->inputs[1], get);
}

void Gate_set_input(Gate *self, const int position, Gate *input) {
    if( position > self->op->num_inputs ) {
        die(
            "Too many inputs for %s (%s), position %d, but %d max",
            self->name, self->op->name, position, self->op->num_inputs
        );
    }

    self->inputs[position] = input;
}

static GateVal ConstGate_get(Gate *self) {
    return self->value;
}

static void ConstGate_init(Gate *self, char *name, va_list inputs) {
    self->name = strdup(name);
    self->value = va_arg(inputs, GateVal);
    self->proto->get = ConstGate_get;
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

static GateVal Gate_get(Gate *self) {
    switch(self->op->type) {
        case SET:
            return GateDoSet(self);
        case NOT:
            return GateDoNot(self);
        case AND:
            return GateDoAnd(self);
        case OR:
            return GateDoOr(self);
        case LSHIFT:
            return GateDoLShift(self);
        case RSHIFT:
            return GateDoRShift(self);
        default:
            die("Unknown op type %s", self->op->name);
            return 0;
    }
}

static void Gate_init(Gate *self, char *name, va_list inputs) {
    self->name = strdup(name);
    self->proto->get = Gate_get;
    
    int num_inputs = self->op->num_inputs;
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

struct GateProto GateProto = {
    .get  = Gate_get,
    .init = Gate_init,
    .destroy = Gate_destroy
};

Gate *Gate_factory(GateOp *op, char *name, ...) {
    Gate *gate = malloc(sizeof(Gate));

    gate->op = op;

    /* Special case for constant gates */
    if( gate->op->type == CONST )
        gate->proto = &ConstGateProto;
    else
        gate->proto = &GateProto;
    
    va_list inputs;
    va_start(inputs, name);

    gate->proto->init(gate, name, inputs);
    
    va_end(inputs);

    return gate;
}
