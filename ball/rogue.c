/*
 * rogue.c — Roguelike mode core for Neverball.
 *
 * Builds a shuffled, cross-set, no-repeat level pool and hands levels
 * out one at a time.  Set enable/disable state and run-length settings
 * persist across menu visits within a single game session.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rogue.h"
#include "set.h"
#include "level.h"
#include "common.h"

/*---------------------------------------------------------------------------*/
/* Configuration state (persists across menu visits)                         */
/*---------------------------------------------------------------------------*/

/* Per-set enabled flags; 1 = included in pool, 0 = excluded. */
static int set_enabled[ROGUE_MAX_SETS];

/* Maximum levels per run; 0 = play everything. */
static int run_length = ROGUE_RUNLEN_20;

/* Whether ball-loss ends the run. */
static int balls_enabled = 1;

/* Whether bonus levels are included in the pool. */
static int include_bonus = 0;

/* Track whether we have been initialised at least once so we can seed
 * the set_enabled array on first use. */
static int config_ready = 0;

static void ensure_config(void)
{
    if (!config_ready)
    {
        int i;
        /* Enable all sets by default. */
        for (i = 0; i < ROGUE_MAX_SETS; i++)
            set_enabled[i] = 1;
        config_ready = 1;
    }
}

void rogue_set_enabled(int set_index, int enabled)
{
    ensure_config();
    if (set_index >= 0 && set_index < ROGUE_MAX_SETS)
        set_enabled[set_index] = enabled ? 1 : 0;
}

int rogue_set_is_enabled(int set_index)
{
    ensure_config();
    if (set_index >= 0 && set_index < ROGUE_MAX_SETS)
        return set_enabled[set_index];
    return 0;
}

void rogue_set_run_length(int n)
{
    run_length = n;
}

int rogue_get_run_length(void)
{
    return run_length;
}

void rogue_set_balls_enabled(int enabled)
{
    balls_enabled = enabled ? 1 : 0;
}

int rogue_get_balls_enabled(void)
{
    return balls_enabled;
}

void rogue_set_include_bonus(int enabled)
{
    include_bonus = enabled ? 1 : 0;
}

int rogue_get_include_bonus(void)
{
    return include_bonus;
}

/*---------------------------------------------------------------------------*/
/* Pool — built fresh at rogue_init()                                        */
/*---------------------------------------------------------------------------*/

/*
 * A pool entry pairs a set index with a level index within that set.
 * We use these indices to call set_goto() / get_level() at play time.
 */
struct pool_entry
{
    int set_index;
    int lvl_index;
};

#define POOL_MAX 4096

static struct pool_entry pool[POOL_MAX];
static int pool_size   = 0;
static int pool_cursor = 0;   /* Next entry to hand out. */
static int run_cap     = 0;   /* Effective cap for this run (0 = unlimited). */
static int played      = 0;   /* Levels played so far.   */

struct history_entry
{
    char set_name[MAXSTR];
    char level_name[MAXSTR];
    char shot[PATHMAX];
    char msg[MAXSTR];
};

static struct history_entry history[POOL_MAX];
static int history_count = 0;

/* Fisher-Yates shuffle. */
static void shuffle_pool(void)
{
    int i;
    srand((unsigned int) time(NULL));
    for (i = pool_size - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        struct pool_entry tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }
}

int rogue_init(void)
{
    int num_sets, si, li;

    ensure_config();

    pool_size   = 0;
    pool_cursor = 0;
    played      = 0;
    history_count = 0;

    num_sets = set_init();   /* (Re-)load all sets. */

    for (si = 0; si < num_sets && si < ROGUE_MAX_SETS; si++)
    {
        if (!set_enabled[si])
            continue;

        set_goto(si);

        for (li = 0; pool_size < POOL_MAX; li++)
        {
            struct level *l = get_level(li);
            if (!l)
                break;
            /* Skip bonus levels unless the player opted in. */
            if (level_bonus(l) && !include_bonus)
                continue;
            pool[pool_size].set_index = si;
            pool[pool_size].lvl_index = li;
            pool_size++;
        }
    }

    shuffle_pool();

    /* Apply run-length cap. */
    if (run_length > 0 && run_length < pool_size)
        run_cap = run_length;
    else
        run_cap = pool_size;

    return pool_size;
}

struct level *rogue_next_level(void)
{
    struct pool_entry *e;
    struct level *l;

    if (pool_cursor >= pool_size || pool_cursor >= run_cap)
        return NULL;

    e = &pool[pool_cursor++];
    played++;

    set_goto(e->set_index);
    l = get_level(e->lvl_index);

    if (l && history_count < POOL_MAX)
    {
        SAFECPY(history[history_count].set_name, set_name(e->set_index));
        SAFECPY(history[history_count].level_name, level_name(l));
        SAFECPY(history[history_count].shot, level_shot(l));
        SAFECPY(history[history_count].msg, level_msg(l));
        history_count++;
    }

    return l;
}

int rogue_has_next(void)
{
    return (pool_cursor < pool_size && pool_cursor < run_cap);
}

int rogue_remaining(void)
{
    int rem = run_cap - pool_cursor;
    return rem > 0 ? rem : 0;
}

int rogue_played(void)
{
    return played;
}

int rogue_run_cap(void)
{
    return run_cap;
}

int rogue_history_count(void)
{
    return history_count;
}

const char *rogue_history_set_name(int i)
{
    return (i >= 0 && i < history_count) ? history[i].set_name : "";
}

const char *rogue_history_level_name(int i)
{
    return (i >= 0 && i < history_count) ? history[i].level_name : "";
}

const char *rogue_history_shot(int i)
{
    return (i >= 0 && i < history_count) ? history[i].shot : "";
}

const char *rogue_history_msg(int i)
{
    return (i >= 0 && i < history_count) ? history[i].msg : "";
}
