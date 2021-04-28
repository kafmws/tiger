### 冲突解决
若存在 reduce/reduce 冲突（实际上良好的语法不应当存在这种冲突），通过指定产生式的优先级来处理。
若存在 shift/reduce  冲突，通过指定规约的产生式（或产生式右部最后一个非终结符）的优先级与产生冲突移进的非终结符的优先级解决。

数组声明中 arrayExp : ID/*typeid*/ [ exp ] of exp 与左值中 lvalue : ID 的 shift/reduce 冲突的解决：
首先肯定不能直接规约，因为 arrayExp 不能由 lvalue [ exp ] of exp 规约。 所以选择移进。为 lvalue 添加产生式
```
lvalue : ID
       | ID [ exp ]
       | ...
```
使 ID [ exp ] 可以直接被规约。 并使移进 '[' 的优先级高于 lvalue : ID 的规约。
则冲突转移到
```
arrayExp : ID/*typeid*/ [ exp ] . of exp
lvalue   : ID [ exp ] .
```
显然移进 OF 的优先级应高于 lvalue   : ID [ exp ] 的规约。

从根本上解决问题：引入符号表，将 token ID 细分为 TYPEID 与 IDENTIFIER

### 语法问题
表达式序列，expSeq（Sequence）在书中明确定义为由括号扩起的、由分号分隔的两个或两个以上的表达式。
而在许多 GitHub 上可供参考的项目中均未严格定义，由此本项目与其他实现在语法上有所不同。可能不能通过某些项目中的测试。

注意非终结符 expSeq 与 absyn.h 中 expSeq 不同，前者'(' exp ';' exp {';' exp} ')' 
与 LET decs IN (/\*empty\*/|exp {;exp}) END 中的 任意个由';'分隔的exp 在数据结构上都表示为后者。

此外，将无值表达式处理为NULL，而非NilExp，后续无值表达式的类型为 void 而非 Nil。
