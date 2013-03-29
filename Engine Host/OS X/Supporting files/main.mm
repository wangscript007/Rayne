//
//  main.mm
//  Rayne Player
//
//  Created by Sidney Just on 12.03.13.
//  Copyright (c) 2013 Sidney Just. All rights reserved.
//

#include <Rayne.h>

int main(int argc, char *argv[])
{
	int result = 0;
	
	for(int i=1; i<argc; i++)
	{
		if(strcmp(argv[i], "-r") == 0 && i < argc - 1)
		{
			char *path = argv[++ i];
			RN::File::AddSearchPath(path);
		}
	}
	
	try
	{
		result = NSApplicationMain(argc, (const char **)argv);
	}
	catch(RN::ErrorException e)
	{
		RN::__HandleExcption(e);
	}
	
	return result;
}
