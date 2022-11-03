#include "seaport_plugin.h"

/*
** Screens Utils
*/

static uint8_t skip_right(uint8_t screen_array, uint8_t probe) {
    PRINTF("Screen move RIGHT\n");
    while (!(screen_array & probe << 1)) {
        PRINTF("Screen skip RIGHT+\n");
        probe <<= 1;
    }
    probe <<= 1;
    return probe;
}

static uint8_t skip_left(uint8_t screen_array, uint8_t probe) {
    PRINTF("Screen move LEFT\n");
    while (!(screen_array & probe >> 1)) {
        PRINTF("Screen skip LEFT+\n");
        probe >>= 1;
    }
    probe >>= 1;
    return probe;
}

static bool get_scroll_direction(uint8_t screen_index, uint8_t prev_screenIndex) {
    if (screen_index > prev_screenIndex || screen_index == 0)
        return RIGHT_SCROLL;
    else
        return LEFT_SCROLL;
}

/*
**  set prevIndex to got scrolling direction
**  return probe, the bitwise screen correspondance
*/

uint8_t get_screen_index(uint8_t screenIndex,
                         uint8_t *prevIndex,
                         uint8_t probe,
                         uint8_t screen_array) {
    if (screenIndex == 0) {
        probe = FIRST_UI;
        *prevIndex = screenIndex;
        if (screen_array & FIRST_UI) return FIRST_UI;
    } else if (screenIndex == *prevIndex) {  // Happen only on last raised bitwise screen.
        probe = LAST_UI;
        if (screen_array & LAST_UI) return LAST_UI;
    }

    // Scroll to next screen
    bool scroll_direction = get_scroll_direction(screenIndex, *prevIndex);
    PRINTF("scroll_direction: %s", (scroll_direction ? "Right" : "Left"));

    // Save prev_screenIndex after all checks are done.
    *prevIndex = screenIndex;

    if (scroll_direction == RIGHT_SCROLL)
        return skip_right(screen_array, probe);
    else
        return skip_left(screen_array, probe);
}
