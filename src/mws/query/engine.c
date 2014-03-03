/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/
/**
 * @brief   Query engine implementation
 * @file    engine.c
 * @date    21 Feb 2013
 *
 * License: GPLv3
 */

#include "mws/query/engine.h"

#include "mws/index/encoded_token_dict.h"
#include "common/utils/macro_func.h"

/*--------------------------------------------------------------------------*/
/* Constants                                                                */
/*--------------------------------------------------------------------------*/

#define MAX_VAR_INSTATIATION_SIZE       256
#define MAX_QUERY_STACK_SIZE            512

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

typedef struct var_instantiation_s {
    bool solved;
    encoded_token_t tokens[MAX_VAR_INSTATIATION_SIZE];
    int num_tokens;
} var_instantiation_t;

typedef struct token_stack_s {
    encoded_token_t data[MAX_QUERY_STACK_SIZE];
    int size;
} token_stack_t;

static inline
encoded_token_t token_stack_pop(token_stack_t* RESTRICT stack) {
    stack->size--;
    return stack->data[stack->size];
}

static inline
void token_stack_push(token_stack_t* RESTRICT stack, encoded_token_t token) {
    stack->data[stack->size] = token;
    stack->size++;
}

static inline
void token_stack_pop_many(token_stack_t* RESTRICT stack, int to_pop) {
    assert(stack->size >= to_pop);
    stack->size -= to_pop;
}

static inline
bool token_stack_empty(const token_stack_t* RESTRICT stack) {
    return (stack->size == 0);
}

typedef struct query_ctxt_s {
    /* query tokens and iterator */
    token_stack_t query_stack;
    /* index iterator */
    const inode_t* curr_index_inode;
    token_stack_t index_stack;

    /* var instantiations */
    var_instantiation_t vars[VAR_ID_MAX];
    /* var solve stack */
    uint32_t solving_var_id;

    /* index allocator */
    const memsector_alloc_header_t* alloc;

    /* result callback */
    result_callback_t result_cb;
    void*             result_cb_handle;
} query_ctxt_t;

/*--------------------------------------------------------------------------*/
/* Local methods                                                            */
/*--------------------------------------------------------------------------*/

static
void query_ctxt_init(query_ctxt_t* RESTRICT      query_ctxt,
                     index_handle_t* RESTRICT    index,
                     encoded_formula_t* RESTRICT query,
                     result_callback_t           result_cb,
                     void* RESTRICT              result_cb_handle);

static
int process_query_token(query_ctxt_t* query_ctxt);

static
int match_var_to_index(query_ctxt_t* query_ctxt, uint32_t arity);

static
int match_var_to_query(query_ctxt_t* query_ctxt, uint32_t arity);

static
int match_var_to_stack(query_ctxt_t* query_ctxt, token_stack_t* stack);

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

int query_engine_run(index_handle_t* RESTRICT    index,
                     encoded_formula_t* RESTRICT query,
                     result_callback_t           result_cb,
                     void* RESTRICT              result_cb_handle) {
    query_ctxt_t query_ctxt;

    query_ctxt_init(&query_ctxt, index, query, result_cb, result_cb_handle);

    return process_query_token(&query_ctxt);
}

/*--------------------------------------------------------------------------*/
/* Local Implementation                                                     */
/*--------------------------------------------------------------------------*/

static
void query_ctxt_init(query_ctxt_t* RESTRICT      query_ctxt,
                     index_handle_t* RESTRICT    index,
                     encoded_formula_t* RESTRICT query,
                     result_callback_t           result_cb,
                     void* RESTRICT              result_cb_handle) {
    int i;

    // initialize variables table
    for (i = 0; i < VAR_ID_MAX; i++) {
        query_ctxt->vars[i].solved = false;
    }

    // initialize query stack
    int size = query->size;
    query_ctxt->query_stack.size = size;
    for (i = 0; i < size; i++) {
        query_ctxt->query_stack.data[size - i - 1] = query->data[i];
    }

    // intialize index
    query_ctxt->curr_index_inode = index->root;
    query_ctxt->index_stack.size = 0;

    // initialize memsector alloc
    query_ctxt->alloc = index->alloc;

    // initialize result callback data
    query_ctxt->result_cb = result_cb;
    query_ctxt->result_cb_handle = result_cb_handle;
}

