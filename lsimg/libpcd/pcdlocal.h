//////////////////////////////////////////////////////////////////////
// pcdlocal.h

#pragma once

#define RANGE       320

extern int pcd_img_start[];
extern int pcd_def_width[];
extern int pcd_def_height[];

#define PCD_WIDTH(res,rot)		(rot&1?pcd_def_height[res]:pcd_def_width[res])
#define PCD_HEIGHT(res,rot)		(rot&1?pcd_def_width[res]:pcd_def_height[res])
#define LUT_RANGE(a)			(min(255, max(0, (a))))

int pcd_free(PCD_IMAGE* img);
void pcd_get_LUT_init(PCD_IMAGE* img);
int pcd_inter_m2(PCD_IMAGE* img);
int pcd_inter_pixels(BYTE **data, int width, int height);
int pcd_inter_lines(BYTE **data, int width, int height);
