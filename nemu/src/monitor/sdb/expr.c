/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <macro.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM,
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"-",'-'},      
  {"\\*",'*'},
  {"/",'/'},
  {"0|[1-9][0-9]*",TK_NUM},
  {"\\(",'('},
  {"\\)",')'},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65535] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  memset(tokens,0,sizeof(Token)*65535);

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
          case TK_NOTYPE:  break;
          default: 
              memccpy(tokens[nr_token].str,substr_start,substr_len,substr_len*sizeof(char));
              tokens[nr_token].type = rules[i].token_type;
              nr_token ++;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}

bool check_parentheses(int p, int q)
{
      // 1. 检查首尾是否为括号
    if (tokens[p].type != '(' || tokens[q].type != ')') {
        return false;
    }

    // 2. 初始化栈（存储左括号的索引）
    int stack[65335];  // 假设括号深度不超过1000
    int top = -1;     // 栈顶指针

    // 3. 遍历区间 [p, q]
    for (int i = p; i <= q; i++) {
        if (tokens[i].type == '(') {
            // 左括号：压栈其索引
            if (top >= 999) return false; // 栈溢出保护
            stack[++top] = i;
        } else if (tokens[i].type == ')') {
            // 右括号：检查栈是否为空
            if (top < 0) return false; // 栈空说明右括号多余
            // 弹出栈顶的左括号索引
            int left_index = stack[top--];
            // 如果是尾括号，检查是否与首括号匹配
            if (i == q && left_index != p) {
                return false; // 尾括号匹配的不是首括号
            }
        }
        // 忽略非括号字符（如数字、运算符）
    }

    // 4. 最终栈应为空（所有左括号已匹配）
    return (top == -1);
}

uint32_t charArrToUint32(char* charArr) {
  char* endptr;
  unsigned long  num;

  num = strtoul(charArr,&endptr,10); 
  return (uint32_t)num;
}


uint32_t getPosMainOp(uint32_t p,uint32_t q) {
  int candicator = 0;
  int type = 0;
  bool flag_bracket = false;
  int bracket_num = 0;
  for (int tmp = p; tmp < q; tmp++) {

    if (tokens[tmp].type == TK_NUM) {
      continue;
    }
    if (tokens[tmp].type == '(') {
      bracket_num++;
      flag_bracket = true;
      continue;
    }
    if (tokens[tmp].type == ')') {
      bracket_num--;
      if (bracket_num == 0) 
        flag_bracket = false;
      continue;
    }

    if (( tokens[tmp].type == '*' || tokens[tmp].type == '/')) {
      if (flag_bracket) {
        continue;
      } else {
        if (type == '+') {
          continue;
        } else {
          type = '*';
          candicator = tmp;
        }
      }
    }

    if (tokens[tmp].type == '+' || tokens[tmp].type == '-' ) {
      if (flag_bracket) {
        continue;
      } else {
        type = '+';
        candicator = tmp;
      }
    }
  }

  return candicator;
}

int eval(int p,int q) {
  if (p > q) {
    /* Bad expression */
    Assert(0,"input is an error,let tokens's end < tokens's start\n");
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    return charArrToUint32(tokens[p].str);
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
    int op = getPosMainOp(p,q);
    int left_val = eval(p,op-1);
    int right_val = eval(op+1,q); 

    switch (tokens[op].type) {
    case '+': return left_val + right_val;
    case '-': return left_val - right_val;
    case '*': return left_val * right_val;
    case '/': return left_val / right_val;
    default: assert(0);
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  uint32_t result = eval(0,nr_token-1);
  printf("%u\n",result);
  // TODO();

  return result;
}

// word_t expr(char *e, bool *success) {
//   if (!make_token(e)) {
//     *success = false;
//     return 0;
//   }
//   /* TODO: Insert codes to evaluate the expression. */
//   uint32_t result = eval(0,nr_token-1);
//   printf("%u\n",result);
//   // TODO();

//   return 0;
// }
