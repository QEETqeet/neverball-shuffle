/*
 * st_rogue.c: Pre-run configuration screen for Neverball Roguelike mode.
 *

 * A second state (st_rogue_sets) shows a scrollable list of all
 * available sets with toggle buttons to include/exclude them.
 */

#include <stdio.h>
#include <string.h>

#include "gui.h"
#include "audio.h"
#include "config.h"
#include "video.h"
#include "set.h"
#include "progress.h"
#include "key.h"
#include "lang.h"
#include "common.h"

#include "game_common.h"   /* AUD_MENU and other AUD_* constants */

#include "rogue.h"
#include "st_rogue.h"
#include "st_level.h"
#include "st_shared.h"
#include "st_title.h"

/*===========================================================================*/
/* st_rogue — main setup screen                                              */
/*===========================================================================*/

enum
{
    ROGUE_START = GUI_LAST,
    ROGUE_EDIT_SETS,

    TOK_LEN_20,
    TOK_LEN_25,
    TOK_LEN_50,
    TOK_LEN_100,
    TOK_LEN_ALL,

    TOK_BALLS_YES,
    TOK_BALLS_NO,

    TOK_BONUS_YES,
    TOK_BONUS_NO,
};

static int len_btn[5];
static int ball_yes_id;
static int ball_no_id;
static int bonus_yes_id;
static int bonus_no_id;

static const int  len_values[] = { 20, 25, 50, 100, 0 };
static const char *len_labels[] = { "20", "25", "50", "100", "All" };
static const int  len_tokens[] = {
    TOK_LEN_20, TOK_LEN_25, TOK_LEN_50, TOK_LEN_100, TOK_LEN_ALL
};
#define LEN_COUNT 5

static int rogue_label_cell(int parent, const char *label)
{
    int id = gui_label(parent, _("enable/disable levels"),
                       GUI_SML, gui_wht, gui_wht);

    gui_set_label(id, label);
    gui_set_trunc(id, TRUNC_TAIL);
    gui_set_rect(id, GUI_ALL);

    return id;
}

static int rogue_state_cell(int parent, const char *label, int tok, int val)
{
    int id = gui_state(parent, label, GUI_SML, tok, val);

    gui_set_rect(id, GUI_ALL);

    return id;
}

static void rogue_refresh_hilites(void)
{
    int i, cur = rogue_get_run_length();
    for (i = 0; i < LEN_COUNT; i++)
        gui_set_hilite(len_btn[i], len_values[i] == cur);

    gui_set_hilite(ball_yes_id,  rogue_get_balls_enabled());
    gui_set_hilite(ball_no_id,  !rogue_get_balls_enabled());

    gui_set_hilite(bonus_yes_id,  rogue_get_include_bonus());
    gui_set_hilite(bonus_no_id,  !rogue_get_include_bonus());
}

static int rogue_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        return goto_state(&st_title);

    case ROGUE_EDIT_SETS:
        return goto_state(&st_rogue_sets);

    case TOK_LEN_20:  rogue_set_run_length(20);  break;
    case TOK_LEN_25:  rogue_set_run_length(25);  break;
    case TOK_LEN_50:  rogue_set_run_length(50);  break;
    case TOK_LEN_100: rogue_set_run_length(100); break;
    case TOK_LEN_ALL: rogue_set_run_length(0);   break;

    case TOK_BALLS_YES: rogue_set_balls_enabled(1); break;
    case TOK_BALLS_NO:  rogue_set_balls_enabled(0); break;

    case TOK_BONUS_YES: rogue_set_include_bonus(1); break;
    case TOK_BONUS_NO:  rogue_set_include_bonus(0); break;

    case ROGUE_START:
    {
        int count = rogue_init();
        if (count == 0)
            return 1;   /* No levels — stay. */

        progress_init(MODE_ROGUE);

        {
            struct level *l = rogue_next_level();
            if (l && progress_play(l))
                return goto_state(&st_level);
        }
        return 1;
    }
    }

    rogue_refresh_hilites();
    return 1;
}

