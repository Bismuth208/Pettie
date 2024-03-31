#include "async_profiler.h"

#include "async_printf.h"
#include "arch/debug_tools_arch.h"

//
#include <assert.h>

// Storage for profile points
static volatile profile_time_t local_profile_time[CONFIG_PROFILER_POINTS_MAX] = {0u};

void IRAM_ATTR
profile_point(profile_point_t state, uint32_t point_id)
{
	assert(point_id < CONFIG_PROFILER_POINTS_MAX);

	// Get system time as soon as possible
	profile_time_t local_time = profiler_get_us_time();
	// Use local varialbe to increase Register/Cache data locality
	profile_time_t profile_time_point = (profile_time_t)local_profile_time[point_id];

	if (state == profile_point_start) {
		profile_time_point = local_time;
	} else {
		profile_time_point = (local_time - profile_time_point);
		// FIXME: here possible overflow!
		async_printf(async_print_type_u32, "profile: %lu ", point_id);
		async_printf(async_print_type_u32, "time: %lu us\n", profile_time_point);
	}

	local_profile_time[point_id] = profile_time_point;
}
