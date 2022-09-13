#include	"memmgr.h"


//--------------------------------------------------------------------------------------------
// DEFINES
#define	MAX_MEMPOINTERS					800000
#define	MEMMGR_MAX_FILENAME_LENGTH		256
#define MEMMGR_MAX_STACK_FILE_LINE		32

#define	GET_SIZE_PTR(p)		*((int  *)((char  *)p  -  sizeof(int) - sizeof(int)))
#define GET_PREHEADER(p)	((PointerPreHeapInfo    *)((char  *)p-sizeof(PointerPreHeapInfo)))
#define GET_POSTHEADER(p)	((PointerPostHeapInfo  *)((char  *)p+(GET_SIZE_PTR(p))))
#define	KEY_NOT_FOUND		-1

#define MEMMGR_LOG_INFO(file,line,s, ...)		MEMMGR_log(LOG_TYPE_INFO,file,line,s, __VA_ARGS__)
#define MEMMGR_LOG_INFOF(file,line,s)			MEMMGR_LOG_INFO(file,line,s,NULL)

#define MEMMGR_LOG_WARNING(file,line,s, ...)	MEMMGR_log(LOG_TYPE_WARNING,file,line,s, __VA_ARGS__)
#define MEMMGR_LOG_WARNINGF(file,line,s)   		MEMMGR_LOG_WARNING(file,line,s,NULL)

#define MEMMGR_LOG_ERROR(file,line,s, ...)		MEMMGR_log(LOG_TYPE_ERROR,file,line,s, __VA_ARGS__)
#define MEMMGR_LOG_ERRORF(file,line,s)   		MEMMGR_LOG_ERROR(file,line,s,NULL)


//--------------------------------------------------------------------------------------------
//  turn  off  macros...
#undef  malloc
#undef  free


typedef enum{
	TERM_CMD_RESET = 0,
	TERM_CMD_BRIGHT = 1,
	TERM_CMD_DIM = 2,
	TERM_CMD_UNDERLINE = 3,
	TERM_CMD_BLINK = 4,
	TERM_CMD_REVERSE = 7,
	TERM_CMD_HIDDEN = 8

}TERM_CMD;

typedef enum{
	TERM_COLOR_BLACK = 0,
	TERM_COLOR_RED = 1,
	TERM_COLOR_GREEN = 2,
	TERM_COLOR_YELLOW = 3,
	TERM_COLOR_BLUE = 4,
	TERM_COLOR_MAGENTA = 5,
	TERM_COLOR_CYAN = 6,
	TERM_COLOR_WHITE = 7
}TermColor;

typedef enum{
	UNKNOWN_ALLOCATE=0,
	MALLOC_ALLOCATOR,  //  by  default
	NEW_ALLOCATOR,
	NEW_WITH_BRACETS_ALLOCATOR,
	MAX_ALLOCATE_TYPES
}ALLOCATOR_TYPE;

typedef enum{
	LOG_TYPE_INFO=0
	,LOG_TYPE_WARNING
	,LOG_TYPE_ERROR
}LogType;

//--------------------------------------------------------------------------------------------
// STRUCTS

typedef  struct{
	uintptr_t 	*ptr;
	char  		filename[MEMMGR_MAX_FILENAME_LENGTH];
	int  		line;
}InfoAllocatedPointer;

typedef  struct{
	int		type_allocator;
	int		offset_mempointer_table;
	char	filename[MEMMGR_MAX_FILENAME_LENGTH];  //  base    		-16-256
	int		line;          					//  base          	-16
	int		size;                      		//  base          	-8
	int		pre_crc;                		//  base          	-4
}PointerPreHeapInfo;

typedef  struct{
	int		post_crc;
}PointerPostHeapInfo;

//--------------------------------------------------------------------------------------------
// GLOBAL VARS

static int	g_n_allocated_bytes  =  0;
static int	g_n_allocated_pointers  =  0;
static bool	g_memmgr_was_init  =  false;

static void	*g_allocated_pointer[MAX_MEMPOINTERS];
static int 	g_free_pointer_idx[MAX_MEMPOINTERS]={0};
static int 	g_n_free_pointers=0;



static 	pthread_mutex_t mutex_main;

