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

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <time.h>
#include <memory/paddr.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = 2;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  uint64_t step  = 1;
  char* endptr = "\0";
  if (args) 
    step = strtoull(args, &endptr, 0);
  
  if (*endptr != '\0') {
    printf("si should be followed by an integer\n");
  } else {
    cpu_exec(step);
  }

  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) {
    printf("info should be followed by 'r' or 'w'\n");
    return 0;
  }

  if (strcmp(args, "r") == 0) {
    isa_reg_display();
  } else if (strcmp(args, "w") == 0) {
    char* header[4] = {"Num","Expr","Last_Value","New_Value"};
    for (int i = 0; i < ARRLEN(header); i++) {
      printf("%8s  ",header[i]);
    }
    printf("\n");
  } else {
    printf("info should be followed by 'r' or 'w'\n");
  }

  return 0;
}

int is_valid_hex(const char *str) {
  if (!str || *str == '\0') return 0;
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
    str += 2;
    if (*str == '\0')
      return 0;
  }

  while(*str) {
    if (!isxdigit((unsigned char)*str)) {
      return 0; // 发现无效字符
    }
    str++;
  }

  return 1;
}

static int cmd_x(char *args) {
  char *nums = strtok(args," ");
  char * endptr;
  long num = strtol(nums,&endptr,10);
  if (*endptr != '\0') {
    printf("input N is illegal!!!\n");
    return 0;
  }

  char *expr = strtok(NULL," ");
  if (is_valid_hex(expr)) {
    long value = strtol(expr,&endptr,16);
    if (*endptr != '\0') {
      printf("input expr is illegal!!!\n");
      return 0;
    } else {
      for (int i = 0; i < num; i ++) {
        word_t word = paddr_read(value,4);
        printf("0x%08lx:0x%08x\n",value,word);
        value += 4;
      }
    }
  } else {
    printf("input expr is illegal!!!\n");
  }
  return 0;
}

static int cmd_p(char *args) {
  bool success; 
  expr(args,&success);
  return 0;
}

static int cmd_w(char* args) {
  WP* Wpoint = new_wp();
  Wpoint->expr = args;
  add_workingPoint(Wpoint);
  return 0;
}

static int cmd_d(char* args) {
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si","Let the program execute N steps,default N=1",cmd_si},
  {"info","Print register status,print monitoring point status",cmd_info},
  {"x","for example: x N EXPR (else),'else' will be discarded",cmd_x},
  {"p","p EXPR,find the value of EXPR",cmd_p},
  {"w","w EXPR,When EXPR changes,suspend program execution",cmd_w},
  {"d","Delete the monitoring point with serial number N",cmd_d},
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%4s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