/* Helper: one labelled toggle row  [label]  [yes] [no]  */
static void rogue_toggle_row(int parent, const char *label,
                              int tok_yes, int tok_no,
                              int *id_yes, int *id_no)
{
    int jd, kd;
    if ((jd = gui_hstack(parent)))
    {
        gui_filler(jd);
        if ((kd = gui_harray(jd)))
        {
            *id_no  = rogue_state_cell(kd, _("no"),  tok_no,  0);
            *id_yes = rogue_state_cell(kd, _("yes"), tok_yes, 0);
        }
        gui_space(jd);
        rogue_label_cell(jd, label);
        gui_filler(jd);
    }
}

static int rogue_gui(void)
{
    int id, jd, kd, ld;
    int i;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            kd = gui_label(jd, _("Run Options"), GUI_SML, gui_yel, gui_red);
            gui_set_rect(kd, GUI_ALL);
            gui_filler(jd);
            rogue_state_cell(jd, _("Back"), GUI_BACK, 0);
        }

        gui_space(id);

        if ((ld = gui_vstack(id)))
        {
            gui_space(ld);

            if ((jd = gui_hstack(ld)))
            {
                gui_filler(jd);
                rogue_state_cell(jd, _(" Select "), ROGUE_EDIT_SETS, 0);
                gui_space(jd);
                rogue_label_cell(jd, _("Enable/disable levels:"));
                gui_filler(jd);
            }

            gui_space(ld);

            rogue_toggle_row(ld, _("Enable balls:"),
                             TOK_BALLS_YES, TOK_BALLS_NO,
                             &ball_yes_id, &ball_no_id);

            gui_space(ld);

            rogue_toggle_row(ld, _("Include bonus levels:"),
                             TOK_BONUS_YES, TOK_BONUS_NO,
                             &bonus_yes_id, &bonus_no_id);

            gui_space(ld);

            if ((jd = gui_hstack(ld)))
            {
                gui_filler(jd);

                if ((kd = gui_harray(jd)))
                {
                    for (i = LEN_COUNT - 1; i >= 0; i--)
                        len_btn[i] = rogue_state_cell(kd, len_labels[i],
                                                      len_tokens[i], 0);
                }

                gui_space(jd);
                rogue_label_cell(jd, _("Run length:"));
                gui_filler(jd);
            }

            gui_space(ld);
        }

        gui_space(id);

        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);
            kd = gui_state(jd, _(" ▶ Start run "), GUI_MED, ROGUE_START, 0);
            gui_set_color(kd, gui_yel, gui_yel);
            gui_set_rect(kd, GUI_ALL);
            gui_focus(kd);
            gui_filler(jd);
        }

        gui_space(id);

        gui_layout(id, 0, 0);
    }

    rogue_refresh_hilites();
    return id;
}

static int rogue_enter(struct state *st, struct state *prev)
{
    audio_music_fade_to_menu(0.5f);
    video_clr_grab();
    return rogue_gui();
}

static int rogue_keybd(int c, int d)
{
    if (d && c == KEY_EXIT)
        return rogue_action(GUI_BACK, 0);
    return 1;
}

static int rogue_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return rogue_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return rogue_action(GUI_BACK, 0);
    }
    return 1;
}

struct state st_rogue = {
    rogue_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    rogue_keybd,
    rogue_buttn
};

/*===========================================================================*/
/* st_rogue_sets — set enable/disable editor                                 */
/*===========================================================================*/

enum
{
    RSETS_TOGGLE = GUI_LAST,
    RSETS_BACK,
    RSETS_ALL,
    RSETS_NONE
};

#define RSETS_STEP 6

static int rsets_total = 0;
static int rsets_first = 0;
static int rsets_shot_id;
static int rsets_desc_id;

/* We re-enter st_rogue_sets after each toggle so the GUI is always fresh. */

