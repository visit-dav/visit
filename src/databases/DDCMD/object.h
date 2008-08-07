#ifndef OBJECT_H
#define OBJECT_H

#define TEST(A,B)  (B.mod != 0 && (A%B.mod )==B.offset)
#define TEST0(A,B)  (B != 0 && (A%B)==0)
enum OBJECTTYPES
{ DOUBLE, INT, SHORT, STRING, FILETYPE, FILELIST, LITERAL };
enum MODE
{ ASCII, BINARY };
enum OBJECTACTION
{ ABORT_IF_NOT_FOUND, IGNORE_IF_NOT_FOUND, WARN_IF_NOT_FOUND };
typedef struct
{
	FILE *file;
	char *name;
} OBJECTFILE;

typedef struct object_st	/* object structure for parsing routine */
{
	char *name;
	char *_class;
	char *value;
	char **valueptr;
} OBJECT;

typedef struct field_st		/* field structure for parsing routine */
{
	int n;
	int size;
	int element_size;
	void *v;
} FIELD;

typedef struct FileList_st
{
	char *name;
	int start, end;
} FileList;
typedef struct pack_buf_st
{
	int n, nobject,mobject;
	char *buffer;
} PACKBUF;

OBJECT *object_initialize(char *name, char *_class, int size);
char *object_read(OBJECTFILE ofile);
FIELD object_parse(OBJECT *, char *, int, char *);
OBJECT *object_find(char *, char *);
OBJECT *object_find2(char *, char *, enum OBJECTACTION);
void object_free(OBJECT *);
int object_get(OBJECT *, char *, void *, int, int, char *);
int object_getv(OBJECT*c, char *member, void **, int type);
int object_getv1(OBJECT*c, char *member, void **, int type);
void object_compilefilesubset(const char *filename, int first, int last);
void object_replacekeyword(OBJECT *object,char *keyword,char* keywordvalue);
void object_Bcast(int root,int comm);
int object_lineparse(char *, OBJECT *);
#endif // #ifndef OBJECT_H
