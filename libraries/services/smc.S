.globl smcTest
.type smcTest, %function;
smcTest:
    smc 0
    ret