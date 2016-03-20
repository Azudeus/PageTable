#include <time.h>
//-----------------------------------------------------------------------------
typedef struct {
    int Valid;
    int Frame;
    int Dirty;
    int Requested;
    clock_t timeLastAccessed;
    } page_table_entry;

typedef page_table_entry* page_table_pointer;
//-----------------------------------------------------------------------------
