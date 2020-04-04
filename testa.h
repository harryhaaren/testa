#pragma once

/* Header file for BDD type workflows, think like Cucumber/Behave.
 * The testa loads and runs a ".features" file, and executes the
 * steps in it against the registered step handler callbacks.
 * The end result is easily templatable, readable and runnable
 * descriptions of program behaviour - enabling BDD.
 *
 * Comments? Harry van Haaren <harryhaaren@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

/* Various parts of the testa library require some type knowledge.
 * These defines are the internal representations of the C types used
 * by the various callbacks or parsing routines for the data provided
 * in the .features file to convert them to C variables.
 */
#define TESTA_TYPE_NONE_T 0
#define TESTA_TYPE_INT32_T 1
#define TESTA_TYPE_UINT32_T 2

#define TESTA_EXAMPLE_VALUES_MAX 32
#define TESTA_EXAMPLE_VALUES_NAME_MAX 32

#define TESTA_RESET "\033[;0m"
#define TESTA_RED   "\033[1;31m"
#define TESTA_BLUE  "\033[0;34m"
#define TESTA_GREEN "\033[;32m"

/* Logging macros to pretty-print steps */
#define TESTA_LOG_STEP_NONE(str)					\
	do {								\
		printf("    %s%.*s%s\n",				\
		       TESTA_GREEN, (int)strcspn(str, "\n"), str,	\
		       TESTA_RESET);\
	} while(0)

#define TESTA_LOG_STEP_INT32(str,val)					\
	do {								\
		printf("    %s%.*s:%s %d\n",			\
		       TESTA_GREEN, (int)strcspn(str, "\n"), str,	\
		       TESTA_RESET, val);				\
	} while(0)

#define TESTA_LOG_STEP_UINT32(str,val)					\
	do {								\
		printf("    %s%.*s:%s %u\n",			\
		       TESTA_GREEN, (int)strcspn(str, "\n"), str,	\
		       TESTA_RESET, val);				\
	} while(0)

#define TESTA_LOG_STEP_WARN(str_reason)				\
	do {								\
		printf("          %s%s() [WARN] %s%s\n", TESTA_BLUE,	\
		       __func__, str_reason, TESTA_RESET);		\
	} while(0)

#define TESTA_LOG_STEP_FAIL(str_reason)				\
	do {								\
		printf("          %s%s() [FAIL] %s%s\n", TESTA_RED,	\
		       __func__, str_reason, TESTA_RESET);		\
	} while(0)

/**
 * Function prototypes to be implemented by the user of Testa.
 * Each function recieves the string that caused the callback to be called,
 * the value (if any) to test with, and a void pointer for userdata.
 */
typedef int32_t (*testa_step_none_t)(const char *str, void *userdata);
typedef int32_t (*testa_step_int32_t)(const char *str,int32_t  value, void *userdata);
typedef int32_t (*testa_step_uint32_t)(const char *str, uint32_t value, void *userdata);

struct testa_step_t {
	/* Match against this string to find its impl callback */
	char find_string[64];

	uint32_t example_value_idx;

	/* Callback function prototypes. Only one must be set, and that
	 * string provided will be parsed to give a variable of that type.
	 */
	testa_step_none_t cb_none_t;
	testa_step_int32_t cb_int32_t;
	testa_step_uint32_t cb_uint32_t;
};

struct testa_example_value_t {
	char name[TESTA_EXAMPLE_VALUES_NAME_MAX];
	uint32_t num_values;
	int64_t values[TESTA_EXAMPLE_VALUES_MAX];
};

/* Defines a test */
struct testa_context_t {
	uint32_t num_steps;
	uint32_t step_to_ex_value_idx;
	/* Each of the steps gets parsed to an action */
	struct testa_step_t *steps[32];

	/* scenario runner state */
	uint32_t in_examples;
	uint32_t in_examples_hdrs_parsed;
	uint32_t num_example_values;
	uint32_t num_example_values_rows;
	struct testa_example_value_t example_values[32];

	/* scenario parsing stores steps and then re-execute them */
	uint16_t scenario_steps[32];
	char *scenario_step_strings[32];
	uint16_t num_scenario_steps;
};

int32_t
testa_ctx_register_steps(struct testa_context_t *ctx,
			 struct testa_step_t *steps,
			 uint32_t num_steps)
{
	for (uint32_t i = 0; i < num_steps; i++) {
		if(num_steps >= 32) {
			printf("hit max steps, make dynamic\n");
			return i;
		}

		ctx->steps[ctx->num_steps] = &steps[i];
		ctx->num_steps++;
	}

	return 0;
}

