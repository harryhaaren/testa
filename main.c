// Use ATM example here for initial testing
// https://en.wikipedia.org/wiki/Cucumber_(software)

#include "testa.h"
#include <assert.h>

/* Program specific data-structures, usually in a header file */
struct bank_account_t {
	uint32_t card_is_valid;
	int32_t balance;
};

struct atm_t {
	uint8_t card_inserted;
	struct bank_account_t *account_for_card;
};

struct userdata_t {
	struct bank_account_t account;
	struct atm_t atm;
};


int32_t
atm_user_has_valid_card(const char *str, void *userdata)
{
	TESTA_LOG_STEP_NONE(str);
	struct userdata_t *ud = userdata;
	ud->account.card_is_valid = 1;
	return 0;
}

int32_t
atm_account_balance(const char *str, int32_t value, void *userdata)
{
	TESTA_LOG_STEP_INT32(str, value);
	struct userdata_t *ud = userdata;
	ud->account.balance = value;
	return 0;
}

int32_t
atm_insert_card(const char *str, void *userdata)
{
	TESTA_LOG_STEP_NONE(str);
	struct userdata_t *ud = userdata;
	ud->atm.card_inserted = 1;
	return 0;
}

int32_t
atm_dispense_amount(const char *str, uint32_t value, void *userdata)
{
	TESTA_LOG_STEP_UINT32(str, value);
	struct userdata_t *ud = userdata;
	(void)ud;
	// highjack or dependency-inject the implementation of the
	// dispense call here, to validate dispensing is the right amount
	return 0;
}

int32_t
atm_withdraw(const char *str, uint32_t value, void *userdata)
{
	TESTA_LOG_STEP_UINT32(str, value);
	struct userdata_t *ud = userdata;
	// call into account code here, making withdrawl of 'value' amount

	if (value > 200) { // atm won't allow this
		TESTA_LOG_STEP_FAILED("Cannot withdraw more than 200");
		return -1;
	}

	int tmp_val = (int)value;

	if((ud->account.balance - tmp_val) >= 0)
		ud->account.balance -= tmp_val;

	return 0;
}

int32_t
atm_resulting_balance(const char *str, int32_t value, void *userdata)
{
	TESTA_LOG_STEP_INT32(str, value);
	struct userdata_t *ud = userdata;
	assert(ud->account.balance == value);
	return 0;
}

/* Array of steps here */
struct testa_step_t atm_steps[] = {
	{ .find_string = "user has a valid",	.cb_none_t = atm_user_has_valid_card},
	{ .find_string = "account balance",	.cb_int32_t = atm_account_balance},
	{ .find_string = "insert the card",	.cb_none_t = atm_insert_card},
	{ .find_string = "withdraw",		.cb_uint32_t = atm_withdraw},
	{ .find_string = "the ATM should",	.cb_uint32_t = atm_dispense_amount},
	{ .find_string = "the balance",		.cb_int32_t = atm_resulting_balance},
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
	const char *path = "atm_dispense.feature";
	err = testa_scenario_run_from_file(&ctx, path, &ud);
	if (err) {
		printf("error running test file %s\n", path);
		return -1;
	}

	return err;
}