static int rsets_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
    case RSETS_BACK:
        return goto_state(&st_rogue);

    case GUI_PREV:
        rsets_first -= RSETS_STEP;
        if (rsets_first < 0)
            rsets_first = 0;
        return goto_state(&st_rogue_sets);

    case GUI_NEXT:
        rsets_first += RSETS_STEP;
        return goto_state(&st_rogue_sets);

    case RSETS_ALL:
    {
        int n = set_init();
        int i;
        for (i = 0; i < n; i++)
            rogue_set_enabled(i, 1);
        return goto_state(&st_rogue_sets);
    }

    case RSETS_NONE:
    {
        int n = set_init();
        int i;
        for (i = 0; i < n; i++)
            rogue_set_enabled(i, 0);
        return goto_state(&st_rogue_sets);
    }

    case RSETS_TOGGLE:
        /* val = set index */
        rogue_set_enabled(val, !rogue_set_is_enabled(val));
        return goto_state(&st_rogue_sets);
    }

    return 1;
}

static void rsets_over(int i)
{
    if (set_exists(i))
    {
        gui_set_image(rsets_shot_id, set_shot(i));
        gui_set_multi(rsets_desc_id, set_desc(i));
    }
}

static void rsets_set_button(int id, int i)
{
    if (set_exists(i))
    {
        int name_id;

        if (i == rsets_first)
            name_id = gui_start(id, "IJKLMNOPQRSTUVWXYZ", GUI_SML,
                                RSETS_TOGGLE, i);
        else
            name_id = gui_state(id, "IJKLMNOPQRSTUVWXYZ", GUI_SML,
                                RSETS_TOGGLE, i);

        gui_set_trunc(name_id, TRUNC_TAIL);
        gui_set_label(name_id, set_name(i));
        gui_set_hilite(name_id, rogue_set_is_enabled(i));
    }
    else
        gui_label(id, "", GUI_SML, 0, 0);
}

static int rsets_gui(void)
{
    int id, jd, kd;
    int i;

    rsets_total = set_init();

    if (rsets_total > 0)
        rsets_first = MIN(rsets_first,
                          (rsets_total - 1) - ((rsets_total - 1) % RSETS_STEP));
    else
        rsets_first = 0;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_state(jd, _("None"), GUI_SML, RSETS_NONE, 0);
            gui_space(jd);
            gui_state(jd, _("All"),  GUI_SML, RSETS_ALL,  0);
            gui_space(jd);
            gui_label(jd, _("Select:"), GUI_SML, gui_yel, gui_red);
            gui_filler(jd);
            gui_navig(jd, rsets_total, rsets_first, RSETS_STEP);
        }

        gui_space(id);

        if (rsets_total > 0 && (jd = gui_harray(id)))
        {
            const int ww = MIN(video.device_w, video.device_h) * 7 / 12;
            const int hh = ww / 4 * 3;

            rsets_shot_id = gui_image(jd, set_shot(rsets_first), ww, hh);

            if ((kd = gui_varray(jd)))
            {
                for (i = rsets_first; i < rsets_first + RSETS_STEP; i++)
                    rsets_set_button(kd, i);
            }
        }

        gui_space(id);
        rsets_desc_id = gui_multi(id, " \n \n \n \n \n", GUI_SML,
                                  gui_yel, gui_wht);

        if (rsets_total > 0)
            rsets_over(rsets_first);
        else
            gui_set_multi(rsets_desc_id, _("No level sets found."));

        gui_layout(id, 0, 0);
    }

    return id;
}

static int rsets_enter(struct state *st, struct state *prev)
{
    return rsets_gui();
}

static int rsets_keybd(int c, int d)
{
    if (d && c == KEY_EXIT)
        return rsets_action(GUI_BACK, 0);
    return 1;
}

static void rsets_point(int id, int x, int y, int dx, int dy)
{
    int jd = shared_point_basic(id, x, y);

    if (jd && gui_token(jd) == RSETS_TOGGLE)
        rsets_over(gui_value(jd));
}

