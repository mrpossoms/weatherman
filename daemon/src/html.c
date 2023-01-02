#include "html.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* view(const key_value_t* kvps)
{
	int fd = open("view.html", O_RDONLY);
	
	if (fd < 0) { return NULL; }

	size_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	char* view_template = (char*)malloc(size);

	read(fd, view_template, size);

	for (unsigned i = 0; i < size; i++)
	{
		char c = view_template[i];

		switch(c)
		{
			case '{': // might match a kvp
			{
				int key_len = strlen(kvps[0].key);
				if (0 == strncmp(view_template + i + 1, kvps[0].key, key_len) &&
					view_template[i + key_len + 1] == '}')
				{
					fputs(kvps[0].value, stdout);
					i += key_len + 2;
				}
			} break;

			default:
				putc(view_template[i], stdout);
		}

	}
abort:
	free(view_template);
	return NULL;
}