#include "Utils.h"

#include <stdexcept>


void check(int res, const char * ctx)
{
    if(res < 0)
        throw std::runtime_error(ctx);
}

void check(bool res, const char * ctx)
{
    if(!res)
        throw std::runtime_error(ctx);
}