static void rsets_stick(int id, int a, float v, int bump)
{
    int jd = shared_stick_basic(id, a, v, bump);

    if (jd && gui_token(jd) == RSETS_TOGGLE)
        rsets_over(gui_value(jd));
}

static int rsets_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return rsets_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return rsets_action(GUI_BACK, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L1, b) && rsets_first > 0)
            return rsets_action(GUI_PREV, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R1, b) &&
            rsets_first + RSETS_STEP < rsets_total)
            return rsets_action(GUI_NEXT, 0);
    }
    return 1;
}

struct state st_rogue_sets = {
    rsets_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    rsets_point,
    rsets_stick,
    shared_angle,
    shared_click,
    rsets_keybd,
    rsets_buttn
};

/*===========================================================================*/
/* st_rogue_quit — "Are you sure you want to leave?" confirmation            */
/*===========================================================================*/

/*===========================================================================*/
/* st_rogue_done / st_rogue_levels - completed run screens                   */
/*===========================================================================*/

enum
{
    RDONE_LEVELS = GUI_LAST,
    RDONE_FINISH,

    RLEV_SELECT,
};

#define RLEV_STEP 5

static int rlev_first = 0;
static int rlev_selected = 0;
static int rlev_shot_id = 0;
static int rlev_msg_id = 0;

static void rogue_time_str(char *buf, int size, int centis)
{
    int cs = centis % 100;
    int sec = centis / 100;
    int h = sec / 3600;
    int m = (sec / 60) % 60;
    int s = sec % 60;

    snprintf(buf, size, "%02d:%02d:%02d.%02d", h, m, s, cs);
}

static const char *rogue_done_message(int n)
{
    if (n > 150)
        return _("You must be INSANE!");
    if (n > 90)
        return _("You made it! That was a lot.");
    if (n > 30)
        return _("A strong run!");
    return _("Its about the levels you play!");
}

static int rdone_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case RDONE_LEVELS:
        rlev_first = 0;
        rlev_selected = 0;
        return goto_state(&st_rogue_levels);

    case GUI_BACK:
    case RDONE_FINISH:
        progress_stop();
        return goto_state(&st_title);
    }

    return 1;
}

static int rdone_gui(void)
{
    int id, jd, kd;
    char time_buf[32];
    char score_buf[64];
    char balls_buf[64];

    rogue_time_str(time_buf, sizeof (time_buf), curr_times());
    snprintf(score_buf, sizeof (score_buf), _("Final score  %d"), curr_score());
    snprintf(balls_buf, sizeof (balls_buf), _("Balls left  %d"),
             MAX(curr_balls(), 0));

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_vstack(id)))
        {
            gui_label(jd, _("Congratulations!"), GUI_MED, gui_grn, gui_grn);
            gui_multi(jd, rogue_done_message(rogue_history_count()),
                      GUI_SML, gui_wht, gui_wht);
            gui_set_rect(jd, GUI_ALL);
        }

        gui_space(id);

        if ((jd = gui_vstack(id)))
        {
            if ((kd = gui_harray(jd)))
            {
                gui_label(kd, time_buf, GUI_MED, gui_yel, gui_yel);
                gui_label(kd, _("In game time:"), GUI_SML, gui_wht, gui_wht);
            }

            gui_label(jd, score_buf, GUI_SML, gui_yel, gui_wht);

            if (rogue_get_balls_enabled())
                gui_label(jd, balls_buf, GUI_SML, gui_yel, gui_wht);

            gui_set_rect(jd, GUI_ALL);
        }

        gui_space(id);

        gui_state(id, _("See completed levels"), GUI_SML, RDONE_LEVELS, 0);
        gui_state(id, _("Finish"), GUI_SML, RDONE_FINISH, 0);

        gui_layout(id, 0, 0);
    }

    return id;
}

