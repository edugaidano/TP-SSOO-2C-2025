#include <master_util.h>

int asign_query_id()
{
    static int id = 0;
    return ++id;
}