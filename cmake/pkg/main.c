#include <glib.h>

typedef struct A
{
    int v;
} A;

void test()
{
    g_print("test\n");
}

void foo(gconstpointer test_data)
{
    A *a = (A *)test_data;
    g_print("A.v = %d\n", a->v);
}

/* run a test with fixture setup and teardown */
typedef struct
{
    guint seed;
    guint prime;
    gchar *msg;
} Fixturetest;

static void fixturetest_setup(Fixturetest *fix, gconstpointer test_data)
{
    g_assert_true(test_data == (void *)0xc0cac01a);
    fix->seed = 18;
    fix->prime = 19;
    fix->msg = g_strdup_printf("%d", fix->prime);
}

static void fixturetest_test(Fixturetest *fix, gconstpointer test_data)
{
    guint prime = g_spaced_primes_closest(fix->seed);
    g_assert_cmpint(prime, ==, fix->prime);
    prime = g_ascii_strtoull(fix->msg, NULL, 0);
    g_assert_cmpint(prime, ==, fix->prime);
    g_assert_true(test_data == (void *)0xc0cac01a);
}

static void fixturetest_teardown(Fixturetest *fix,
                     gconstpointer test_data)
{
    g_assert_true(test_data == (void *)0xc0cac01a);
    g_free(fix->msg);
}

static void int_hash_test(void)
{
    gint i, rc;
    GHashTable *h;
    gint values[20];
    gint key;

    h = g_hash_table_new(g_int_hash, g_int_equal);
    g_assert(h != NULL);
    for (i = 0; i < 20; i++)
    {
        values[i] = i + 42;
        g_hash_table_insert(h, &values[i], GINT_TO_POINTER(i + 42));
    }

    g_assert(g_hash_table_size(h) == 20);

    for (i = 0; i < 20; i++)
    {
        key = i + 42;
        rc = GPOINTER_TO_INT(g_hash_table_lookup(h, &key));

        g_assert_cmpint(rc, ==, i + 42);
    }

    g_hash_table_destroy(h);
}

static void test_autofree(void)
{
#ifdef __clang_analyzer__
    g_test_skip("autofree tests aren’t understood by the clang analyser");
#else
    g_autofree gchar *p = NULL;
    g_autofree gchar *p2 = NULL;
    g_autofree gchar *alwaysnull = NULL;

    p = g_malloc(10);
    p2 = g_malloc(42);

    p[0] = 1;
    p2[0] = 1;

    if (TRUE)
    {
        g_autofree guint8 *buf = g_malloc(128);
        g_autofree gchar *alwaysnull_again = NULL;

        buf[0] = 1;

        g_assert_null(alwaysnull_again);
    }

    if (TRUE)
    {
        g_autofree guint8 *buf2 = g_malloc(256);

        buf2[255] = 42;
    }

    g_assert_null(alwaysnull);
#endif /* __clang_analyzer__ */
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    gchar *base_name = g_path_get_basename(argv[0]);
    g_set_prgname(base_name);
    g_free(base_name);

    g_log_set_debug_enabled(TRUE);
    g_debug("start test...");

    g_test_add_func("/autoptr/autofree", test_autofree);
    g_test_add_func("/hash/int", int_hash_test);

    A a;
    a.v = 100;

    g_test_add_func("/t", test);
    g_test_add_data_func("/td", &a, foo);

    g_test_add("/t1", Fixturetest, (void *)0xc0cac01a, fixturetest_setup, fixturetest_test, fixturetest_teardown);
    int ret = g_test_run();
    g_message("A:%d", a.v);
    return ret;
}