static int rdone_enter(struct state *st, struct state *prev)
{
    video_clr_grab();
    return rdone_gui();
}

static int rdone_keybd(int c, int d)
{
    if (d && c == KEY_EXIT)
        return rdone_action(RDONE_FINISH, 0);
    return 1;
}

static int rdone_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return rdone_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return rdone_action(RDONE_FINISH, 0);
    }
    return 1;
}

struct state st_rogue_done = {
    rdone_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    rdone_keybd,
    rdone_buttn
};

static void rlev_over(int i)
{
    if (i >= 0 && i < rogue_history_count())
    {
        gui_set_image(rlev_shot_id, rogue_history_shot(i));
        gui_set_multi(rlev_msg_id, rogue_history_msg(i));
        rlev_selected = i;
    }
}

static int rlev_action(int tok, int val)
{
    int total = rogue_history_count();

    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        return goto_state(&st_rogue_done);

    case GUI_PREV:
        rlev_first -= RLEV_STEP;
        if (rlev_first < 0)
            rlev_first = 0;
        rlev_selected = rlev_first;
        return goto_state(&st_rogue_levels);

    case GUI_NEXT:
        rlev_first += RLEV_STEP;
        if (rlev_first >= total)
            rlev_first = MAX(total - 1, 0);
        rlev_first -= rlev_first % RLEV_STEP;
        rlev_selected = rlev_first;
        return goto_state(&st_rogue_levels);

    case RLEV_SELECT:
        rlev_over(val);
        return 1;
    }

    return 1;
}

static int rlev_button(int id, int i)
{
    char label[MAXSTR * 2];
    int bd;

    if (i >= rogue_history_count())
    {
        bd = gui_label(id, "", GUI_SML, 0, 0);
        gui_set_rect(bd, GUI_ALL);
        return bd;
    }

    snprintf(label, sizeof (label), "%s - %s",
             rogue_history_set_name(i), rogue_history_level_name(i));

    bd = gui_state(id, label, GUI_SML, RLEV_SELECT, i);

    gui_set_trunc(bd, TRUNC_TAIL);
    gui_set_rect(bd, GUI_ALL);

    return bd;
}

static int rlev_gui(void)
{
    int id, jd, kd;
    int total = rogue_history_count();
    int pages = (total + RLEV_STEP - 1) / RLEV_STEP;
    int page;
    int i;
    char page_buf[32];

    if (total > 0)
        rlev_first = MIN(rlev_first, (total - 1) - ((total - 1) % RLEV_STEP));
    else
        rlev_first = 0;

    if (total > 0)
        rlev_selected = CLAMP(rlev_first, rlev_selected,
                              MIN(rlev_first + RLEV_STEP - 1, total - 1));
    else
        rlev_selected = 0;

    page = pages ? rlev_first / RLEV_STEP + 1 : 1;
    snprintf(page_buf, sizeof (page_buf), _("page %d/%d"), page, MAX(pages, 1));

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            int bk, pr, nx, bl;

            bk = gui_state(jd, _("Back"), GUI_SML, GUI_BACK, 0);
            gui_set_rect(bk, GUI_ALL);

            gui_space(jd);
            nx = gui_maybe(jd, GUI_TRIANGLE_RIGHT, GUI_NEXT, GUI_NONE,
                           rlev_first + RLEV_STEP < total);
            gui_set_rect(nx, GUI_ALL);
            gui_space(jd);

            {
                int pg = gui_label(jd, page_buf, GUI_SML, gui_wht, gui_wht);
                gui_set_rect(pg, GUI_ALL);
            }

            gui_space(jd);

            pr = gui_maybe(jd, GUI_TRIANGLE_LEFT, GUI_PREV, GUI_NONE,
                           rlev_first > 0);
            gui_set_rect(pr, GUI_ALL);



            gui_filler(jd);

            /* "beaten levels" right-aligned with yel→red gradient */
            bl = gui_label(jd, _("beaten levels"), GUI_SML, gui_yel, gui_red);
            gui_set_rect(bl, GUI_ALL);
        }

        gui_space(id);

        /*
         * Main content: screenshot on the right, level list on the left.
         * Mirrors the st_start layout (image left, varray right in harray).
         */
        if ((jd = gui_harray(id)))
        {
            const int ww = MIN(video.device_w, video.device_h) * 5 / 12;
            const int hh = ww / 4 * 3;

            /* Level list — varray so every cell is the same height */
            if ((kd = gui_varray(jd)))
            {
                for (i = rlev_first; i < rlev_first + RLEV_STEP; i++)
                    rlev_button(kd, i);
            }

            rlev_shot_id = gui_image(jd,
                                     total > 0 ? rogue_history_shot(rlev_selected) : "",
                                     ww, hh);
        }

        gui_space(id);

        rlev_msg_id = gui_multi(id, " \n \n \n \n ", GUI_TNY, gui_wht, gui_wht);
        gui_set_rect(rlev_msg_id, GUI_ALL);
        if (total > 0)
            rlev_over(rlev_selected);
        else
            gui_set_multi(rlev_msg_id, _("No completed levels."));

        gui_layout(id, 0, 0);
    }

    return id;
}

