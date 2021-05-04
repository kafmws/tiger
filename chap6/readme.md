### x86栈帧的表示
- 函数参数由 caller push 到栈上，由 %ebx + offset 向上偏移得到
- 局部变量若在当前栈帧内，由 %ebx - offset 向下访问得到
