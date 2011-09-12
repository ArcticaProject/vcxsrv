/**
 * Copyright © 2011 Red Hat, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/Xlib.h>
#include <list.h>
#include <string.h>
#include <assert.h>

struct parent {
    int a;
    struct list children;
    int b;
};

struct child {
    int foo;
    int bar;
    struct list node;
};

static void
test_list_init(void)
{
    struct parent parent, tmp;

    memset(&parent, 0, sizeof(parent));
    parent.a = 0xa5a5a5;
    parent.b = ~0xa5a5a5;

    tmp = parent;

    list_init(&parent.children);

    /* test we haven't touched anything else. */
    assert(parent.a == tmp.a);
    assert(parent.b == tmp.b);

    assert(list_is_empty(&parent.children));
}

static void
test_list_add(void)
{
    struct parent parent = {0};
    struct child child[3];
    struct child *c;

    list_init(&parent.children);

    list_add(&child[0].node, &parent.children);
    assert(!list_is_empty(&parent.children));

    c = list_first_entry(&parent.children, struct child, node);
    assert(memcmp(c, &child[0], sizeof(struct child)) == 0);

    /* note: list_add prepends */
    list_add(&child[1].node, &parent.children);
    c = list_first_entry(&parent.children, struct child, node);
    assert(memcmp(c, &child[1], sizeof(struct child)) == 0);

    list_add(&child[2].node, &parent.children);
    c = list_first_entry(&parent.children, struct child, node);
    assert(memcmp(c, &child[2], sizeof(struct child)) == 0);
};

static void
test_list_del(void)
{
    struct parent parent = {0};
    struct child child[3];
    struct child *c;

    list_init(&parent.children);

    list_add(&child[0].node, &parent.children);
    assert(!list_is_empty(&parent.children));

    list_del(&parent.children);
    assert(list_is_empty(&parent.children));

    list_add(&child[0].node, &parent.children);
    list_del(&child[0].node);
    assert(list_is_empty(&parent.children));

    list_add(&child[0].node, &parent.children);
    list_add(&child[1].node, &parent.children);

    c = list_first_entry(&parent.children, struct child, node);
    assert(memcmp(c, &child[1], sizeof(struct child)) == 0);

    /* delete first node */
    list_del(&child[1].node);
    assert(!list_is_empty(&parent.children));
    assert(list_is_empty(&child[1].node));
    c = list_first_entry(&parent.children, struct child, node);
    assert(memcmp(c, &child[0], sizeof(struct child)) == 0);

    /* delete last node */
    list_add(&child[1].node, &parent.children);
    list_del(&child[0].node);
    c = list_first_entry(&parent.children, struct child, node);
    assert(memcmp(c, &child[1], sizeof(struct child)) == 0);

    /* delete list head */
    list_add(&child[0].node, &parent.children);
    list_del(&parent.children);
    assert(list_is_empty(&parent.children));
    assert(!list_is_empty(&child[1].node));
    assert(!list_is_empty(&child[2].node));
}

static void
test_list_for_each(void)
{
    struct parent parent = {0};
    struct child child[3];
    struct child *c;
    int i = 0;

    list_init(&parent.children);

    list_add(&child[2].node, &parent.children);
    list_add(&child[1].node, &parent.children);
    list_add(&child[0].node, &parent.children);

    list_for_each_entry(c, &parent.children, node) {
        assert(memcmp(c, &child[i], sizeof(struct child)) == 0);
        i++;
    }

    /* foreach on empty list */
    list_del(&parent.children);
    assert(list_is_empty(&parent.children));

    list_for_each_entry(c, &parent.children, node) {
        assert(0); /* we must not get here */
    }
}


int main(int argc, char** argv)
{
    test_list_init();
    test_list_add();
    test_list_del();
    test_list_for_each();

    return 0;
}