static int rlev_enter(struct state *st, struct state *prev)
{
    video_clr_grab();
    return rlev_gui();
}

static void rlev_point(int id, int x, int y, int dx, int dy)
{
    int jd = shared_point_basic(id, x, y);

    if (jd && gui_token(jd) == RLEV_SELECT)
        rlev_over(gui_value(jd));
}

static void rlev_stick(int id, int a, float v, int bump)
{
    int jd = shared_stick_basic(id, a, v, bump);

    if (jd && gui_token(jd) == RLEV_SELECT)
        rlev_over(gui_value(jd));
}

static int rlev_keybd(int c, int d)
{
    if (d && c == KEY_EXIT)
        return rlev_action(GUI_BACK, 0);
    return 1;
}

static int rlev_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return rlev_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return rlev_action(GUI_BACK, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L1, b) && rlev_first > 0)
            return rlev_action(GUI_PREV, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R1, b) &&
            rlev_first + RLEV_STEP < rogue_history_count())
            return rlev_action(GUI_NEXT, 0);
    }
    return 1;
}

struct state st_rogue_levels = {
    rlev_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    rlev_point,
    rlev_stick,
    shared_angle,
    shared_click,
    rlev_keybd,
    rlev_buttn
};

enum
{
    RQUIT_YES = GUI_LAST,
    RQUIT_NO
};

static struct state *rquit_back = NULL;

void goto_rogue_quit(struct state *back_state)
{
    rquit_back = back_state;
    goto_state(&st_rogue_quit);
}

static int rquit_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case RQUIT_YES:
        progress_stop();
        return goto_state(&st_title);

    case GUI_BACK:
    case RQUIT_NO:
        return goto_state(rquit_back ? rquit_back : &st_title);
    }
    return 1;
}

static int rquit_gui(void)
{
    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Leave the run?"), GUI_MED, gui_red, gui_red);
        gui_space(id);
        gui_label(id, _("Your progress will be lost."), GUI_SML, gui_wht, gui_wht);
        gui_space(id);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_start(jd, _("Keep playing"), GUI_SML, RQUIT_NO,  0);
            gui_state(jd, _("Leave"),        GUI_SML, RQUIT_YES, 0);
        }

        gui_layout(id, 0, 0);
    }

    return id;
}

static int rquit_enter(struct state *st, struct state *prev)
{
    return rquit_gui();
}

static int rquit_keybd(int c, int d)
{
    if (d && c == KEY_EXIT)
        return rquit_action(RQUIT_NO, 0);
    return 1;
}

static int rquit_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return rquit_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return rquit_action(RQUIT_NO, 0);
    }
    return 1;
}

struct state st_rogue_quit = {
    rquit_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    rquit_keybd,
    rquit_buttn
};