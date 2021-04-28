### 如何判定break语句属于哪个循环
定义静态全局变量，进入for/while语句时记录当前exp，退出时置NULL，处理break语句时该变量即为其所在循环。
以上操作无法处理嵌套的循环，内层循环会覆盖掉外层记录的for/while
修正方法为，在进入每个for/while语句时以局部变量保存当前的全局指针，然后将全局指针指向当前语句，退出时恢复全局指针。
这样，嵌套的多个循环语句对应的栈帧中的多个保存全局指针的局部变量就是一个事实上的循环语句指针的栈。

### 如何检查for中的循环变量在循环体中未被赋值（UNDO）
原理同上。若赋值语句处于for循环中，检查左值是否为循环变量。

循环中有与循环变量同名变量的定义
嵌套循环中，依次对每层的循环变量的检查

### 处理相互递归的声明（UNDO）
Tiger 允许相邻的类型声明、函数声明进行递归地定义（当然递归中不能出现重名与环），
因此在typeDecList或者funcdecList中，先遍历一遍所有的声明，将名字填充到符号表，不进行绑定。
再次遍历所有声明进行声明的解析进行绑定。
当处理完当前一系列声明后，检查其中每个声明是否完整。

### 关于名字的重复定义（UNDO）
Tiger 中类型和变量名可以相同，而同名的变量和函数将相互隐藏。

### 关于错误信息的输出
由于所有的类型都是通过 Ty_ty 来确定的（比较指针），
每一个名字与其实体\<name, Ty_ty\>都是唯一的，
因此利用 S_dump/TAB_dump 通过 Ty_ty 查找出符号名字，
在报错信息中输出，给用户更友好的体验。

并在初始环境(tyEnv、idEnv)中添加表示记录类型本身、数组类型本身的两个类型、表示函数的E_enventry，
以及对应NULL的未知类型、未知名字，
便于配合TYPE_CHECK、NAME_CHECK等宏给出较友好的错误信息。

### 类型别名的处理
可以使用一个 type 来定义另一个 type，新产生的 type 的类型为 Ty_name，即它只是另一个类型的名字。
在对 Ty_ty 进行比较时，将追溯到其真正的类型进行比较。
而在其余使用中，使用声明类型进行处理，以便错误信息与声明类型保持一致。

另一种“鸵鸟策略”是在处理类型声明时，将所有别名都绑定到真实定义的 Ty_ty，这样别名类型就名存实亡。
而无法通过 Ty_ty 得到作为声明类型的类型别名。


### 小坑
在 parsetest.c 中使用了 EM_anyErrors 判断是否存在错误，死活找不到符号
最后用 nm 命令查看符号表，才发现 errormsg.c 中定义的是 anyErrors，errormsg.h 声明的是 EM_anyErrors.. :)



### ※
在调试过程中遇到一个有趣（😡）的bug
符号表的打印依赖于以下函数，可见该函数中表的遍历依赖于 t->top，
因此，当我使用该函数遍历表时，若我的show函数再次使用该函数遍历表 t，
则遍历到的是不完整的表。

我把这种特性称为**不可重入**。（当一轮递归未完成时，不可再开启一轮相同的递归）
```C
void TAB_dump(TAB_table t, void (*show)(void *key, void *value)) {
  void *k = t->top;
  int index = ((unsigned)k) % TABSIZE;
  binder b = t->table[index];
  if (b==NULL) return;
  t->table[index]=b->next;
  t->top = b->prevtop;
  show(b->key,b->value);
  TAB_dump(t,show);
  assert(t->top == b->prevtop && t->table[index]==b->next);
  t->top=k;
  t->table[index]=b;
}
```