//--------------------------------------------------------------------------------------------
static void  MEMMGR_print_status(void);
//--------------------------------------------------------------------------------------------
// PATH UTILS
void  MEMMGR_get_filename(char  *filename, const char *absolute_filename)
{
	const  char  *to_down_ptr;
	int  i=0, lenght;

	if(absolute_filename==NULL)
	{
		return;
	}

	if((lenght = (strlen(absolute_filename)-1)) > 0)
	{

		to_down_ptr = &absolute_filename[lenght-1];
		//  get  name  ...
		if((to_down_ptr-1)  >=  absolute_filename)
		{
			do
			{
				to_down_ptr--;
				i++;

			}while(*(to_down_ptr-1)  !=  '\\'  &&  *(to_down_ptr-1)  !=  '/'  &&  to_down_ptr  >  absolute_filename && i < MEMMGR_MAX_FILENAME_LENGTH);
		}

		sprintf(filename,"%s",to_down_ptr);
	}
}

void MEMMGR_set_color_terminal(FILE *std_type, int attr, int fg, int bg)
{

	char command[50]={0};

	/* Command is the control command to the terminal */
	sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
	fprintf(std_type, "%s", command);
}

//--------------------------------------------------------------------------------------------
// LOG UTILS
#ifndef  __GNUC__
#pragma  managed(push,  off)
#endif
void  MEMMGR_log(LogType _log_type, const char *_file, int _line, const  char  *string_text, ...) {

	char  filename[MEMMGR_MAX_FILENAME_LENGTH]="";
	if(_file != NULL){
		MEMMGR_get_filename(filename,  _file);
	}

	FILE *std_type = stdout;
	const char *log_type_str="MEMINF";
	char  text[4096] = { 0 };
	va_list  ap;

	va_start(ap,  string_text);
	vsprintf(text,  string_text,  ap);
	va_end(ap);

	switch(_log_type){
	case LOG_TYPE_INFO:
		break;
	case LOG_TYPE_WARNING:
		log_type_str="MEMWRN";
		std_type = stderr;
		break;
	case LOG_TYPE_ERROR:
		log_type_str="MEMERR";
		std_type = stderr;
		break;

	}

	//  Results  Are  Stored  In  Text
#ifdef _WIN32
  SetConsoleTextAttribute(GetStdHandle(_log_type==LOG_TYPE_ERROR?STD_ERROR_HANDLE:STD_OUTPUT_HANDLE), _log_type==LOG_TYPE_ERROR?FOREGROUND_RED:_log_type==LOG_TYPE_WARNING?(FOREGROUND_RED   | FOREGROUND_GREEN):(FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE));
#else // ansi color
  MEMMGR_set_color_terminal(std_type, TERM_CMD_BRIGHT, _log_type==LOG_TYPE_ERROR?TERM_COLOR_RED:_log_type==LOG_TYPE_WARNING?TERM_COLOR_YELLOW:TERM_COLOR_WHITE, TERM_COLOR_BLACK);
#endif
	fprintf(std_type, "[ %27s:%04i - %3s]=%s",filename,_line,log_type_str, text);
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(_log_type==LOG_TYPE_ERROR?STD_ERROR_HANDLE:STD_OUTPUT_HANDLE), FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else // ansi color
	MEMMGR_set_color_terminal(std_type, TERM_CMD_BRIGHT, TERM_COLOR_WHITE, TERM_COLOR_BLACK);
#endif


	fprintf(std_type, "\n");
	fflush(std_type);
}

#ifndef  __GNUC__
#pragma  managed(pop)
#endif

//--------------------------------------------------------------------------------------------
// MEMMGR Functions
void  MEMMGR_init(void)
{

	if(!g_memmgr_was_init)
	{
		g_n_allocated_bytes  =  0;
		memset(g_allocated_pointer,0,sizeof(g_allocated_pointer));
		g_n_allocated_pointers  =  0;
		g_n_free_pointers = MAX_MEMPOINTERS-1;

		for(int i = 0; i < g_n_free_pointers; i++){
			g_free_pointer_idx[i]=MAX_MEMPOINTERS-1-i;
		}

		MEMMGR_LOG_INFOF(__FILE__,__LINE__,"******************************");
		MEMMGR_LOG_INFOF(__FILE__,__LINE__,"Memory management initialized!");
		MEMMGR_LOG_INFOF(__FILE__,__LINE__,"******************************");

		atexit(MEMMGR_print_status);

		g_memmgr_was_init  =  true;
	}
}

//--------------------------------------------------------------------------------------------
int  MEMMGR_get_free_cell_memptr_table(void)
{
	if(g_n_free_pointers > 0){
		return g_free_pointer_idx[g_n_free_pointers];
	}
	return KEY_NOT_FOUND; // no memory free...
}

