//  ===== Date Created: 14 April, 2020 ===== 

#include <stdio.h>
#include <glad/glad.h>

int main(int argc, char* argv[])
{
	if (!gladLoadGL())
	{
		// Will fail due to no context
		printf("Glad initialisation failed.\n");
		return 1;
	}
	
	return 0;
}
