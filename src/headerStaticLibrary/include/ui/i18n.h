#pragma once

#include "libraries.h"

namespace ui {

    enum Language {
        LANG_EN = 0,
        LANG_ES = 1
    };

    enum StringKey {
        K_MENU_FILE,
        K_MENU_NEW,
        K_MENU_OPEN,
        K_MENU_SAVE,
        K_MENU_SAVE_AS,
        K_MENU_EXIT,

        K_MENU_EDIT,
        K_MENU_NEW_TASK,
        K_MENU_EDIT_SELECTED,
        K_MENU_DELETE_SELECTED,
        K_MENU_UNDO,

        K_MENU_VIEW,
        K_MENU_OVERVIEW,
        K_MENU_TASKS,
        K_MENU_ANALYTICS,
        K_MENU_BENCHMARK,
        K_MENU_LIGHT_THEME,
        K_MENU_DARK_THEME,

        K_MENU_ALGORITHMS,
        K_MENU_PRIORITY_ORDER,
        K_MENU_DEADLINE_ORDER,
        K_MENU_QUICK_SORT,
        K_MENU_BUBBLE_SORT,

        K_MENU_LANGUAGE,
        K_MENU_LANG_EN,
        K_MENU_LANG_ES,

        K_MENU_HELP,
        K_MENU_USER_GUIDE,
        K_MENU_ABOUT,

        K_TOAST_UNDO,

        K_NAV_OVERVIEW,
        K_NAV_TASKS,
        K_NAV_ANALYTICS,
        K_NAV_BENCHMARK,
        K_NAV_HELP,

        K_PAGE_OVERVIEW,
        K_PAGE_TASKS,
        K_PAGE_ANALYTICS,
        K_PAGE_BENCHMARK,
        K_PAGE_HELP,
        K_PAGE_WORKSPACE,

        K_SUB_OVERVIEW,
        K_SUB_TASKS,
        K_SUB_ANALYTICS,
        K_SUB_BENCHMARK,
        K_SUB_HELP,

        K_GREETING_MORNING,
        K_GREETING_AFTERNOON,
        K_GREETING_EVENING,

        K_TODAY_LABEL,
        K_OVERVIEW_TOPBAR_SUB,
        K_STATE_UNSAVED,
        K_STATE_READY,
        K_STATE_SAVED,
        K_NEW_TASK_BTN,

        K_SIDEBAR_LOGO_SUB,
        K_SIDEBAR_ROOT_PROJECTS_FMT,
        K_SIDEBAR_TASKS_ACROSS_FMT,
        K_SIDEBAR_COLLAPSE,
        K_SIDEBAR_EXPAND,

        K_STATUS_NO_FILE,
        K_STATUS_OVERDUE_FMT,
        K_STATUS_TASKS_FMT,

        K_KEY_COUNT
    };

    void setLanguage(Language lang);
    Language currentLanguage();
    const char* tr(StringKey key);

}
