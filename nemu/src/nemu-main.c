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

#include <common.h>
#include <stdio.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}




/*
    Thinking: unsigned result = (32-634)/65;
    ​​32、634和 65这三个数字常量（字面量）在C语言中默认的类型是 int，也就是有符号整数​​。
    这个是C语言所规定的，所以即使按照uint32_t 来处理，在最中间的过程中也是可以产生负数的。
    最后 int -> word_t 类型转化，求出结果，机器码表示是不会改变的。
*/
// test expr's function
// extern word_t expr(char *e, bool *success);
// extern void init_regex();
// word_t transStrToU32i(const char *str) {
//     char* endptr;
//     unsigned long num = strtoul(str,&endptr,10);

//     word_t res = (word_t)num;
//     return res;
// }

// int main(int argc,char* argv[]) {
//   init_regex();
//   FILE* file;
//   char line[65535+20] = {};
//   char *res = NULL;
//   char *input = NULL;

//   file = fopen("/home/dh/ysyx/ics2024/nemu/tools/gen-expr/input", "r");
//   if (file == NULL) {
//     perror("Error to open the file");
//     exit(EXIT_FAILURE);
//   }

//   int counter = 1;
//   while(fgets(line,sizeof(line),file) != NULL) {
//     res = strtok(line," ");
//     if (res != NULL) {
//         input = strtok(NULL," ");
//         input[strlen(input) -1] ='\0';
//     }

//     // if (res != NULL && input != NULL) {
//     //     printf("res: %s\n", res); 
//     //     printf("input: %s\n", input); 
//     // } else {
//     //     printf("strtok failed\n");
//     // }

//     bool test = true;
//     word_t expr_res = expr(input,&test); 
//     if (expr_res == transStrToU32i(res)) {
//         //printf("%d line is correct!!!\n",counter);
//     } else {
//         printf("%d line is failed!!!\n",counter);
//     }

//     counter++;
//   }

//   fclose(file);
//   return 0;
// }