int32_t
testa_ctx_example_parsing(struct testa_context_t *ctx,
			  const char *string)
{
	(void)ctx;

	/* remove whitespaces for cleaner data */
	int32_t in_len = strlen(string);
	char *tmp_string = strdup(string);

	int j = 0;
	for(int i = 0; i < in_len; i++) {
		char c = string[i];
		if (!isspace(c)) {
			tmp_string[j++] = c;
		}
	}
	tmp_string[j] = '\0';

	// TODO: _r version
	const char *delim = "|";
	char *token = strtok(tmp_string, delim);
	int token_idx = 0;
	while(token) {
		//printf("token: %s\n", token);
		/* TODO: store names of first set of things
		 * then store next line, execute the values against them
		 * load next line, execute next line etc
		 */
		if(!ctx->in_examples_hdrs_parsed) {
			/* allocate a name/values[] pair */
			int exidx = ctx->num_example_values;
			struct testa_example_value_t *exv =
				&ctx->example_values[exidx];
			snprintf(exv->name, TESTA_EXAMPLE_VALUES_NAME_MAX,
				 "%s", token);
			ctx->num_example_values++;
			/* printf("example name %s\n", exv->name); */
		} else {
			/* put value into num example values item */
			struct testa_example_value_t *exv =
				&ctx->example_values[token_idx];
			exv->values[exv->num_values] = strtol(token, NULL, 0);
			exv->num_values++;
#if 0
			printf("name %s gets token %s, value %ld, at idx %d\n",
			       exv->name, token, exv->values[exv->num_values],
			       exv->num_values);
#endif
		}

		token_idx++;
		token = strtok(NULL, delim);
	}

	free(tmp_string);

	if(ctx->in_examples_hdrs_parsed) {
		ctx->num_example_values_rows++;
	}

	/* Note that headers are now parsed, so move to values next */
	ctx->in_examples_hdrs_parsed = 1;

	return 0;
}

int32_t
testa_ctx_steps_parse(struct testa_context_t *ctx,
		      const char *string)
{
	/* find correct string */
	uint32_t found = 0;
	uint32_t found_idx = 0;

	if (!strncmp(string, "Feature", 7)) {
		// Pretty print feature name?
		printf("%s", string);
		return 0;
	}
	if (!strncmp(string, "  Scenario Outline", 18)) {
		// Pretty print feature name?
		printf("  %s", string);
		ctx->in_examples = 0;
		ctx->in_examples_hdrs_parsed = 0;
		ctx->step_to_ex_value_idx = 0;
		ctx->num_example_values_rows = 0;
		return 0;
	}
	if (!strncmp(string, "    Examples", 12)) {
		ctx->in_examples = 1;
		return 0;
	}

	/* empty lines */
	int len = strlen(string);
	if (len < 2) {
		ctx->in_examples = 1;
		return 0;
	}

	if (ctx->in_examples) {
		testa_ctx_example_parsing(ctx, string);
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

	/* If the string has a < char, it must be using a value, so
	 * consume one of the values from the examples table */
	char *consumes_value = strstr(string, "<");
	if (consumes_value) {
		ctx->steps[found_idx]->example_value_idx =
			ctx->step_to_ex_value_idx++;
	}

	/* Append this step to the runtime list */
	ctx->scenario_steps[ctx->num_scenario_steps] = found_idx;
	ctx->scenario_step_strings[ctx->num_scenario_steps] = strdup(string);
	ctx->num_scenario_steps++;
	return 0;
}

int32_t
testa_ctx_steps_execute(struct testa_context_t *ctx,
			uint32_t example_idx,
			void *userdata)
{
	int soft_error_count = 0;

	for(uint16_t i = 0; i < ctx->num_scenario_steps; i++) {
		uint16_t step_idx = ctx->scenario_steps[i];
		struct testa_step_t *step = ctx->steps[step_idx];
		char *string = ctx->scenario_step_strings[i];

		/* No variables handles early and return */
		if (step->cb_none_t) {
			step->cb_none_t(string, userdata);
			continue;
		}

		int64_t exv_idx = ctx->steps[step_idx]->example_value_idx;
		int64_t v = (int64_t)ctx->example_values[exv_idx].values[example_idx];

		if (step->cb_uint32_t) {
			int err = step->cb_uint32_t(string, v, userdata);
			if (err == 0)
				continue;
			if (err == -1) {
				soft_error_count++;
				continue;
			}

			printf("code error, fix code in test\n");
			return -5;
		}
		if (step->cb_int32_t) {
			int err = step->cb_int32_t(string, v, userdata);
			if(err)
				return -5;
			continue;
		}
	}

	return 0;
}

int32_t
testa_ctx_steps_execute_all(struct testa_context_t *ctx, void *userdata)
{
	int ret = 0;
	for (uint32_t i = 0; i < ctx->num_example_values_rows; i++) {
		printf("      Example %d\n", i);
		ret += testa_ctx_steps_execute(ctx, i, userdata);
	}
	return ret;
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
		int32_t err = testa_ctx_steps_parse(ctx, line);
		if (err) {
			if(err == -1) { /* not found */
				printf("No handler found for this step"
				       "add callback code to handle it!\n  %s",
				       line);
			} else {
				printf("line %s returns err: %d\n", line, err);
			}
		}
		step_counter++;
	}
	//printf("steps parsed, %d steps\n", step_counter);

	/* TODO: now execute the steps with the parsed examples */
	int32_t err = testa_ctx_steps_execute_all(ctx, userdata);
	if(err)
		printf("error executing steps\n");

	free(line);
	fclose(file);
	return 0;
}
