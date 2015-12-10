/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2015, ruki All rights reserved.
 *
 * @author      ruki
 * @file        allocator_default.c
 *
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME            "allocator_default"
#define TB_TRACE_MODULE_DEBUG           (0)

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "../pool.h"
#include "../large_pool.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * types
 */

/// the default allocator type
typedef struct __tb_allocator_default_t
{
    // the base
    tb_allocator_t          base;

    // the pool
    tb_pool_ref_t           pool;

    // the large pool   
    tb_large_pool_ref_t     large_pool;

}tb_allocator_default_t, *tb_allocator_default_ref_t;

/* //////////////////////////////////////////////////////////////////////////////////////
 * private implementation
 */
static tb_pointer_t tb_allocator_default_malloc(tb_allocator_ref_t self, tb_size_t size __tb_debug_decl__)
{
    // check
    tb_allocator_default_ref_t allocator = (tb_allocator_default_ref_t)self;
    tb_check_return_val(allocator && allocator->pool, tb_null);

    // trace
    tb_trace_d("malloc(%lu) at %s(): %lu, %s", size, func_, line_, file_);

    // malloc it
    return tb_pool_malloc_(allocator->pool, size __tb_debug_args__);
}
static tb_pointer_t tb_allocator_default_ralloc(tb_allocator_ref_t self, tb_pointer_t data, tb_size_t size __tb_debug_decl__)
{
    // check
    tb_allocator_default_ref_t allocator = (tb_allocator_default_ref_t)self;
    tb_check_return_val(allocator && allocator->pool, tb_null);

    // trace
    tb_trace_d("ralloc(%p, %lu) at %s(): %lu, %s", data, size, func_, line_, file_);

    // ralloc it
    return tb_pool_ralloc_(allocator->pool, data, size __tb_debug_args__);
}
static tb_bool_t tb_allocator_default_free(tb_allocator_ref_t self, tb_pointer_t data __tb_debug_decl__)
{ 
    // check
    tb_allocator_default_ref_t allocator = (tb_allocator_default_ref_t)self;
    tb_check_return_val(allocator && allocator->pool, tb_false);

    // trace    
    tb_trace_d("free(%p) at %s(): %lu, %s", data, func_, line_, file_);

    // free it
    return tb_pool_free_(allocator->pool, data __tb_debug_args__);
}
#ifdef __tb_debug__
static tb_void_t tb_allocator_default_dump(tb_allocator_ref_t self)
{
    // check
    tb_allocator_default_ref_t allocator = (tb_allocator_default_ref_t)self;
    tb_check_return(allocator && allocator->pool);

    // dump it
    tb_pool_dump(allocator->pool);
}
#endif
static tb_bool_t tb_allocator_default_instance_init(tb_handle_t instance, tb_cpointer_t priv)
{
    // check
    tb_allocator_default_ref_t allocator = (tb_allocator_default_ref_t)instance;
    tb_check_return_val(allocator && priv, tb_false);

    // the data and size
    tb_value_ref_t  tuple = (tb_value_ref_t)priv;
    tb_byte_t*      data = (tb_byte_t*)tuple[0].ptr;
    tb_size_t       size = tuple[1].ul;

    /* init the page first
     *
     * because this allocator may be called before tb_init()
     */
    if (!tb_page_init()) return tb_false;

    /* init the native memory first
     *
     * because this allocator may be called before tb_init()
     */
    if (!tb_native_memory_init()) return tb_false;

    // init allocator
    allocator->base.malloc   = tb_allocator_default_malloc;
    allocator->base.ralloc   = tb_allocator_default_ralloc;
    allocator->base.free     = tb_allocator_default_free;
#ifdef __tb_debug__
    allocator->base.dump     = tb_allocator_default_dump;
#endif

    // init large pool
    allocator->large_pool = tb_large_pool_init(data, size);
    tb_check_return_val(allocator->large_pool, tb_false);

    // init pool
    allocator->pool = tb_pool_init(allocator->large_pool);
    tb_check_return_val(allocator->pool, tb_false);

    // ok
    return tb_true;
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
tb_allocator_ref_t tb_allocator_default(tb_byte_t* data, tb_size_t size)
{
    // init
    static tb_atomic_t              s_inited = 0;
    static tb_allocator_default_t   s_allocator = {{0}, 0};

    // init the static instance
    tb_value_t tuple[2];
    tuple[0].ptr    = (tb_pointer_t)data;
    tuple[1].ul     = size;
    tb_singleton_static_init(&s_inited, &s_allocator, tb_allocator_default_instance_init, tuple);

    // ok
    return (tb_allocator_ref_t)&s_allocator;
}
