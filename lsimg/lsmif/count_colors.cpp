///////////////////////////////////////////////////////////////////////
// count_colors.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define NODESINALIST			2048
#define MAXTREEDEPTH			8

typedef struct tagCOLORSLIST {
	BYTE red;
	BYTE green;
	BYTE blue;
	DWORD count;
} COLORSLIST;

typedef struct tagNODEINFO {
	BYTE level;
	DWORD number_unique;
	COLORSLIST* list;
    struct tagNODEINFO* child[8];
} NODEINFO;

typedef struct tagNODES {
	NODEINFO nodes[NODESINALIST];
	struct tagNODES* next;
} NODES;

typedef struct tagCUBEINFO {
	NODEINFO* root;
	UINT progress, colors;
	unsigned int free_nodes;
	NODEINFO* node_info;
	NODES* node_list;
} CUBEINFO;


///////////////////////////////////////////////////////////////////////
//

static NODEINFO* InitializeNode(CUBEINFO* color_cube, const UINT level)
{
	register int i;
	NODEINFO* node_info;

	if (color_cube->free_nodes == 0)
    {
		NODES* nodes;

		// Allocate a new nodes of nodes.
		nodes = (NODES*)malloc(sizeof(NODES));
		if (nodes == NULL)
			return NULL;

		nodes->next = color_cube->node_list;
		color_cube->node_list = nodes;
		color_cube->node_info = nodes->nodes;
		color_cube->free_nodes = NODESINALIST;
	}
	color_cube->free_nodes--;
	node_info = color_cube->node_info++;
	for (i = 0; i < 8; i++)
		node_info->child[i] = NULL;
	node_info->level = level;
	node_info->number_unique = 0;
	node_info->list = NULL;
	return node_info;
}

static void DestroyList(const NODEINFO* node_info)
{
	register unsigned int id;
	for (id = 0; id < 8; id++)
	{
		if (node_info->child[id] != NULL)
			DestroyList(node_info->child[id]);
	}
	if (node_info->level == MAXTREEDEPTH)
	{
		if (node_info->list != NULL)
			free(node_info->list);
	}
}

static DWORD countcolors_indexed(mif& image)
{
	int* marker = new int[256];
	memset(marker, 0, sizeof(int) * 256);
	RGBA* p = image.m_pixels;
	int i;
	for (i = 0; i < image.m_packets; i++) {
		marker[p->alpha] = 1;
		p++;
	}

	DWORD cnt = 0;
	for (i = 0; i < 256; i++) {
		if (marker[i])
			cnt++;
		p++;
	}
	return cnt;
}

DWORD countcolors(mif& image)
{
	if (image.m_bits <= 8)
		return countcolors_indexed(image);

	register UINT id, index, level;
	register DWORD i = image.m_width;
	register DWORD j = image.m_height;

	CUBEINFO color_cube;
	color_cube.node_list = NULL;
	color_cube.colors = 0;
	color_cube.free_nodes = 0;
	color_cube.root = InitializeNode(&color_cube, 0);
	if (color_cube.root == NULL)
		return 0;

	NODEINFO* node_info;
	NODES* nodes;
	DWORD cnt = image.m_packets;
	register RGBA* p = image.m_pixels;
	for (i = 0; i < cnt; i++)
	{
		// Start at the root and proceed level by level.
		node_info = color_cube.root;
		index = MAXTREEDEPTH - 1;
		for (level = 1; level <= MAXTREEDEPTH; level++)
		{
			id = ((p->red >> index) & 1) << 2 |
				 ((p->green >> index) & 1) << 1 |
				 ((p->blue >> index) & 1);
			if (node_info->child[id] == NULL)
			{
				node_info->child[id] = InitializeNode(&color_cube,level);
				if (node_info->child[id] == NULL)
					return 0;
			}
			node_info = node_info->child[id];
			index--;
			if (level != MAXTREEDEPTH)
				continue;
			for (j = 0; j < node_info->number_unique; j++)
			{
				if ((p->red == node_info->list[j].red) &&
					(p->green == node_info->list[j].green) &&
					(p->blue == node_info->list[j].blue))
					break;
			}
			if (j < node_info->number_unique)
			{
				node_info->list[j].count++;
				continue;
			}
			if (node_info->number_unique == 0)
				node_info->list = (COLORSLIST*)malloc(sizeof(COLORSLIST));
			else
				node_info->list = (COLORSLIST*)realloc(node_info->list,
													(j + 1) * sizeof(COLORSLIST));
			if (node_info->list == NULL)
				return 0;

			node_info->list[j].red = p->red;
			node_info->list[j].green = p->green;
			node_info->list[j].blue = p->blue;
			node_info->list[j].count++;
			node_info->number_unique++;
			color_cube.colors++;
		}
		p++;
	}

	DestroyList(color_cube.root);
	do {
		nodes = color_cube.node_list->next;
		free(color_cube.node_list);
		color_cube.node_list = nodes;
	} while (color_cube.node_list != NULL);

	return color_cube.colors;
}
