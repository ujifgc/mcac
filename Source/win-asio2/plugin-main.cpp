#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("win-asio2", "en-US")

void RegisterWinASIO2Input();

bool obs_module_load(void)
{
	RegisterWinASIO2Input();
	return true;
}
