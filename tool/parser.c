#include "common.h"

#include <ctype.h>
#include <string.h>

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
}
