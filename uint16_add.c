
#include "testa.h"
#include <assert.h>

struct userdata_t {
	uint16_t one;
	uint16_t two;
	uint16_t result;
	uint16_t result_u8s;
};

int32_t
add_numbers_two(const char *str, int32_t value, void *userdata)
{
	TESTA_LOG_STEP_INT32(str, value);
	struct userdata_t *ud = userdata;
	ud->two = value;
	return 0;
}

int32_t
add_numbers_one(const char *str, int32_t value, void *userdata)
{
	TESTA_LOG_STEP_INT32(str, value);
	struct userdata_t *ud = userdata;
	ud->one = value;
	return 0;
}

int32_t
add_do(const char *str, void *userdata)
{
	TESTA_LOG_STEP_NONE(str);
	struct userdata_t *ud = userdata;
	ud->result = ud->one + ud->two;

	/* u8 style adds */
	uint16_t tmp1_lo = (ud->one & UINT8_MAX);
	uint16_t tmp2_lo = (ud->two & UINT8_MAX);
	uint16_t tmp1_hi = (ud->one >> 8) & UINT8_MAX;
	uint16_t tmp2_hi = (ud->two >> 8) & UINT8_MAX;
	uint16_t res = ((tmp1_hi + tmp2_hi) << 8) + (tmp1_lo + tmp2_lo);
	ud->result_u8s = res;

	return 0;
}

int32_t
add_result_equals(const char *str, int32_t value, void *userdata)
{
	TESTA_LOG_STEP_INT32(str, value);
	struct userdata_t *ud = userdata;

	assert(ud->result == value);
	assert(ud->result_u8s == value);

	return 0;
}

/* Array of steps here */
struct testa_step_t atm_steps[] = {
	{ .find_string = "a number",		.cb_int32_t = add_numbers_one},
	{ .find_string = "another number",	.cb_int32_t = add_numbers_two},
	{ .find_string = "added together",	.cb_none_t  = add_do},
	{ .find_string = "the result",		.cb_int32_t = add_result_equals},
};
#define num_atm_steps (sizeof(atm_steps) / sizeof(atm_steps[0]))


int main()
{
	struct testa_context_t ctx = {0};
	int32_t err = testa_ctx_register_steps(&ctx, atm_steps, num_atm_steps);
	if (err) {
		printf("error registering steps\n");
		return -1;
	}

	struct userdata_t ud = {0};
	const char *path = "add_numbers.feature";
	err = testa_scenario_run_from_file(&ctx, path, &ud);
	if (err) {
		printf("error running test file %s\n", path);
		return -1;
	}

	return err;
}
