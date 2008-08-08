#ifndef ALPHA
#define EXTRA_ARGS ,...
#else
#define EXTRA_ARGS
#endif

#define MAXKEYWORDS 4096
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "object.h"
#include "error.h"

#ifdef WIN32
#define strtok_r(s,sep,lasts) (*(lasts)=strtok((s),(sep)))
#define strcasecmp stricmp
#endif

static const char *filenames[] = { "object.data", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static int nfiles = 1;
static int nobject = 0, mobject = 0;
static int niobject = 0, miobject = 0;
static OBJECT *object = NULL;
static OBJECT **object_list = NULL;
int object_lineparse(char *, OBJECT *);
OBJECTFILE object_fopen(const char *filename, const char *mode);
void object_fclose(OBJECTFILE file);
void object_set(char *get, ... )
{
	va_list ap;
	char *what, *string, *list, *file;
	void **ptr;
	va_start(ap, get);
	what = strdup(get);
	string = strtok(what, " ");
	while (string != NULL)
	{
		ptr = va_arg(ap, void **);
		if (strcmp(string, "files") == 0)
		{
			list = strdup((char *)ptr);
			nfiles = 0;
			file = strtok(list, " ");
			while (file != NULL)
			{
				filenames[nfiles++] = strdup(file);
				file = strtok(NULL, " ");
			}
		}
		string = strtok(NULL, " ");
	}
	va_end(ap);
}

void trim(char *string)
{
	int i, j, k, l;
	if (string == NULL) return;
	i = -1;
	while (string[++i] == ' ');
	j = strlen(string);
	while (string[--j] == ' ');
	k = 0;
	for (l = i; l <= j; l++)
		string[k++] = string[l];
	string[k] = 0x0;
}

int object_get(OBJECT*object, char *name, void *ptr, int type, int length, char *dvalue)
{
	int l;
	FIELD f;
	f = object_parse(object, name, type, dvalue);
	if (f.n == 0 || length <= 0) return 0;
	l = MIN(f.n, length);
	memmove(ptr, f.v, l*f.element_size);
	return f.n;
}

int object_getv(OBJECT*object, char *name, void **pptr, int type)
{
	FIELD f;
	void *ptr;
	f = object_parse(object, name, type, NULL);
	ptr = malloc(f.size);
	memmove(ptr, f.v, f.size);
	*pptr = (void *)ptr;
	return f.n;
}

int object_getv1(OBJECT*object, char *name, void **pptr, int type)
{
	FIELD f;
	void *ptr;
	f = object_parse(object, name, type, (char *)IGNORE_IF_NOT_FOUND);
	if (f.n > 0)
	{
		ptr = malloc(f.size);
		memmove(ptr, f.v, f.size);
		*pptr = (void *)ptr;
	}
	else
	{
		*pptr = (void *)NULL;
	}
	return f.n;
}

int object_test(OBJECT*object, char *name, char **value_ptr, int *nvalue_ptr)
{
	static int lbuff = 0;
	static char *buffer = NULL, *line = NULL;
	static int nkeyword = 0;
	static char *keyword = NULL;
	static char *value;
	static int nvalue;
	int found;
	int nname;
	char *name_list[16], *name_save;
	char *string;
	int i, lstring;
	char *ptr;
	value = NULL;
	nvalue = 0;
	if (value_ptr != NULL) value = *value_ptr;
	if (nvalue != 0) nvalue = *nvalue_ptr;
	name_save = strdup(name);
	ptr = strtok(name_save, ";");
	nname = 0;
	while (ptr != NULL && nname < 16)
	{
		trim(ptr);
		name_list[nname] = ptr;
		ptr = strtok(NULL, ";");
		nname++;
	}
	string = object->value;
	lstring = strlen(string) + 1;
	if (lstring > lbuff)
	{
		lbuff = lstring;
		buffer = (char*) realloc(buffer, lstring*sizeof(char));
		line = (char*) realloc(line, lstring*sizeof(char));
	}
	strcpy(buffer, string);
	ptr = strtok(buffer, ";");
	while (ptr != NULL)
	{
		strcpy(line, ptr);
		trim(line);
		ptr = strchr(line, '=');
		*ptr = 0x0;
		while (nkeyword < strlen(line) + 1)
		{
			nkeyword += 256;
			keyword = (char*) realloc(keyword, nkeyword);
		}
		strncpy(keyword, line, nkeyword);
		trim(keyword);
		ptr++;
		while (nvalue < strlen(ptr) + 1)
		{
			nvalue += 256;
			value = (char*) realloc(value, nvalue);
		}
		strncpy(value, ptr, nvalue);
		trim(value);
		found = 0;
		for (i = 0; i < nname; i++)
		{
			if (strcmp(name_list[i], keyword) == 0)
			{
				found = 1;
				break;
			}
		}
		if (found) break;
		ptr = strtok(NULL, ";");
	}
	free(name_save);
	if (value_ptr != NULL) *value_ptr = value;
	else free(value);
	if (nvalue_ptr != NULL) *nvalue_ptr = nvalue;
	return found;
}

FILE *object_fileopen(char *filename)
{
	char *ptr, *name;
	FILE *file;
	int first, last;
	name = strdup(filename);
	ptr = strchr(name, '@');	/* filename */
	if (ptr != NULL)
	{
		*ptr = 0x0;	/* add a string termination to filename */
		file = fopen(name, "r");
		sscanf(ptr + 1, "%d-%d", &first, &last);
		fseek(file, first, SEEK_SET);
	}
	else
	{
		file = fopen(name, "r");
	}
	free(name);
	return file;
}

void object_compilevalue(OBJECT*object)
{
	enum MODE
	{ FIRST, LAST };
	static int lbuff = 0;
	static char *buffer = NULL;
	char *keyword, *value, *kvalue, *ptr;
        const char *op;
	struct
	{
            char *keyword;
            const char *op;
            char *value;
	} list[MAXKEYWORDS];
	int nlist;
	char *string, *opptr;
	int i, j, lstring;
	int mode = LAST;
	string = object->value;
	lstring = strlen(string) + 1;
	if (lstring > lbuff)	/* Create temp character  array */
	{
		lbuff = lstring;
		buffer = (char*) realloc(buffer, lstring*sizeof(char));
	}
	strcpy(buffer, string);
	nlist = 0;
	ptr = strtok(buffer, ";");
	while (ptr != NULL)	/*  loop through   "keyword=value ; */
	{
		opptr = strchr(ptr, '=');
		keyword = ptr;
		value = opptr + 1;
		op = "=";
		*opptr = 0x0;
		if (*(opptr - 1) == '+')
		{
			op = "+=";
			*(opptr - 1) = 0x0;
		}
		trim(keyword);
		trim(value);
		list[nlist].keyword = keyword;
		list[nlist].op = op;
		list[nlist].value = value;
		nlist++;
		ptr = strtok(NULL, ";");
	}
	object->value[0] = 0x0;
	for (i = 0; i < nlist; i++)
	{
		keyword = list[i].keyword;
		if (keyword != NULL)
		{
			strcat(object->value, keyword);
			strcat(object->value, "=");
			kvalue = object->value + strlen(object->value);
			strcpy(kvalue, list[i].value);
			for (j = i + 1; j < nlist; j++)
			{
				if (list[j].keyword != NULL)
				{
					if (strcmp(list[j].keyword, keyword) == 0)
					{
						list[j].keyword = NULL;
						if (mode == LAST)
						{
							if (strcmp(list[j].op, "=") == 0) strcpy(kvalue, list[j].value);
						}
						if (strcmp(list[j].op, "+=") == 0)
						{
							strcat(kvalue, " ");
							strcat(kvalue, list[j].value);
						}
					}
				}
			}
			strcat(object->value, ";");
		}
	}
	if (object->valueptr) *object->valueptr = object->value;
}

FIELD object_parse(OBJECT*object, char *name, int type, char *dvalue)
{
	static int lbuff = 0, msize = 0;
	static char *buffer = NULL, *line = NULL, *tail;
        static const char *sep;
	static union
	{
		double *d;
		int *i;
		short *sh;
		char **s;
		FILE **file;
		FileList *filelist;
		void *v;
	} v;
	static int nvalue = 0;
	static int nkeyword = 0;
	static char *value = NULL;
	static char *keyword = NULL;
	FILE *file;
	int nname;
	char *name_list[16], *name_save;
	char *string;
	FIELD f;
	int i, lstring, nv, size, element_size, found, first, last;
	char *ptr, *vptr, *eptr;
	name_save = strdup(name);
	ptr = strtok(name_save, ";");
	nname = 0;
	while (ptr != NULL && nname < 16)	/* create list of names to parse */
	{
		trim(ptr);
		name_list[nname] = ptr;
		ptr = strtok(NULL, ";");
		nname++;
	}
	string = object->value;
	lstring = strlen(string) + 1;
	if (lstring > lbuff)	/* Create some temp character  array */
	{
		lbuff = lstring;
		if (lbuff < 512) lbuff = 512;
		buffer = (char*) realloc(buffer, lbuff*sizeof(char));
		line = (char*) realloc(line, lbuff*sizeof(char));
	}
	strcpy(buffer, string);
	ptr = strtok(buffer, ";");
	while (ptr != NULL)	/*  loop through   "keyword=value ; "  fields  and check if keyword matches one of the name fields */
	{
		strcpy(line, ptr);
		trim(line);
		ptr = strchr(line, '=');
		*ptr = 0x0;
		while (nkeyword < strlen(line) + 1)
		{
			nkeyword += 256;
			keyword = (char*) realloc(keyword, nkeyword);
		}
		strncpy(keyword, line, nkeyword);
		trim(keyword);
		ptr++;
		while (nvalue < strlen(ptr) + 1)
		{
			nvalue += 256;
			value = (char*) realloc(value, nvalue);
		}
		strncpy(value, ptr, nvalue);
		trim(value);
		found = 0;
		for (i = 0; i < nname; i++)
		{
			if (strcmp(name_list[i], keyword) == 0)
			{
				found = 1;
				break;
			}
		}
		if (found) break;
		ptr = strtok(NULL, ";");
	}
	if (found == 0)
	{
		if (dvalue == NULL) error_action("Unable to locate ", name, " in object ", object->name, ERROR_IN("object_parse", ABORT));
		if (dvalue == (char *)IGNORE_IF_NOT_FOUND)
		{
			f.n = 0;
			f.v = NULL;
			return f;
		}
		while (nvalue < strlen(dvalue) + 1)
		{
			nvalue += 256;
			value = (char*) realloc(value, nvalue);
		}
		strncpy(value, dvalue, nvalue);
	}

	trim(value);
	if (type == LITERAL)
	{
		vptr = value;
		v.s[0] = strdup(vptr);
		f.n = 1;
		f.v = v.v;
		f.size = f.element_size = sizeof(char *);
		return f;
	}
	tail = value;
	if (*tail == (char)'"') sep = "\"";
	else
		sep = " ";	/*Check if value start with "  */
	vptr = strtok_r(value, sep, &tail);
	nv = size = 0;
	if (vptr != NULL) if (!strncmp(vptr, "$B", 2))
		{
			unsigned int ptr;
			sscanf(vptr + 2, "%d-%x", &size, &ptr);
			vptr = NULL;
			nv = 1;
		}
	while (vptr != NULL)
	{
		if (size + 8 > msize)
		{
			msize += 128;
			v.v = realloc(v.v, msize);
		}
		switch (type)
		{
		case DOUBLE:
			if (!strncmp(vptr, "0x", 2))
			{
				v.i[nv + 1] = strtol(vptr + 10, &eptr, 16);
				vptr[10] = (char)NULL;
				v.i[nv] = strtol(vptr + 2, &eptr, 16);
			}
			else
			{
				v.d[nv] = strtod(vptr, &eptr);
			}
			element_size = sizeof(double);
			break;
		case INT:
			v.i[nv] = strtol(vptr, &eptr, 0);
			element_size = sizeof(int);
			break;
		case SHORT:
			v.sh[nv] = strtol(vptr, &eptr, 0);
			element_size = sizeof(short);
			break;
		case STRING:
			v.s[nv] = strdup(vptr);
			element_size = sizeof(char *);
			break;
		case FILETYPE:
			ptr = strchr(vptr, '@');	/* filename */
			if (ptr != NULL)
			{
				*ptr = 0x0;	/* add a string termination to filename */
				file = fopen(vptr, "r");
				sscanf(ptr + 1, "%d-%d", &first, &last);
				fseek(file, first, SEEK_SET);
			}
			else
			{
				file = fopen(vptr, "r");
			}
			element_size = sizeof(FILE *);
			v.file[nv] = file;
			break;
		case FILELIST:
			ptr = strchr(vptr, '@');	/* filename */
			if (ptr != NULL)
			{
				*ptr = 0x0;	/* add a string termination to filename */
				sscanf(ptr + 1, "%d-%d", &first, &last);
			}
			else
			{
				file = fopen(vptr, "r");
				first = 0;
				last = -1;
			}
			v.filelist[nv].name = strdup(vptr);
			v.filelist[nv].start = first;
			v.filelist[nv].end = last;
			element_size = sizeof(FileList);
			break;
		default:
			break;
		}
		nv++;
		size += element_size;
		trim(tail);
		if (*tail == (char)'"') sep = "\"";
		else
			sep = " ";
		vptr = strtok_r(NULL, sep, &tail);
	}
	f.n = nv;
	f.v = v.v;
	f.size = size;
	f.element_size = element_size;
	return f;
}

void object_compileSectionedFile(char *filename, int section)
{
	OBJECT obj, obj_delim;
	char *line;
	OBJECTFILE file;
	int rc, i, l, sec;
	obj.name = obj_delim.name = NULL;
	file = object_fopen(filename, "r");
	sec = -1;
	if (file.file != NULL)
	{
		while ((line = object_read(file)) != NULL)
		{
			rc = object_lineparse(line, &obj);
			if (rc < 2)
			{
				if (sec == -1)
				{
					obj_delim.name = strdup(obj.name);
					obj_delim._class = strdup(obj._class);
				}
				if (!strcmp(obj_delim._class, obj._class) && !strcmp(obj_delim.name, obj.name)) sec++;
				if (sec > section) break;
				if (sec == section)
				{
					for (i = 0; i < nobject; i++)
					{
						if (!strcmp(object[i]._class, obj._class) && !strcmp(object[i].name, obj.name)) break;
					}
					if (i == nobject)
					{
						nobject++;
						if (mobject < nobject) mobject += 100;
						object = (OBJECT*) realloc(object, mobject*sizeof(OBJECT));
						object[i].name = strdup(obj.name);
						object[i]._class = strdup(obj._class);
						object[i].value = strdup(obj.value);
						object[i].valueptr = NULL;;
					}
					else
					{
						l = strlen(object[i].value) + strlen(obj.value) + 1;
						object[i].value = (char*)realloc(object[i].value, l);
						strcat(object[i].value, obj.value);
					}
				}
			}
			if (obj.name != NULL)
			{
				free(obj.name);
				free(obj._class);
				free(obj.value);
			}
		}
		if (obj_delim.name != NULL)
		{
			free(obj_delim.name);
			free(obj_delim._class);
		}
		object_fclose(file);
	}
	for (i = 0; i < nobject; i++) object_compilevalue(object + i);
}

void object_compilefilesubset(const char *filename, int first, int last)
{
	OBJECT obj;
	char *line;
	OBJECTFILE file;
	int rc, i, l, n;
	file = object_fopen(filename, "r");
	n=0; 
	if (file.file != NULL)
	{
		while ((line = object_read(file)) != NULL )
		{
			rc = object_lineparse(line, &obj);
			if (rc < 2 && n>=first)
			{
				for (i = 0; i < nobject; i++)
				{
					if (!strcmp(object[i]._class, obj._class) && !strcmp(object[i].name, obj.name)) break;
				}
				if (i == nobject)
				{
					nobject++;
					if (mobject < nobject) mobject += 100;
					object = (OBJECT*) realloc(object, mobject*sizeof(OBJECT));
					object[i].name = strdup(obj.name);
					object[i]._class = strdup(obj._class);
					object[i].value = strdup(obj.value);
					object[i].valueptr = NULL;;
				}
				else
				{
					l = strlen(object[i].value) + strlen(obj.value) + 1;
					object[i].value = (char*) realloc(object[i].value, l);
					strcat(object[i].value, obj.value);
				}
			}
			free(obj.name);
			free(obj._class);
			free(obj.value);
			if ( ++n > last ) break; 
		}
		object_fclose(file);
	}
	for (i = 0; i < nobject; i++) object_compilevalue(object + i);
}
void object_compilefile(const char *filename) { object_compilefilesubset(filename,0,0x7fffffff); }

void object_replacekeyword(OBJECT *object,char *keyword,char* keywordvalue)
{
	char value[1024];
	int l; 
	sprintf(value,"%s=%s;",keyword,keywordvalue);
	l = strlen(object->value) + strlen(value) + 1;
	object->value = (char*) realloc(object->value, l);
	strcat(object->value, value);
	object_compilevalue(object);
}
void object_compile()
{
	int k;
	if (mobject < nobject)
	{
		mobject += 100;
		object = (OBJECT*) realloc(object, mobject*sizeof(OBJECT));
	}
	for (k = 0; k < nfiles; k++)
	{
		object_compilefile(filenames[k]);
	}
}

void object_reset(OBJECT*target)
{
	OBJECT obj;
	char *line;
	OBJECTFILE file;
	int rc, l, k;
	if (target->name == NULL || target->_class == NULL) return;
	*(target->value) = '\0';
	for (k = 0; k < nfiles; k++)
	{
		file = object_fopen(filenames[k], "r");
		if (file.file != NULL)
		{
			while ((line = object_read(file)) != NULL)
			{
				rc = object_lineparse(line, &obj);
				if (rc < 2)
				{
					if (!strcmp(target->_class, obj._class) && !strcmp(target->name, obj.name))
					{
						l = strlen(target->value) + strlen(obj.value) + 1;
						target->value = (char*) realloc(target->value, l);
						strcat(target->value, obj.value);
					}
				}
				free(obj.name);
				free(obj._class);
				free(obj.value);
			}
			object_fclose(file);
		}
	}
}

OBJECT *object_find(char *name, char *classIn)
{
	int i;
	for (i = 0; i < nobject; i++)
	{
		if (!strcmp(object[i]._class, classIn) && !strcmp(object[i].name, name)) break;
	}
	if (i == nobject) error_action("Unable to locate object ", name, "in data files", ERROR_IN("object_find", ABORT));
	return object + i;
}

OBJECT *object_find2(char *name, char *classIn, enum OBJECTACTION action)
{
	int i;
	for (i = 0; i < nobject; i++)
	{
		if (!strcmp(object[i]._class, classIn) && !strcmp(object[i].name, name)) break;
	}
	if (i == nobject)
	{
		switch (action)
		{
		case ABORT_IF_NOT_FOUND:
			error_action("Unable to locate object ", name, "in data files", ERROR_IN("object_find2", ABORT));
			break;
		case WARN_IF_NOT_FOUND:
			error_action("Unable to locate object ", name, "in data files", ERROR_IN("object_find2", CONTINUE));
			return NULL;
			break;
		case IGNORE_IF_NOT_FOUND:
			return NULL;
			break;
		}
		return NULL;
	}
	return object + i;
}

OBJECT *object_initialize(char *name, char *classIn, int size)
{
	OBJECT *object_short, *object_long;
	object_long = (OBJECT*) malloc(size);
	object_short = object_find(name, classIn);
	object_long->name = object_short->name;
	object_long->_class = object_short->_class;
	object_long->value = object_short->value;
	object_long->valueptr = &object_short->value;
	object_short->valueptr = &object_long->value;
	if (miobject <= niobject)
	{
		miobject += 100;
		object_list = (OBJECT**) realloc(object_list, miobject*sizeof(OBJECT *));
	}
	object_list[niobject] = object_long;
	niobject++;
	return object_long;
}

OBJECT *object_find1(char *name, char *type)
{
	OBJECTFILE file;
	int rc, nfound, l, k;
	OBJECT *object = NULL;
	char *line, *value;
	if (object == NULL) object = (OBJECT*) malloc(sizeof(OBJECT));
	value = (char*) malloc(1);
	*value = 0x0;
	nfound = 0;
	for (k = 0; k < nfiles; k++)
	{
		file = object_fopen(filenames[k], "r");
		while ((line = object_read(file)) != NULL)
		{
			rc = object_lineparse(line, object);
			if (rc < 2 && (strcmp(object->_class, type) == 0) && (strcmp(object->name, name) == 0))
			{
				nfound++;
				l = strlen(value) + 1;
				if (object->value != NULL)
				{
					l += strlen(object->value);
					value = (char*) realloc(value, l);
					strcat(value, object->value);
				}
			}
			free(object->name);
			free(object->_class);
			free(object->value);
		}
		object_fclose(file);
	}
	if (nfound > 0)
	{
		object->name = strdup(name);
		object->_class = strdup(type);
		object->value = strdup(value);
		return object;
	}
	error_action("Unable to locate object ", name, "in data files", ERROR_IN("object_find1", ABORT));
	return NULL;
}

char *object_read(OBJECTFILE ofile)
{
	FILE *file;
	static char *line = NULL;
	static int nline = 0;
	int n, c, first, last, len, nitems, offset;
	file = ofile.file;
	n = 0;
	c = getc(file);
	while (!feof(file))
	{
		while (nline < n + 5)
		{
			nline += 256;
			line = (char*) realloc(line, nline);
		}
		switch (c)
		{
		case '"':
			line[n++] = (char)c;
			c = getc(file);
			while (c != '"')
			{
				if (nline < n + 5 )
				{
					nline += 256;
					line = (char*) realloc(line, nline);
				}
				line[n++] = (char)c;
				c = getc(file);
			}
			break;
		case '/':
			c = getc(file);
			if (c != '*')
			{
				ungetc(c, file);
				c = '/';
				break;
			}
			else
			{
				char clast; 
				clast = c = getc(file);
				while (c != EOF)
				{
					if (c == '/'  && clast == '*') {c = ' '; break;}
					clast = c; 
					c = getc(file);
				}
				if (c==EOF) error_action("Comment termination ('*/') not found in object file : ", ofile.name, ERROR_IN("object_read", ABORT));
				
			}
			break;
		case '\n':
			c = ' ';
			break;
		case '\t':
			c = ' ';
			break;
		case ' ':
			break;
		case '{':
			break;
		case '}':
			break;
		case '\\':
			c = (char)getc(file);
			switch (c)
			{
			case 'n':
				c = '\n';
				break;
			case 't':
				c = '\t';
			}
			break;
		case '$':
			c = (char)getc(file);
			switch (c)
			{
			case 'S':
				nitems = fscanf(file, "%d", &offset);
				c = (char)getc(file);
				while (!isgraph(c))
				{
					if (feof(file)) return NULL;
					c = (char)getc(file);
				}
				first = ftell(file) - 1;
				if (nitems > 0) fseek(file, offset, SEEK_CUR);
				while (c != ';' && c != '}')
				{
					if (feof(file)) return NULL;
					c = (char)getc(file);
				}
				last = ftell(file) - 1;
				len = strlen(ofile.name);
				while (nline < n + len + 1 + 2 + 64)
				{
					nline += 256;
					line = (char*) realloc(line, nline);
				}
				len = sprintf(line + n, "%s@%d-%d", ofile.name, first, last);
				n += len;
				break;
			case 'B':
				line[n++] = '$';
				break;
			}
			break;
		case ';':
			break;
		default:
			break;
		}
		line[n++] = (char)c;
		if (c == '}')
		{
			line[n++] = 0x0;
			trim(line);
			return line;
		}
		c = (char)getc(file);
	}
	return NULL;
}

void object_free(OBJECT*object)
{
	free(object->name);
	free(object->_class);
	free(object->value);
	free(object);
}

int object_lineparse(char *line, OBJECT*object)
{
	char *tok;
	int rc, l;
	trim(line);
	tok = strchr(line,'{');
	*tok = '\0';
	tok++; 
	l = strlen(tok);
	tok[l-1] = '\0'; 
	object->value = strdup(tok);
	tok = strtok(line, " ");
	object->name = strdup(tok);
	tok = strtok(NULL, " ");
	object->_class = strdup(tok);

	trim(object->name);
	trim(object->_class);
	trim(object->value);
	if (object->value != NULL)
	{
		l = strlen(object->value);
		if (object->value[l - 1] != ';')
		{
			object->value = (char*) realloc(object->value, l + 2);
			strcat(object->value, ";");
		}
	}
	rc = 0;
	if (object->name == NULL) rc += 4;
	if (object->_class == NULL) rc += 2;
	if (object->value == NULL) rc += 1;
	return rc;
}

OBJECTFILE object_fopen(const char *filename, const char *mode)
{
	OBJECTFILE file;
	file.file = fopen(filename, mode);
	if (file.file == NULL)
	{
		char *msg;
		int msg_length; 
		msg_length = strlen(filename)+256; 
		msg = (char*) malloc(msg_length); 
		sprintf(msg, "Error opening file=%s with mode %s from object_fopen", filename, mode);
		free(msg); 
		perror(msg);
	}
	file.name = strdup(filename);
	return file;
}

void object_fclose(OBJECTFILE file)
{
	fclose(file.file);
	free(file.name);
}

int object_register(char *name, char *type, int itype, void *address)
{
	return 0;
}

int modeindex(char *mode)
{
	if (!strcasecmp(mode, "FORMATTED")) return ASCII;
	if (!strcasecmp(mode, "ASCII")) return ASCII;
	if (!strcasecmp(mode, "BINARY")) return BINARY;
	if (!strcasecmp(mode, "UNFORMATTED")) return BINARY;
	return ASCII;
}
void object_pack(PACKBUF *buf)
{
	int i,l,n;
	n=0;
	for (i=0;i<nobject;i++)
	{
		n+= l =  strlen(object[i].name)+1 ; buf->buffer=(char*) realloc(buf->buffer,n);memcpy(buf->buffer+n-l,object[i].name,l); 
		n+= l =  strlen(object[i]._class)+1; buf->buffer=(char*) realloc(buf->buffer,n);memcpy(buf->buffer+n-l,object[i]._class,l); 
		n+= l =  strlen(object[i].value)+1; buf->buffer=(char*) realloc(buf->buffer,n);memcpy(buf->buffer+n-l,object[i].value,l);
	}
	buf->n =n; 
	buf->nobject =nobject; 
	buf->mobject =mobject; 
}
void object_unpack(PACKBUF *buf)
{
	int i,l;
	char *ptr; 
	nobject=buf->nobject ; 
	mobject=buf->mobject ; 
	object = (OBJECT*) realloc(object, mobject*sizeof(OBJECT));
	ptr = buf->buffer; 
	for (i=0;i<nobject;i++)
	{
		l = strlen(ptr)+1 ; object[i].name = strdup(ptr);ptr += l;
		l = strlen(ptr)+1 ; object[i]._class = strdup(ptr);ptr += l;
		l = strlen(ptr)+1 ; object[i].value = strdup(ptr);ptr += l;
		object[i].valueptr = NULL;;
	}
}
