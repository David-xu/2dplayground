#include "pub.h"

pg_simple_2d_ctx_t g_ctx;

int main(int argc, char *argv[])
{
    MSG msg;

    cpkl_ri_seed();

    pg_simple_2d_init(&g_ctx);
#if 0
	while (win.exit_sig == 0) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);

		screen_dispatch();

        screen_clean(&win, -0);

		screen_update(&win);
	}
#endif
    while (g_ctx.window.exit_sig == 0) {
        if (GetMessage(&msg, NULL, 0, 0) <= 0) {
            cpkl_printf("last err %d\n", GetLastError());
            g_ctx.window.exit_sig = 1;
        }
        DispatchMessage(&msg);
    }

    return 0;
}