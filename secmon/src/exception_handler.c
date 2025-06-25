#include "exception_handler.h"
#include "libraries/hardware/debug.h"

void invalid_exception(uint64_t err_type)
{
	LOGT("Error type: %d\n", err_type);
}