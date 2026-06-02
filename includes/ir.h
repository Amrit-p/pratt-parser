#ifndef IR_H_
#define IR_H_

#include "AST.h"
#include "compiler.h"

Compiler* ir_gen(AST *ast);

#endif /* IR_H_ */