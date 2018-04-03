//
// numb.c
// by Benedict Henshaw (2018-04-01)
//
// A calculator utility.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint64_t u64;
typedef double f64;
typedef _Bool bool;
#define true 1
#define false 0

//
// Lexer.
//

#define TOKEN_BAD 'b'
#define TOKEN_END 'e'
#define TOKEN_NUMBER 'n'

typedef struct
{
    char * start;
    int length;
    f64 value;
    u8 kind;
}
Token;

char * stream;
Token token;

bool next_token()
{
    if (*stream >= '0' && *stream <= '9')
    {
        token.start = stream;
        // The next token is a number.
        f64 whole_portion = 0.0;
        f64 fractional_portion = 0.0;
        f64 fractional_multiplier = 0.1;
        bool fractional = false;
        while ((*stream >= '0' && *stream <= '9') || *stream == '.')
        {
            if (*stream == '.')
            {
                fractional = true;
                ++stream;
            }
            else if (fractional)
            {
                fractional_portion += (*stream++ - '0') * fractional_multiplier;
                fractional_multiplier *= 0.1;
            }
            else
            {
                whole_portion *= 10.0;
                whole_portion += *stream++ - '0';
            }
        }
        token.length = stream - token.start;
        token.kind = TOKEN_NUMBER;
        token.value = whole_portion + fractional_portion;
    }
    else if (*stream == ' ' || *stream == '\t')
    {
        // The next token is whitespace, so skip it.
        while (*stream == ' ' || *stream == '\t') ++stream;
        next_token();
    }
    else if (*stream == '+' || *stream == '-' ||
             *stream == '*' || *stream == '/' ||
             *stream == '(' || *stream == ')')
    {
        // The next token is an operator.
        token.start = stream;
        token.length = 1;
        token.kind = *stream++;
    }
    else if (*stream == '\0')
    {
        // The next token is the end of the character stream.
        token.kind = TOKEN_END;
        token.start = stream;
        token.length = 1;
        return false;
    }
    else
    {
        // The next token is an unknown character.
        if (token.kind == TOKEN_BAD)
        {
            ++token.length;
        }
        else
        {
            token.kind = TOKEN_BAD;
            token.start = stream;
            token.length = 1;
        }
        return false;
    }
    return true;
}

//
// Direct parser.
//
// This parser will parse and evaluate expressions,
// producing a final numerical value.
//

bool token_is(int kind)
{
    return token.kind == kind;
}

bool match_token(int kind)
{
    if (token_is(kind))
    {
        next_token();
        return true;
    }
    else
    {
        return false;
    }
}

// Numerical values and brackets.
f64 parse_level_three()
{
    if (token_is(TOKEN_NUMBER))
    {
        f64 value = token.value;
        next_token();
        return value;
    }
    else if (match_token('('))
    {
        f64 parse_level_zero();
        f64 value = parse_level_zero();
        if (match_token(')'))
        {
            return value;
        }
        else
        {
            printf("Invalid Expression: Expected ')'.\n");
            exit(1);
        }
    }
    else
    {
        printf("Invalid Expression: Expected number or '('.\n");
        exit(1);
    }
}

// Unary plus and minus.
f64 parse_level_two()
{
    if (match_token('-'))
    {
        return -parse_level_two();
    }
    else if (match_token('+'))
    {
        return parse_level_two();
    }
    else
    {
        return parse_level_three();
    }
}

// Multiplication and division.
f64 parse_level_one()
{
    f64 value = parse_level_two();
    while (token_is('*') || token_is('/'))
    {
        int operator = token.kind;
        next_token();
        f64 r_value = parse_level_one();
        if (operator == '*')
        {
            value *= r_value;
        }
        else if (operator == '/')
        {
            value /= r_value;
        }
        else
        {
            printf("Invalid Expression: Expected '*' or '/'.\n");
            exit(1);
        }
    }
    return value;
}

// Addition and subtraction.
f64 parse_level_zero()
{
    f64 value = parse_level_one();
    while (token_is('+') || token_is('-'))
    {
        int operator = token.kind;
        next_token();
        f64 r_value = parse_level_one();
        if (operator == '+')
        {
            value += r_value;
        }
        else if (operator == '-')
        {
            value -= r_value;
        }
        else
        {
            printf("Invalid Expression: Expected '+' or '-'.\n");
            exit(1);
        }
    }
    return value;
}

f64 parse_string(char * string)
{
    stream = string;
    next_token();
    f64 result = parse_level_zero();
    return result;
}

//
// Main.
//

int main(int argument_count, char ** arguments)
{
    if (argument_count < 2)
    {
        // Run in interpreted mode.
        char buffer[512];
        char * input;
        while (true)
        {
            input = fgets(buffer, 512, stdin);
            if (!input) break;
            printf("= %g\n", parse_string(input));
        }
    }
    else if (argument_count == 2)
    {
        // Evaluate, print, and exit.
        printf("%g\n", parse_string(arguments[1]));
    }
    else
    {
        printf("Invalid expression!\nBe sure to put quotes around your expression.\n");
    }
}