static
int process_query_token(query_ctxt_t* RESTRICT query_ctxt) {
    int ret;
    token_stack_t* query = &query_ctxt->query_stack;

    // check if we reached a leaf - report results
    if (token_stack_empty(query)) {
        const leaf_t *leaf = (leaf_t*) query_ctxt->curr_index_inode;
        assert(leaf->type == LEAF_NODE);

        return query_ctxt->result_cb(query_ctxt->result_cb_handle, leaf);
    }

    // otherwise get next token and find match
    encoded_token_t query_token = token_stack_pop(query);

    if (encoded_token_is_var(query_token)) {  // variable query token
        int var_id = encoded_token_get_id(query_token);
        if (query_ctxt->vars[var_id].solved) {  // solved
            var_instantiation_t* var = &query_ctxt->vars[var_id];
            int i;
            int size = var->num_tokens;
            for (i = 0; i < size; ++i) {
                token_stack_push(query, var->tokens[size - i - 1]);
            }

            // continue
            ret = process_query_token(query_ctxt);
            if (ret != QUERY_CONTINUE) return ret;

            // revert token stack
            token_stack_pop_many(query, size);
        } else {  // unsolved
            query_ctxt->solving_var_id = var_id;
            query_ctxt->vars[var_id].num_tokens = 0;
            ret = match_var_to_index(query_ctxt, 1);
            if (ret != QUERY_CONTINUE) return ret;
        }

        token_stack_push(query, query_token);

    } else {  // constant query token
        if (!token_stack_empty(&query_ctxt->index_stack)) {  // index stack
            encoded_token_t index_token =
                    token_stack_pop(&query_ctxt->index_stack);
            if (encoded_token_is_var(index_token)) {  // variable index token
                // push back query token to stack
                token_stack_push(query, query_token);

                int var_id = encoded_token_get_id(index_token);
                query_ctxt->solving_var_id = var_id;
                ret = match_var_to_query(query_ctxt, 1);
                if (ret != QUERY_CONTINUE) return ret;
            } else {  // constant index token
                if (memcmp(&query_token, &index_token,
                           sizeof(query_token)) == 0) {
                    // continue
                    ret = process_query_token(query_ctxt);
                    if (ret != QUERY_CONTINUE) return ret;
                }
                // revert query stack
                token_stack_push(query, query_token);
            }
            token_stack_push(&query_ctxt->index_stack, index_token);
        } else {  // regular index
            const inode_t* curr = query_ctxt->curr_index_inode;
            memsector_off_t off = inode_get_child(curr, query_token);
            if (off != MEMSECTOR_OFF_NULL) {  // move to corresponding child
                const inode_t* child =
                        (inode_t*) memsector_off2addr(query_ctxt->alloc, off);

                query_ctxt->curr_index_inode = child;

                // continue
                ret = process_query_token(query_ctxt);
                if (ret != QUERY_CONTINUE) return ret;

                // revert
                query_ctxt->curr_index_inode = curr;

                // revert query token before hvars processing
                token_stack_push(query, query_token);

                // hvars
                uint32_t hvar_id_max =
                        inode_get_max_var(query_ctxt->curr_index_inode);
                uint32_t hvar_id;
                for (hvar_id = 0; hvar_id < hvar_id_max; hvar_id++) {
                    query_ctxt->solving_var_id = hvar_id;
                    ret = match_var_to_query(query_ctxt, 1);
                    if (ret != QUERY_CONTINUE) return ret;
                }
            } else {  // no child node exists
                // revert query token
                token_stack_push(query, query_token);
            }
        }
    }

    return QUERY_CONTINUE;
}

