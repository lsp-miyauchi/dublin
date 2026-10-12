#include <windows.h>
#include "Cabinet.hpp"
#include <stdio.h>
#include "resource.h"

void main(int argc, char **argv)
{
	CResourceCabinet cabTest;

	// First create the FDI context
	if (!cabTest.CreateFDIContext())
	{
		printf("ERROR: Could not create FDI context\n");
		return;
	}

	// Verify if the cab resource is valid
	if (!cabTest.IsResourceCabinet(ID_CAB_TEST, "CABFILE"))
	{
		printf("ERROR: 'ID_CAB_TEST' is not a valid cabinet\n");
		return;
	}

	// Extract all files in the current directory
	if (cabTest.CopyFromResource(ID_CAB_TEST, "CABFILE", "Extract"))
		printf("Extracted all files succesfully!\n");
	else
		printf("ERROR: Not all files could be extracted\n");

	// Destroy the FDI context
	cabTest.DestroyFDIContext();
}