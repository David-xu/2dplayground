#include "pub.h"

int main(int argc, char *argv[])
{
    pg_win_ab_t win = {0};
    TCHAR *title = _T("2d playground");
    screen_init(&win, 800, 600, title);

	while (win.exit_sig == 0) {
		screen_dispatch();

        screen_clean(&win, -0);

		screen_update(&win);
	}

    return 0;
}