static
int match_var_to_index(query_ctxt_t* RESTRICT query_ctxt, uint32_t arity) {
    int ret;
    var_instantiation_t* var = &query_ctxt->vars[query_ctxt->solving_var_id];

    if (arity == 0) {
        uint32_t var_id = query_ctxt->solving_var_id;
        if (var->num_tokens > 0) {
            var->solved = true;
        }

        // continue
        ret = process_query_token(query_ctxt);
        if (ret != QUERY_CONTINUE) return ret;

        query_ctxt->solving_var_id = var_id;
        var->solved = false;

    } else if (!token_stack_empty(&query_ctxt->index_stack)) {  // index stack
        return match_var_to_stack(query_ctxt, &query_ctxt->index_stack);
    } else {  // regular index
        uint32_t i;
        uint32_t size = query_ctxt->curr_index_inode->size;
        const encoded_token_dict_entry_t* data =
                query_ctxt->curr_index_inode->data;

        for (i = 0; i < size; ++i) {
            const encoded_token_dict_entry_t* entry = &data[i];
            int pushed_var_tokens = 0;
            token_stack_t var_stack;
            var_stack.size = 0;
            token_stack_push(&var_stack, entry->token);

            while (!token_stack_empty(&var_stack)) {
                encoded_token_t token = token_stack_pop(&var_stack);
                if (encoded_token_is_var(token)) {  // var
                    uint32_t var_id = encoded_token_get_id(token);
                    // check self-referencing variable
                    if (var_id == query_ctxt->solving_var_id) {
                        if (token_stack_empty(&var_stack) &&
                                var->num_tokens == 0) {
                            // variable self references (e.g. Q1 -> H1 -> Q1)
                            // => set as not solved and proceed
                            break;
                        } else {
                            // variable is infinitely recursive
                            // (e.g. Q1 -> f(H1) -> f(g(Q1)))
                            // => no solution
                            goto revert_index;
                        }
                    }
                    if (query_ctxt->vars[var_id].solved) {  // solved var
                        var_instantiation_t* solved_var =
                                &query_ctxt->vars[var_id];
                        // push in reverse order on stack
                        int i;
                        for (i = solved_var->num_tokens - 1; i >= 0; --i) {
                            token_stack_push(&var_stack, solved_var->tokens[i]);
                        }
                        continue;
                    }
                }
                // save to var_instantiation
                var->tokens[var->num_tokens] = token;
                var->num_tokens++;
                pushed_var_tokens++;
            }

            // advance in the index
            const inode_t* curr = query_ctxt->curr_index_inode;
            const inode_t* child = (inode_t*)
                    memsector_off2addr(query_ctxt->alloc, entry->off);
            query_ctxt->curr_index_inode = child;

            // continue
            ret = match_var_to_index(query_ctxt,
                                     arity + entry->token.arity - 1);
            if (ret != QUERY_CONTINUE) return ret;

revert_index:
            // revert
            var->num_tokens -= pushed_var_tokens;
            query_ctxt->curr_index_inode = curr;
        }
    }

    return QUERY_CONTINUE;
}

static
int match_var_to_query(query_ctxt_t* query_ctxt, uint32_t arity) {
    UNUSED(arity);
    return match_var_to_stack(query_ctxt, &query_ctxt->query_stack);
}

static
int match_var_to_stack(query_ctxt_t* query_ctxt, token_stack_t* stack) {
    int ret;
    var_instantiation_t* var = &query_ctxt->vars[query_ctxt->solving_var_id];

    token_stack_t tmp_rev_stack;
    tmp_rev_stack.size = 0;

    // move the var matching to a temp reverted stack
    int arity = 1;
    while (arity > 0) {
        encoded_token_t token = token_stack_pop(stack);
        arity += encoded_token_get_arity(token) - 1;
        token_stack_push(&tmp_rev_stack, token);
    }

    // copy stack and revert
    token_stack_t var_stack;
    int i;
    int size = tmp_rev_stack.size;
    var_stack.size = size;
    for (i = 0; i < size; ++i) {
        var_stack.data[i] = tmp_rev_stack.data[size - 1 - i];
    }

    // simplify using solved vars and save to var_instantiation
    bool solved = true;
    while (!token_stack_empty(&var_stack)) {
        encoded_token_t token = token_stack_pop(&var_stack);
        if (encoded_token_is_var(token)) {  // var
            uint32_t var_id = encoded_token_get_id(token);
            // check self-referencing variable
            if (var_id == query_ctxt->solving_var_id) {
                if (token_stack_empty(&var_stack) && var->num_tokens == 0) {
                    // variable self references (e.g. Q1 -> H1 -> Q1)
                    // => set as not solved and proceed
                    solved = false;
                    break;
                } else {
                    // variable is infinitely recursive
                    // (e.g. Q1 -> f(H1) -> f(g(Q1)))
                    // => no solution
                    goto revert_stack;
                }
            }
            if (query_ctxt->vars[var_id].solved) {  // solved var
                var_instantiation_t* solved_var = &query_ctxt->vars[var_id];
                // push in reverse order on stack
                int i;
                for (i = solved_var->num_tokens - 1; i >= 0; --i) {
                    token_stack_push(&var_stack, solved_var->tokens[i]);
                }
                continue;
            }
        }
        // save to var_instantiation
        var->tokens[var->num_tokens] = token;
        var->num_tokens++;
    }

    // set var as solved
    var->solved = solved;

    // continue
    ret = process_query_token(query_ctxt);
    if (ret != QUERY_CONTINUE) return ret;

revert_stack:
    var->solved = false;
    while (!token_stack_empty(&tmp_rev_stack)) {
        token_stack_push(stack, token_stack_pop(&tmp_rev_stack));
    }

    return QUERY_CONTINUE;
}
