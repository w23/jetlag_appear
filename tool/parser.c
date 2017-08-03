#include "common.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void parseLine(ParserContext *context) {
	const char *p = context->line;
	context->tokens = 0;
	for (;;) {
		while (*p && isspace(*p) && *p != '\n') ++p;
		if (!*p) {
			context->status = Status_End;
			break;
		}

		if (*p == '\n') {
			context->status = Status_Parsed;
			break;
		}

		if (*p == ';') {
			while (*p && *p != '\n') ++p;
			context->status = Status_Parsed;
			break;
		}

		const char *start = p;
		while (*p && isgraph(*p)) ++p;
		const int tok_len = p - start;
		if (tok_len >= PARSER_MAX_TOKEN_LENGTH) {
			context->status = Status_TokenTooLong;
			break;
		}

		if (context->tokens == PARSER_MAX_LINE_TOKENS) {
			context->status = Status_TokenTooLong;
			break;
		}

		memcpy(context->token[context->tokens].str, start, tok_len);
		context->token[context->tokens].str[tok_len] = '\0';
		++context->tokens;
	}
	if (*p == '\n') ++p;
	context->line = p;
	++context->line_number;
}

int parserTokenizeLine(ParserTokenizer *tokenizer) {
	for (;;) {
		parseLine(&tokenizer->ctx);

		if (tokenizer->ctx.status == Status_End)
			return Tokenize_End;

		if (tokenizer->ctx.status != Status_Parsed) {
			MSG("Parsing error: %d at line %d",
				tokenizer->ctx.status, tokenizer->ctx.line_number);
			return 0;
		}

		if (tokenizer->ctx.tokens < 1)
			continue;

		ParserArg args[PARSER_MAX_ARGS];

		int itok;
		for (itok = 0; itok < tokenizer->line_table_length; ++itok)
			if (strcmp(tokenizer->line_table[itok].name, tokenizer->ctx.token[0].str) == 0)
				break;

		if (itok >= tokenizer->line_table_length) {
			MSG("Invalid token '%s'", tokenizer->ctx.token[0].str);
			return Tokenize_TokenWrongFormat;
		}

		const ParserLine *line = tokenizer->line_table + itok;
		const int num_args = tokenizer->ctx.tokens - 1;

		if (num_args < line->min_args || num_args > line->max_args) {
			MSG("Invalid number of arguments %d, should be [%d, %d]", num_args, line->min_args, line->max_args);
			return Tokenize_TokenWrongFormat;
		}

		for (int j = 0; j < num_args; ++j) {
			const char *arg = tokenizer->ctx.token[1+j].str;
			args[j].type = PAF_None;
			args[j].s = arg;
			args[j].slen = strlen(arg);

			for (unsigned f = 1; f < PAF__MAX; f<<=1) {
				if (!(line->args[j].type_flags & f))
					continue;

				char *endptr = 0;
				switch(f) {
					case PAF_Int:
						args[j].value.i = strtol(arg, &endptr, 10);
						if (endptr[0] != '\0')
							continue;
						break;
					case PAF_Float:
						args[j].value.f = (float)strtod(arg, &endptr);
						if (endptr[0] != '\0')
							continue;
						break;
					case PAF_Time:
						if (2 != sscanf(arg, "%d:%d", &args[j].value.time.bar, &args[j].value.time.tick)) {
							MSG("Cannot extract bar:tick from '%s'", arg);
							continue;
						}
						break;
					case PAF_Var:
						if (arg[0] != '$')
							continue;
						args[j].s = arg + 1;
						args[j].slen--;
						break;
					case PAF_String:
						break;
					default:
						continue;
				} // switch arg type
				args[j].type = f;
				break;
			} // for all possible arg types

			if (args[j].type == PAF_None && line->args[j].type_flags != PAF_None) {
				MSG("arg '%s' cant be parsed as any of %x", arg, line->args[j].type_flags);
				return Tokenize_TokenWrongFormat;
			}
		} // for all args

		break;
	} // forever

	return Tokenize_Parsed;
}
