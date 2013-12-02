#include <vfs/mm.h>
#include "vfs_private.h"


uvar64   vfs_mm_mem_total_used()
{
    return g_vfs?g_vfs->_M_mem_total_used:0;
}

uvar64   vfs_mm_mem_pak_used()
{
    return g_vfs?g_vfs->_M_mem_pak_used:0;
}

uvar64   vfs_mm_mem_hashtable_used()
{
    return g_vfs?g_vfs->_M_mem_hashtable_used:0;
}