//--------------------------------------------------------------------------------------------
void 	*MEMMGR_malloc(size_t  _size,  const  char  *_absolute_filename,  int  _line)
{
	char  filename[MEMMGR_MAX_FILENAME_LENGTH];
	MEMMGR_get_filename(filename,  _absolute_filename);
	// do not register
	if(_size == 0){
		MEMMGR_LOG_WARNINGF(filename,_line,"Try to allocate pointer with 0 bytes");
		return NULL;
	}

	pthread_mutex_lock(&mutex_main);

	PointerPreHeapInfo  *heap_allocat  =  NULL;
	void  * pointer=NULL;
	int  random_number,index;


	if(!g_memmgr_was_init)  MEMMGR_init();  //  auto_inicialize  return  malloc(size);

	heap_allocat  =  (PointerPreHeapInfo  *)malloc(sizeof(PointerPreHeapInfo)  +  sizeof(PointerPostHeapInfo)  +  _size);
	if(heap_allocat
			&&
			((index  =  MEMMGR_get_free_cell_memptr_table())  !=  -1))
	{
		strcpy(heap_allocat->filename,filename);
		heap_allocat->size  =  _size;
		heap_allocat->offset_mempointer_table  =  index;


		heap_allocat->line  =  _line;

		random_number  =  ((unsigned)(rand()%0xFFFF)  <<  16)  |  (rand()%0xFFFF);

		heap_allocat->pre_crc  =  random_number;
		heap_allocat->size        =  _size;
		heap_allocat->type_allocator  =  MALLOC_ALLOCATOR;

		g_allocated_pointer[index] 	    = heap_allocat;

		*((int  *)((char  *)heap_allocat+_size+sizeof(PointerPreHeapInfo)))  =  random_number;

		g_n_allocated_bytes  +=  _size;

		pointer  =  ((char  *)heap_allocat+sizeof(PointerPreHeapInfo));

		g_n_allocated_pointers++;
		g_n_free_pointers--;

		//MEMMGR_LOG_INFO(NULL,0,"Current allocated pointers: %i of %i (%i%%)",g_n_allocated_pointers,MAX_MEMPOINTERS,(g_n_allocated_pointers*100/MAX_MEMPOINTERS));

		//  memset  pointer
		memset(pointer,0,_size);
	}else{
		MEMMGR_LOG_ERRORF(__FILE__,__LINE__,"Table full of pointers or not enough memory");
	}

	//malloc_mutex.unlock();
	pthread_mutex_unlock(&mutex_main);

	return pointer;
}
//--------------------------------------------------------------------------------------------
void *MEMMGR_calloc(size_t  n_items,size_t  size_item,  const  char  *absolute_filename,  int  line){
	return MEMMGR_malloc(n_items*size_item,absolute_filename,line);
}
//--------------------------------------------------------------------------------------------
void  MEMMGR_free_c_pointer(void  *pointer){
	free(pointer);
}

//--------------------------------------------------------------------------------------------
void  MEMMGR_free(void  *pointer,  const  char  *filename,  int  line)
{

	//std::lock_guard<std::mutex> lg(mutex_main);
	pthread_mutex_lock(&mutex_main);

	PointerPreHeapInfo    *preheap_allocat    =  NULL;
	PointerPostHeapInfo  *postheap_allocat  =  NULL;
	void  *base_pointer;

	if(pointer == NULL){
		MEMMGR_LOG_ERRORF(filename,line,"Try to deallocate NULL pointer");
		goto MEMMGR_free_continue;
	}


	//  Getheaders...
	base_pointer  =  preheap_allocat    =  GET_PREHEADER(pointer);
	postheap_allocat  =  GET_POSTHEADER(pointer);

	//  Check  headers...
	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)  //  crc  ok  :)
	{
		MEMMGR_LOG_ERRORF(filename,line,"free():Try to deallocate a pointer with CRC error. Either is a corrupted pointer or not managed pointer!");
		goto MEMMGR_free_continue;
	}

	if(preheap_allocat->offset_mempointer_table  <  0  ||  preheap_allocat->offset_mempointer_table  >=  MAX_MEMPOINTERS)
	{
		MEMMGR_LOG_ERRORF(filename,line,"Bad  index  mem  table");
		goto MEMMGR_free_continue;
	}

	//  deallocate  pointer  will  be  ok  :)

	//  Mark  freed...
	g_allocated_pointer[preheap_allocat->offset_mempointer_table]  =  NULL;

	if(g_n_free_pointers>=(MAX_MEMPOINTERS-1)){
		MEMMGR_LOG_ERRORF(__FILE__,__LINE__,"Reached max table of free pointers!");
		goto MEMMGR_free_continue;
	}

	g_n_free_pointers++;
	g_free_pointer_idx[g_n_free_pointers] = preheap_allocat->offset_mempointer_table;

	//-----------------------------------------------------------------
	// DS delete element ...
	//if(MEMMGR_dicotomic_delete((intptr_t)base_pointer)){
	g_n_allocated_bytes-=preheap_allocat->size;
	g_n_allocated_pointers--;
	free(base_pointer);
	//}

