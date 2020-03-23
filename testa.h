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

/* Various parts of the testa library require some type knowledge.
 * These defines are the internal representations of the C types used
 * by the various callbacks or parsing routines for the data provided
 * in the .features file to convert them to C variables.
 */
#define TESTA_TYPE_NONE_T 0
#define TESTA_TYPE_INT32_T 1
#define TESTA_TYPE_UINT32_T 2

/* Logging macros to pretty-print steps */
#define TESTA_LOG_STEP_NONE(str)					\
	do {								\
		printf("    %.*s [%s()]\n",				\
		       (int)strcspn(str, "\n"), str, __func__);		\
	} while(0)

#define TESTA_LOG_STEP_INT32(str,val)					\
	do {								\
		printf("    %.*s [%s(%d)]\n",				\
		       (int)strcspn(str, "\n"), str, __func__, val);	\
	} while(0)

#define TESTA_LOG_STEP_UINT32(str,val)					\
	do {								\
		printf("    %.*s [%s(%u)]\n",				\
		       (int)strcspn(str, "\n"), str, __func__, val);	\
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

	/* Callback function prototypes. Only one must be set, and that
	 * string provided will be parsed to give a variable of that type.
	 */
	testa_step_none_t cb_none_t;
	testa_step_int32_t cb_int32_t;
	testa_step_uint32_t cb_uint32_t;
};

/* Defines a test */
struct testa_context_t {
	uint32_t num_steps;
	/* Each of the steps gets parsed to an action */
	struct testa_step_t *steps[32];

	/* scenario runner state */
	uint32_t in_examples;

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

	/* Figure out how to cleanly split rows of data into
	 * executable iterations of each test
	 */
	printf("example str: %s", string);

	const char *delim = "|";
	char *tmp_string = strdup(string);
	// TODO: _r version
	char *token = strtok(tmp_string, delim);
	while(token) {
		printf("token: %s\n", token);
		/* TODO: store names of first set of things
		 * then store next line, execute the values against them
		 * load next line, execute next line etc
		 */

		token = strtok(NULL, delim);
	}

	free(tmp_string);

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
		printf("Feature:\n");
		return 0;
	}
	if (!strncmp(string, "  Scenario Outline", 18)) {
		// Pretty print feature name?
		printf("  Scenario Outline:\n");
		ctx->in_examples = 0;
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

	/* Append this step to the runtime list */
	ctx->scenario_steps[ctx->num_scenario_steps] = found_idx;
	ctx->scenario_step_strings[ctx->num_scenario_steps] = strdup(string);
	ctx->num_scenario_steps++;
	return 0;
}

int32_t
testa_ctx_steps_execute(struct testa_context_t *ctx,
			void *userdata)
{
	for(uint16_t i = 0; i < ctx->num_scenario_steps; i++) {
		uint16_t step_idx = ctx->scenario_steps[i];
		struct testa_step_t *step = ctx->steps[step_idx];
		char *string = ctx->scenario_step_strings[i];

		/* No variables handles early and return */
		if (step->cb_none_t) {
			step->cb_none_t(string, userdata);
			continue;
		}

		/* find first < in string to get variable value */
		char *first_gt_symbol = strstr(string, "<");
		if (!first_gt_symbol) {
			printf("variable callback without <variable> in step.\n"
			       "Ensure step in .features file has a <variable>\n");
			return -1;
		}

		int first_gt_len = strlen(first_gt_symbol);
		if (first_gt_len < 1) {
			return -5;
		}
		// move past < symbol to number
		first_gt_symbol = &first_gt_symbol[1];

		int parse_base = 0;
		if (step->cb_uint32_t) {
			uint32_t value = strtoul(first_gt_symbol, NULL, parse_base);
			int err = step->cb_uint32_t(string, value, userdata);
			if(err)
				return -5;
			continue;
		}
		if (step->cb_int32_t) {
			int32_t value = strtol(first_gt_symbol, NULL, parse_base);
			int err = step->cb_int32_t(string, value, userdata);
			if(err)
				return -5;
			continue;
		}
	}

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

	printf("steps parsed, %d steps\n", step_counter);

	/* TODO: now execute the steps with the parsed examples */
	int32_t err = testa_ctx_steps_execute(ctx, userdata);
	if(err)
		printf("error executing steps\n");

	free(line);
	fclose(file);
	return 0;
}
