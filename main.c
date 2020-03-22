#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/* TODO: make this type more explicity, remove string type */
#define TESTA_TYPE_NONE_T 0
#define TESTA_TYPE_INT32_T 1
#define TESTA_TYPE_UINT32_T 2

// String for matching printf varargs style
// Enables typed parsing of complex data, with warnings on mismatch
//
// Use ATM example here for initial testing
// https://en.wikipedia.org/wiki/Cucumber_(software)

/* TODO: add more types and string type */
typedef int32_t (*testa_step_none_t)(void *userdata);
typedef int32_t (*testa_step_int32_t)(int32_t  value, void *userdata);
typedef int32_t (*testa_step_uint32_t)(uint32_t value, void *userdata);

struct testa_step_t {
	/* Match against this string to find its impl callback */
	char find_string[64];
	/* c string for type to cast to and call, eg uint32_t */
	char type[16];

	// TODO: refactor this to use offsetof() and store the data
	// into the *userdata local structure blindly? Could avoid
	// boilerplate code

	/* the function to call */
	union {
		testa_step_none_t cb_none_t;
		testa_step_int32_t cb_int32_t;
		testa_step_uint32_t cb_uint32_t;
	};
};

/* Defines a test */
struct testa_context_t {
	uint32_t num_steps;
	/* Each of the steps gets parsed to an action */
	struct testa_step_t *steps[32];
};

int32_t testa_ctx_register_steps(struct testa_context_t *ctx,
				 struct testa_step_t *steps,
				 uint32_t num_steps)
{
	/* TODO: register steps to context here */
	(void)ctx;
	(void)steps;
	(void)num_steps;

	for (uint32_t i = 0; i < num_steps; i++) {
		if(num_steps >= 32) {
			printf("hit max steps, make dynamic\n");
			return i;
		}

		ctx->steps[ctx->num_steps] = &steps[i];
		ctx->num_steps++;
		//printf("ctx adds step %d:  \"%s\"\n", i, steps[i].find_string);
	}

	return 0;
}

static inline int32_t
testa_parse_var(const char *value_str, uint32_t testa_type, void *value)
{
	/* Autodetect */
	int base = 0;

	switch (testa_type) {
	case TESTA_TYPE_INT32_T:
		*(int32_t *)value = strtol(value_str, NULL, base);
		break;
	case TESTA_TYPE_UINT32_T:
		*(uint32_t *)value = strtoul(value_str, NULL, base);
		break;
	default:
		printf("warning: %s cannot parse value string %s\n",
		       __func__, value_str);
		return -1;
	};
	return 0;
}

int32_t testa_ctx_execute_step(struct testa_context_t *ctx,
			       const char *string,
			       void *userdata)
{
	/* find correct string */
	uint32_t found = 0;
	uint32_t found_idx = 0;

	if (!strncmp(string, "Feature", 7)) {
		// Pretty print feature name?
		printf("Feature:\n");
		return 0;
	}
	if (!strncmp(string, "Scenario Outline", 16)) {
		// Pretty print feature name?
		printf("  Scenario Outline:\n");
		return 0;
	}

	for (uint32_t i = 0; i < ctx->num_steps; i++) {
		/* TODO: implement fuzzy search here? */
		if (!strstr(string, ctx->steps[i]->find_string)) {
			continue;
		}
		if (found) {
			printf("warning: duplicate find_string on idx %d\n", i);
		}

		found_idx = i;
		found++;
	}

	switch (found) {
	case 0: return -1; /* not found */
	case 1: break; /* found once, continue processing */
	default: return -2; /* error in doubled finding */
	}

	/* No variables handles early and return */
	if (!strcmp(ctx->steps[found_idx]->type, "none")) {
		ctx->steps[found_idx]->cb_uint32_t(0, userdata);
		return 0;
	}

	/* find first < in string to get variable value */
	char *first_gt_symbol = strstr(string, "<");
	if (!first_gt_symbol) {
		/* type is not "none", so this is an error */
		printf("variable callback without <variable> in step\n");
		return -1;
	}

	int first_gt_len = strlen(first_gt_symbol);
	if (first_gt_len < 2) {
		return -5;
	}
	// move past < symbol to number
	first_gt_symbol = &first_gt_symbol[1];

	const char *target_type = ctx->steps[found_idx]->type;
	if (!strcmp(target_type, "uint32_t")) {
		uint32_t value;
		int32_t err = testa_parse_var(first_gt_symbol, TESTA_TYPE_UINT32_T, &value);
		if (err) {
			printf("failed to parse %s\n", first_gt_symbol);
			return -4;
		}
		ctx->steps[found_idx]->cb_uint32_t(value, userdata);
		return 0;
	}
	if (!strcmp(target_type, "int32_t")) {
		int32_t value;
		int32_t err = testa_parse_var(first_gt_symbol, TESTA_TYPE_INT32_T, &value);
		if (err) {
			printf("failed to parse %s\n", first_gt_symbol);
			return -4;
		}
		ctx->steps[found_idx]->cb_int32_t(value, userdata);
		return 0;
	}

	printf("no valid handler for %s found, fail\n", string);
	return -3;
}