MEMMGR_free_continue:

	pthread_mutex_unlock(&mutex_main);
}

void *MEMMGR_realloc(void *ptr, size_t size,  const  char  *absolute_filename,  int  line) {


	if (ptr==NULL) {
		// NULL ptr. realloc should act like malloc.
		return MEMMGR_malloc(size, absolute_filename, line);
	}


	PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(ptr);


	if ((size_t)pre_head->size >= size) {
		// We have enough space. Could free some once we implement split.
		return ptr;
	}

	// Need to really realloc. Malloc new space and free old space.
	// Then copy old data to new space.
	void * new_ptr=NULL;
	new_ptr = MEMMGR_malloc(size, absolute_filename, line);

	if (!new_ptr) {
		return NULL; // TODO: set errno on failure.
	}
	memcpy(new_ptr, ptr, pre_head->size);
	MEMMGR_free(ptr, absolute_filename, line);

	return new_ptr;
}
//--------------------------------------------------------------------------------------------
void  MEMMGR_print_error_on_wrong_deallocate_method(const char *_filename, int _line,int  _allocator)
{


	switch(_allocator)
	{
	case  MALLOC_ALLOCATOR:
		MEMMGR_LOG_ERRORF(_filename,  _line,"Allocated_pointer must freed  with  function  free()");
		break;
	case  NEW_ALLOCATOR:
		MEMMGR_LOG_ERRORF(_filename,  _line,"Allocated_pointer must freed  with  operator  delete");
		break;
	case  NEW_WITH_BRACETS_ALLOCATOR:
		MEMMGR_LOG_ERRORF(_filename,  _line,"Allocated_pointer must freed  with  operator  delete[]");
		break;
	}
}

//----------------------------------------------------------------------------------------
void  MEMMGR_free_from_malloc(void  *p,  const  char  *_absolute_filename,  int  _line  )
{
	char  filename[MEMMGR_MAX_FILENAME_LENGTH];
	MEMMGR_get_filename(filename,_absolute_filename);
	PointerPreHeapInfo  *preheap_allocat  =  NULL;
	PointerPostHeapInfo  *postheap_allocat  =  NULL;


	if(p == NULL)
	{
		MEMMGR_LOG_WARNINGF(filename,  _line,"NULL  pointer  to  deallocate");
		return;
	}

	preheap_allocat  =  GET_PREHEADER(p);
	postheap_allocat  =  GET_POSTHEADER(p);

	//  Check  headers...
	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)  //  crc  ok  :)
	{
		MEMMGR_LOG_ERRORF(filename,_line,"Bad  crc  pointer");
		return;
	}

	if(preheap_allocat->type_allocator  !=  MALLOC_ALLOCATOR)
	{
		MEMMGR_print_error_on_wrong_deallocate_method(filename, _line,preheap_allocat->type_allocator);
		return;

	}

	MEMMGR_free(p,  filename,  _line);
}
//--------------------------------------------------------------------------------------------
void  MEMMGR_print_status(void)
{
	PointerPreHeapInfo    *preheap_allocat;
	int  i;
	size_t allocated_bytes=0;
	size_t pointers_to_deallocate=0;

	for(i  =  0;  i  <  MAX_MEMPOINTERS;  i++)
	{
		if((preheap_allocat  =  (PointerPreHeapInfo    *)g_allocated_pointer[i]))
		{
			if(preheap_allocat->line>0 && (strcmp("??",preheap_allocat->filename)!=0)) // leak from others libs
			{
				allocated_bytes+=preheap_allocat->size;
				pointers_to_deallocate++;
				const char *pointer=((char *)preheap_allocat)+sizeof(PointerPreHeapInfo);
				MEMMGR_LOG_ERROR(preheap_allocat->filename,  preheap_allocat->line,"Allocated  pointer  NOT  DEALLOCATED (%p)",pointer);
			}
		}
	}

	//-----
	if(pointers_to_deallocate>0  ||  allocated_bytes>0)
	{
		MEMMGR_LOG_ERROR(__FILE__,__LINE__,"Bytes  to  deallocate  =  %i  bytes",allocated_bytes);
		MEMMGR_LOG_ERROR(__FILE__,__LINE__,"Mempointers  to  deallocate  =  %i",pointers_to_deallocate);
	}
	else
	{
		MEMMGR_LOG_INFOF(__FILE__,__LINE__,"MEMRAM OK");
	}
}

