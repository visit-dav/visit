#define ERROR_IN(name,action)  NULL,name,__FILE__,__LINE__,action
#define LOCATION(name)  location(name,__FILE__,__LINE__)
char *location(char *string, char *file, int linenumber);
enum ACTION
{ CONTINUE, ABORT };
void error_action(char *start, ...);
