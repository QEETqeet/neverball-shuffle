#ifndef ROGUE_H
#define ROGUE_H

#include "level.h"

/*---------------------------------------------------------------------------*/
/*
 * Roguelike mode: randomised, cross-set, no-repeat level pool.
 *
 * Call rogue_init() once before starting a run to build and shuffle
 * the pool.  Then use rogue_next_level() to obtain the next level
 * pointer (returns NULL when the pool is exhausted).
 *
 * Set enable/disable state and run-length cap survive across menus via
 * the static storage in rogue.c; they are intentionally NOT persisted
 * to the config file so that each session starts with sensible defaults.
 */

/* Maximum number of sets we can track. */
#define ROGUE_MAX_SETS  64

/* Run-length presets passed to rogue_set_run_length() (0 = unlimited). */
#define ROGUE_RUNLEN_20    20
#define ROGUE_RUNLEN_25    25
#define ROGUE_RUNLEN_50    50
#define ROGUE_RUNLEN_100   100
#define ROGUE_RUNLEN_ALL   0

/*---------------------------------------------------------------------------*/

/* --- Configuration (survives between menu visits) --- */

/* Enable or disable a set by its index in the loaded set list. */
void rogue_set_enabled(int set_index, int enabled);
int  rogue_set_is_enabled(int set_index);

/* Maximum levels to play in one run (0 = all). */
void rogue_set_run_length(int n);
int  rogue_get_run_length(void);

/* Whether losing a ball ends the run (Challenge-style ball tracking). */
void rogue_set_balls_enabled(int enabled);
int  rogue_get_balls_enabled(void);

/* Whether bonus levels are included in the pool (treated as normal levels). */
void rogue_set_include_bonus(int enabled);
int  rogue_get_include_bonus(void);

/*---------------------------------------------------------------------------*/

/* --- Run state --- */

/*
 * Build and shuffle the level pool from all enabled sets, honouring
 * the run-length cap.  Call this once when the player presses "Start".
 * Returns the total number of levels in the pool (0 if none available).
 */
int  rogue_init(void);

/*
 * Advance to and return the next level in the shuffled pool.
 * Returns NULL when the pool is exhausted or the run limit is reached.
 * The returned pointer is owned by the set module; do not free it.
 */
struct level *rogue_next_level(void);

/* True if there are more levels left in the current run. */
int  rogue_has_next(void);

/* How many levels remain in this run. */
int  rogue_remaining(void);

/* Total levels played so far in this run. */
int  rogue_played(void);

/* Effective cap for the current run (total levels in pool after cap applied). */
int  rogue_run_cap(void);

/* Completed-run history, in the order levels were played. */
int         rogue_history_count(void);
const char *rogue_history_set_name(int i);
const char *rogue_history_level_name(int i);
const char *rogue_history_shot(int i);
const char *rogue_history_msg(int i);

/*---------------------------------------------------------------------------*/

#endif /* ROGUE_H */
