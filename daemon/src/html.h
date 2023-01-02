#ifndef WM_RENDERER
#define WM_RENDERER

typedef struct
{
	const char* key;
	const char* value;
} key_value_t;

char* view(const key_value_t* kvps);
#endif
