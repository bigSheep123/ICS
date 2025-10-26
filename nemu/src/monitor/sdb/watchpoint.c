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

#include "sdb.h"

#define NR_WP 32


static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  WP* tmp = free_;
  WP* next = free_->next;
  free_ = next;
  tmp->next = NULL;
  return tmp;
}

void add_workingPoint(WP* wp) {
  if (head == NULL) 
    head = wp;
  else  {
    head->next = wp;
  }
}

void insert_front_free(WP* wp) {
  wp->next = free_;
  free_ = wp;
}

void free_wp(int order) {
  if (head == NULL) {
    printf("No wp need to be free!\n");
    return;
  }

  WP* tmp = head;
  WP* record = NULL;
  while (tmp != NULL) {
    if (tmp->NO == order)
      break;
    record = tmp;
    tmp = tmp->next;
  }
  if (tmp == NULL){
    printf("Can't find the order wp\n");
    return;
  }

  if (record == NULL) {
    head = tmp->next;
    insert_front_free(tmp);
    return;
  }

  record->next = tmp->next;
  insert_front_free(tmp);
}


