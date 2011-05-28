/*
 *  wCmd.c
 *  wShell
 *
 *  Created by windtw on 5/28/11.
 *  Copyright 2011 Light wind. All rights reserved.
 *
 */

#include "wCmd.h"

void init_cmd(CmdList *list)
{
	list->size = 0;
	list->head = list->end = NULL;
}

void add_cmd(CmdList *list, char *buf)
{
	char *delim;
	
	CmdNode *node = malloc(sizeof(CmdNode));
	node->next = NULL;
	
	node->argc = 0;
	while ((delim = strchr(buf, ' '))) 
	{
		node->argv[node->argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		remove_blank(&buf);
	}
	
	node->argv[node->argc] = NULL;
	
	if (list->size) {
		list->end->next = node;
		list->end = node;
	}
	else
		list->head = list->end = node;
	
	list->size += 1;
}

int check_bg(CmdNode *node)
{
	if(*node->argv[node->argc-1] == '&') {
		node->argv[--(node->argc)] = NULL;
		return 1;
	} else
		return 0;
}

void clean_cmd(CmdList *list)
{
	CmdNode *curr = list->head;
	CmdNode *next;
	while (curr != NULL) 
	{
		next = curr->next;
		free(curr);
		curr = next;
	}
	free(list);
}

