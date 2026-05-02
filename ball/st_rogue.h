#ifndef ST_ROGUE_H
#define ST_ROGUE_H

#include "state.h"

/*
 * st_rogue      — the "Play" / pre-run setup screen.
 * st_rogue_sets — sub-screen for enabling / disabling individual sets.
 * st_rogue_quit — "Are you sure?" confirmation before abandoning a run.
 */
extern struct state st_rogue;
extern struct state st_rogue_sets;
extern struct state st_rogue_quit;
extern struct state st_rogue_done;
extern struct state st_rogue_levels;

/*
 * Call this from anywhere in the game to show the quit confirmation.
 * back_state is where to return if the player cancels (usually the
 * calling state itself).
 */
void goto_rogue_quit(struct state *back_state);

#endif /* ST_ROGUE_H */
