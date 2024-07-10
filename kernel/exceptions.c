#include "kernel/exceptions.h"

void InvalidException(void* ex)
{
	//kprintf("InvalidException!\n");
}

void data_abort_exception(uint64_t status)
{
	kprintf("----------\nData Abort Exception:\nStatus code: %d\n", status);
	while(1);
}