/*
 *  wCmd.h
 *  wShell
 *
 *  Created by windtw on 5/28/11.
 *  Copyright 2011 Light wind. All rights reserved.
 *
 */

#ifndef WCMD_H
#define WCMD_H

#include <stdio.h>

#define MAXARGS 128

typedef struct cmdNode {
	int argc;
	char *argv[MAXARGS];
	struct cmdNode *next;
} CmdNode;

typedef struct cmdList {
	int size;
	CmdNode *head, *end;
} CmdList;

void init_cmd(CmdList *list);
void add_cmd(CmdList *list, char *buf);
int check_bg(CmdNode *node);
void clean_cmd(CmdList *list);

#endif