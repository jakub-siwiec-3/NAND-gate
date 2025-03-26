#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>

#define SUCCESS 0
#define FAIL (-1)

// Forward declarations
typedef struct NodeList NodeList;
typedef struct NandGate NandGate;
typedef struct Input Input;

// Structure representing a NAND gate
struct NandGate {
    Input *inputs;    // Array of inputs
    unsigned n;       // Number of inputs
    bool visited;     // Flag for evaluation
    NodeList *outputs; // List of connected output gates
};

// Structure representing an input connection
struct Input {
    NandGate *gate;   // Connected NAND gate
    bool const *signalBool; // Constant boolean signal (if any)
};

// Linked list node for storing output connections
struct NodeList {
    NandGate *gate;
    NodeList *next;
};

// Function to create a new NAND gate with `n` inputs
NandGate *nand_new(unsigned n) {
    NandGate *gate = malloc(sizeof(NandGate));
    if (gate == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    gate->inputs = malloc(n * sizeof(Input));
    if (gate->inputs == NULL) {
        free(gate);
        errno = ENOMEM;
        return NULL;
    }

    for (unsigned i = 0; i < n; i++) {
        gate->inputs[i].gate = NULL;
        gate->inputs[i].signalBool = NULL;
    }

    gate->n = n;
    gate->visited = false;

    NodeList *dummy = malloc(sizeof(NodeList));
    if (dummy == NULL) {
        free(gate->inputs);
        free(gate);
        errno = ENOMEM;
        return NULL;
    }

    dummy->gate = NULL;
    dummy->next = NULL;
    gate->outputs = dummy;

    return gate;
}

// Function to clear an input connection
void clear_input(NandGate *gate, unsigned i) {
    gate->inputs[i].gate = NULL;
    gate->inputs[i].signalBool = NULL;
}

// Function to remove an output connection
void remove_output(NandGate *source, NandGate *target) {
    NodeList *prev = source->outputs;
    NodeList *cur = prev->next;
    while (cur != NULL) {
        if (cur->gate != target) {
            prev = cur;
            cur = cur->next;
        } else {
            prev->next = cur->next;
            free(cur);
            return;
        }
    }
}

// Function to remove an input connection from a specific NAND gate
void remove_single_input(NandGate *gate, NandGate *target) {
    Input *inputs = gate->inputs;
    unsigned i = 0;
    while (inputs[i].gate != target) { i++; }
    clear_input(gate, i);
}

// Function to delete a NAND gate and its associated connections
void nand_delete(NandGate *gate) {
    if (gate != NULL) {
        if (gate->n > 0) {
            Input *inputs = gate->inputs;
            for (unsigned i = 0; i < gate->n; i++) {
                if (inputs[i].gate != NULL) {
                    remove_output(inputs[i].gate, gate);
                }
            }
        }

        NodeList *temp = gate->outputs->next;
        while (temp != NULL) {
            remove_single_input(temp->gate, gate);
            temp = temp->next;
        }

        NodeList *next;
        temp = gate->outputs;
        while (temp != NULL) {
            next = temp->next;
            free(temp);
            temp = next;
        }

        free(gate->inputs);
        free(gate);
    }
}

// Function to connect the output of one NAND gate to an input of another
int nand_connect_nand(NandGate *outputGate, NandGate *inputGate, unsigned k) {
    if (outputGate == NULL || inputGate == NULL || k >= inputGate->n) {
        errno = EINVAL;
        return FAIL;
    }

    NodeList *newOutput = malloc(sizeof(NodeList));
    if (newOutput == NULL) {
        errno = ENOMEM;
        return FAIL;
    }

    newOutput->gate = inputGate;
    newOutput->next = outputGate->outputs->next;
    outputGate->outputs->next = newOutput;

    if (inputGate->inputs[k].signalBool != NULL) {
        assert(inputGate->inputs[k].gate == NULL);
        inputGate->inputs[k].signalBool = NULL;
        inputGate->inputs[k].gate = outputGate;
    } else if (inputGate->inputs[k].signalBool == NULL && inputGate->inputs[k].gate == NULL) {
        inputGate->inputs[k].gate = outputGate;
    } else {
        remove_output(inputGate->inputs[k].gate, inputGate);
        inputGate->inputs[k].gate = outputGate;
    }
    return SUCCESS;
}

// Function to connect a constant signal to a NAND gate input
int nand_connect_signal(bool const *signal, NandGate *gate, unsigned k) {
    if (gate == NULL || signal == NULL || k >= gate->n) {
        errno = EINVAL;
        return FAIL;
    }
    if (gate->inputs[k].gate != NULL) {
        assert(gate->inputs[k].signalBool == NULL);
        remove_output(gate->inputs[k].gate, gate);
        gate->inputs[k].signalBool = signal;
        gate->inputs[k].gate = NULL;
    } else {
        assert(gate->inputs[k].gate == NULL);
        gate->inputs[k].signalBool = signal;
    }

    return SUCCESS;
}

// Function to retrieve the input source (either gate or signal) of a NAND gate
void *nand_input(NandGate const *gate, unsigned k) {
    if (gate == NULL || k >= gate->n) {
        errno = EINVAL;
        return NULL;
    }

    if (gate->inputs[k].gate == NULL && gate->inputs[k].signalBool == NULL) {
        errno = 0;
        return NULL;
    }

    if (gate->inputs[k].gate == NULL) {
        return (bool *) gate->inputs[k].signalBool;
    }
    return gate->inputs[k].gate;
}

// Function to count the number of output connections
ssize_t nand_fan_out(NandGate const *gate) {
    if (gate == NULL) {
        errno = EINVAL;
        return FAIL;
    }
    unsigned count = 0;
    NodeList *node = gate->outputs->next;

    while (node != NULL) {
        count++;
        node = node->next;
    }

    return count;
}

// Function to retrieve an output connection
NandGate *nand_output(NandGate const *gate, ssize_t k) {
    if (gate == NULL || k >= nand_fan_out(gate)) {
        errno = EINVAL;
        return NULL;
    }

    NodeList *node = gate->outputs->next;
    unsigned i = 0;

    while (i < k) {
        node = node->next;
        i++;
    }

    return node->gate;
}

// Function to evaluate a NAND gate
ssize_t evaluate_gate(NandGate *gate, bool *signal) {
    if (gate->n == 0) {
        *signal = false;
        return 0;
    }

    if (gate->visited) {
        errno = ECANCELED;
        gate->visited = false;
        return FAIL;
    }

    gate->visited = true;
    bool allTrue = true;
    ssize_t depth = 0;

    for (unsigned i = 0; i < gate->n; i++) {
        bool tempSignal;
        if (gate->inputs[i].gate != NULL) {
            ssize_t eval = evaluate_gate(gate->inputs[i].gate, &tempSignal);
            if (eval == FAIL) {
                gate->visited = false;
                return FAIL;
            }
            depth = (eval > depth) ? eval : depth;
            if (!tempSignal) allTrue = false;
        }
    }

    gate->visited = false;
    *signal = (!allTrue);
    return depth + 1;
}

// Function to evaluate an array of NAND gates
ssize_t nand_evaluate(NandGate **gates, bool *signals, size_t m) {
    if (m == 0 || gates == NULL || signals == NULL) {
        errno = EINVAL;
        return FAIL;
    }
    ssize_t maxDepth = 0;

    for (size_t i = 0; i < m; i++) {
        bool result;
        ssize_t depth = evaluate_gate(gates[i], &result);
        if (depth == FAIL) return FAIL;
        signals[i] = result;
        maxDepth = (depth > maxDepth) ? depth : maxDepth;
    }

    return maxDepth;
}
