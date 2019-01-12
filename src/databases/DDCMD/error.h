#define ERROR_IN(name,action)  NULL,name,__FILE__,__LINE__,action
#define LOCATION(name)  location(name,__FILE__,__LINE__)
char *location(const char *string, const char *file, int linenumber);
enum ACTION
{ CONTINUE, ABORT };
void error_action(const char *start, ...);