struct testa_scenario_t {
	/* Human readable name for this test. This string is not parsed.
	 *   Scenario: A user withdraws money from an ATM
	 */
	const char *name;

	/* Human readable *AND* computer parsable steps. Split each
	 * instruction with a semicolon character ; to indicate end of line.
	 * This is the typical Gherkin input steps, below is valid input:
	 *     Given User has a valid credit card
	 *     And account balance is 50
	 *     When they insert the card
	 *     And withdraws 20
	 *     Then the ATM should return 20
	 *     And balance should be 30
	 */
	char *steps;
};

int32_t
testa_run_scenario(struct testa_context_t *ctx,
		   const struct testa_scenario_t *scn,
		   void *userdata)
{
	if (!scn->steps) {
		return -1;
	}
	char *steps = strdup(scn->steps);
	if (!steps)
		return -2;

	char *save_ptr = NULL;
	char *token = strtok_r(steps, ";", &save_ptr);
	uint32_t step_counter = 0;

	while (token) {
		int32_t err = testa_ctx_execute_step(ctx, token, userdata);
		if (err) {
			printf("line %s returns err: %d\n", token, err);
		}
		step_counter++;
		token = strtok_r(NULL, ";", &save_ptr);
	}

	free(steps);
	return 0;
}

int32_t
testa_scenario_run_from_file(struct testa_context_t *ctx,
			     const char *path,
			     void *userdata)
{
	FILE *file = fopen(path, "r");
	if (!file) {
		printf("failed to load file %s\n", path);
		return -1;
	}

	uint32_t step_counter = 0;

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, file)) != -1) {
		int32_t err = testa_ctx_execute_step(ctx, line, userdata);
		if (err) {
			printf("line %s returns err: %d\n", line, err);
		}
		step_counter++;
	}

	free(line);
	fclose(file);
	return 0;
}


int32_t
atm_user_has_valid_card(void *userdata)
{
	(void)userdata;
	printf(" - in %s\n", __func__);
	return 0;
}

int32_t
atm_account_balance(int32_t value, void *userdata)
{
	(void)value;
	(void)userdata;
	printf(" - in %s, value %d\n", __func__, value);
	return 0;
}

int32_t
atm_dispense_amount(uint32_t value, void *userdata)
{
	(void)value;
	(void)userdata;
	printf(" - in %s, value %d\n", __func__, value);
	return 0;
}

int32_t
atm_resulting_balance(int32_t value, void *userdata)
{
	(void)value;
	(void)userdata;
	printf(" - in %s, value %d\n", __func__, value);
	return 0;
}

int32_t
atm_withdraw(uint32_t value, void *userdata)
{
	(void)value;
	(void)userdata;
	printf(" - in %s, value %d\n", __func__, value);
	return 0;
}

int32_t
atm_insert_card(void *userdata)
{
	(void)userdata;
	printf(" - in %s\n", __func__);
	return 0;
}

/* Array of steps here */
struct testa_step_t atm_steps[] = {
	{ .find_string = "user has a valid",	.type = "none", .cb_none_t = atm_user_has_valid_card, },
	{ .find_string = "account balance",	.type = "int32_t",  .cb_int32_t = atm_account_balance, },
	{ .find_string = "they insert the card",.type = "none", .cb_none_t = atm_insert_card, },
	{ .find_string = "withdraw",		.type = "uint32_t", .cb_uint32_t = atm_withdraw, },
	{ .find_string = "the ATM should",	.type = "uint32_t", .cb_uint32_t = atm_dispense_amount, },
	{ .find_string = "the balance",		.type = "int32_t", .cb_int32_t = atm_resulting_balance, },
};
#define num_atm_steps (sizeof(atm_steps) / sizeof(atm_steps[0]))

int
main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	struct testa_context_t ctx = {0};
	static const struct testa_scenario_t scn = {
		.name = "Scenario Test 1",
		.steps =
"Given user has a valid card;\
And account balance is <50>;\
When they insert the card;\
And withdraw <20>;\
Then the ATM should return <20>;\
And the balance should be <30>"
	};

	int32_t err = testa_ctx_register_steps(&ctx, atm_steps, num_atm_steps);
	if (err) {
		printf("error %d\n", __LINE__);
		return -1;
	}

	void *userdata = NULL;
	err = testa_run_scenario(&ctx, &scn, userdata);

	err = testa_scenario_run_from_file(&ctx, "atm_dispense.feature", userdata);

	printf("running testa: %d\n", err);
	return err;
}
