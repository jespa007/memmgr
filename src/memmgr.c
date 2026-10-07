#include	"memmgr.h"
#include	"common.h"

#define MEMMGR_PATTERN_ALLOCATED   0xCD  // newly allocated, uninitialized
#define MEMMGR_PATTERN_FREED       0xDD  // freed memory
#define MEMMGR_PATTERN_GUARD       0xFD  // guard/canary areas
#define MEMMGR_PATTERN_REALLOC_OLD 0xEE  // optional

//--------------------------------------------------------------------------------------------
// DEFINES


#define DEFAULT_C_ALIGNMENT	sizeof(void *)

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

//--------------------------------------------------------------------------------------------
// GLOBAL VARS

static bool g_enable_log=true;

static size_t g_n_allocated_bytes  =  0;
static int	g_n_allocated_pointers  =  0;
static bool	g_memmgr_was_init  =  false;

static void	*g_allocated_pointer[MAX_MEMPOINTERS];
static int 	g_free_pointer_idx[MAX_MEMPOINTERS]={0};
static int 	g_n_free_pointers=0;



static 	pthread_mutex_t mutex_main = PTHREAD_MUTEX_INITIALIZER;

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

	lenght = (((int)strlen(absolute_filename)) - 1);

	if(lenght > 0)
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

void MEMMGR_enableLog(bool _enable_log) {
	g_enable_log=_enable_log;
}



#ifndef  __GNUC__
#pragma  managed(push,  off)
#endif
void  MEMMGR_log(LogType _log_type, const char *_file, int _line, const  char  *string_text, ...) {

	if(g_enable_log==false){
		return;
	}

	char  filename[MEMMGR_MAX_FILENAME_LENGTH+1]={0};
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


#ifndef EMSCRIPTEN
	//  Results  Are  Stored  In  Text
#ifdef _WIN32
  SetConsoleTextAttribute(GetStdHandle(_log_type==LOG_TYPE_ERROR?STD_ERROR_HANDLE:STD_OUTPUT_HANDLE), _log_type==LOG_TYPE_ERROR?FOREGROUND_RED:_log_type==LOG_TYPE_WARNING?(FOREGROUND_RED   | FOREGROUND_GREEN):(FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE));
#else // ansi color
  MEMMGR_set_color_terminal(std_type, TERM_CMD_BRIGHT, _log_type==LOG_TYPE_ERROR?TERM_COLOR_RED:_log_type==LOG_TYPE_WARNING?TERM_COLOR_YELLOW:TERM_COLOR_WHITE, TERM_COLOR_BLACK);
#endif
#endif

	fprintf(std_type, "[ %27s:%04i - %3s]=%s",filename,_line,log_type_str, text);

#ifndef EMSCRIPTEN
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(_log_type==LOG_TYPE_ERROR?STD_ERROR_HANDLE:STD_OUTPUT_HANDLE), FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else // ansi color
	MEMMGR_set_color_terminal(std_type, TERM_CMD_BRIGHT, TERM_COLOR_WHITE, TERM_COLOR_BLACK);
#endif
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

		MEMMGR_LOG_INFO(__FILE__,__LINE__,"******************************");
		MEMMGR_LOG_INFO(__FILE__,__LINE__,"Memory management initialized!");
		MEMMGR_LOG_INFO(__FILE__,__LINE__,"******************************");
		MEMMGR_LOG_INFOF(__FILE__,__LINE__,"mem alloc : %iMb",(sizeof(g_allocated_pointer)+sizeof(g_free_pointer_idx))/(1024*1024));

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

static int MEMMGR_find_pointer_index(void *_ptr) {
    int i;

    if(_ptr == NULL){
        return -1;
    }

    for(i = 0; i < MAX_MEMPOINTERS; ++i){
        PointerPreHeapInfo *pre_header = g_allocated_pointer[i];

        if(pre_header == NULL){
            continue;
        }

        void *user_pointer = GET_POINTER(
            pre_header,
            pre_header->alignment
        );

        if(user_pointer == _ptr){
            return i;
        }
    }

    return -1;
}
/*
static int MEMMGR_find_containing_pointer_index(void *_ptr)
{
    int i;

    if(_ptr == NULL){
        return -1;
    }

    for(i = 0; i < MAX_MEMPOINTERS; ++i){
        PointerPreHeapInfo *pre_header = g_allocated_pointer[i];

        if(pre_header == NULL){
            continue;
        }

        char *user_begin = (char *)GET_POINTER(
            pre_header,
            pre_header->alignment
        );

        char *user_end = user_begin + pre_header->size;

        if((char *)_ptr > user_begin && (char *)_ptr < user_end){
            return i;
        }
    }

    return -1;
}*/

bool MEMMGR_owns_pointer(void *_ptr)
{
    bool owns_pointer;

    pthread_mutex_lock(&mutex_main);

    owns_pointer = MEMMGR_find_pointer_index(_ptr) >= 0;

    pthread_mutex_unlock(&mutex_main);

    return owns_pointer;
}

static const char * MEMMGR_get_allocator_name(int _type_allocator){
	switch(_type_allocator){
	case MALLOC_ALLOCATOR: return "MALLOC_ALLOCATOR";
	case NEW_ALLOCATOR: return "NEW_ALLOCATOR";
	case NEW_WITH_BRACETS_ALLOCATOR: return "NEW_WITH_BRACETS_ALLOCATOR";
	}

	return "UNKNOWN_ALLOCATOR";
}


//--------------------------------------------------------------------------------------------
void 	*MEMMGR_malloc_alignment(size_t  _size,  const  char  *_absolute_filename,  int  _line, size_t _aligment){
	char  filename[MEMMGR_MAX_FILENAME_LENGTH+1] = {0};
	MEMMGR_get_filename(filename,  _absolute_filename);
	// do not register
	if(_size == 0){
		MEMMGR_LOG_WARNING(filename,_line,"Try to allocate pointer with 0 bytes");
		return NULL;
	}

	pthread_mutex_lock(&mutex_main);

	PointerPreHeapInfo  *heap_allocat  =  NULL;
	void  * pointer=NULL;
	int  random_number,index;


	if(!g_memmgr_was_init)  {
		MEMMGR_init();  //  auto_inicialize  return  malloc(size);
	}

	size_t size_of_aligned_header=SIZEOF_ALIGNED_HEADER(_aligment);

	heap_allocat  =  (PointerPreHeapInfo  *)malloc(size_of_aligned_header    +  _size +  sizeof(PointerPostHeapInfo));



	if(heap_allocat
			&&
			((index  =  MEMMGR_get_free_cell_memptr_table())  !=  -1))
	{

		// set begin/end guard blocks
		memset(heap_allocat ,MEMMGR_PATTERN_GUARD,size_of_aligned_header);
		memset((uint8_t *)heap_allocat+ size_of_aligned_header + _size,MEMMGR_PATTERN_GUARD,sizeof(PointerPostHeapInfo));

		// copy data
		strcpy(heap_allocat->filename,filename);
		heap_allocat->size  =  _size;
		heap_allocat->offset_mempointer_table  =  index;
		heap_allocat->alignment = _aligment;


		heap_allocat->line  =  _line;

		random_number  =  ((unsigned)(rand()%0xFFFF)  <<  16)  |  (rand()%0xFFFF);

		heap_allocat->pre_crc  =  random_number;
		heap_allocat->size        =  _size;
		heap_allocat->type_allocator  =  MALLOC_ALLOCATOR;

		g_allocated_pointer[index] 	    = heap_allocat;

		((PointerPostHeapInfo  *)((uint8_t  *)heap_allocat+size_of_aligned_header+_size))->post_crc  =  random_number;

		g_n_allocated_bytes  +=  (int)_size;

		pointer  =  ((uint8_t  *)heap_allocat+size_of_aligned_header);

		g_n_allocated_pointers++;
		g_n_free_pointers--;

		//MEMMGR_LOG_INFOF(NULL,0,"Current allocated pointers: %i of %i (%i%%)",g_n_allocated_pointers,MAX_MEMPOINTERS,(g_n_allocated_pointers*100/MAX_MEMPOINTERS));

	}else{

	    if(heap_allocat != NULL){
	        free(heap_allocat);
	    }

		MEMMGR_LOG_ERROR(__FILE__,__LINE__,"Table full of pointers or not enough memory");
	}

	//malloc_mutex.unlock();
	pthread_mutex_unlock(&mutex_main);

	return pointer;
}

void 	*MEMMGR_malloc(size_t _size,  const  char  *_absolute_filename,  int  _line){
	void *p = MEMMGR_malloc_alignment(_size, _absolute_filename,_line,DEFAULT_C_ALIGNMENT);

	//  memset  pointer
	if( p != NULL){
		memset(p,MEMMGR_PATTERN_ALLOCATED,_size);
	}

	return p;

}
//--------------------------------------------------------------------------------------------
void *MEMMGR_calloc(size_t  n_items,size_t  size_item,  const  char  *absolute_filename,  int  line){
	if(size_item != 0 && n_items > SIZE_MAX / size_item){
	    MEMMGR_LOG_ERROR(
	        absolute_filename,
	        line,
	        "calloc size overflow"
	    );

	    return NULL;
	}

	size_t size = n_items*size_item;

	void * p = MEMMGR_malloc_alignment(size,absolute_filename,line,DEFAULT_C_ALIGNMENT);

	//  memset  pointer
	if(p != NULL){
		memset(p,0,size);
	}

	return p;
}
//--------------------------------------------------------------------------------------------
void  MEMMGR_free_c_pointer(void  *pointer){
	free(pointer);
}

//--------------------------------------------------------------------------------------------
void MEMMGR_free(
    void *_ptr,
    const char *_filename,
    int _line,
    size_t _alignment,
    int _expected_allocator
){
    int pointer_idx;
    PointerPreHeapInfo *pre_header;
    void *base_pointer;

    if(_ptr == NULL){
        return;
    }

    pthread_mutex_lock(&mutex_main);

    pointer_idx = MEMMGR_find_pointer_index(_ptr);

    if(pointer_idx < 0){
        pthread_mutex_unlock(&mutex_main);

        MEMMGR_LOG_ERRORF(
            _filename,
            _line,
            "Pointer was not allocated by MEMMGR: %p",
            _ptr
        );

        return;
    }

    pre_header = g_allocated_pointer[pointer_idx];

    /*
     * Now it is safe to read headers because we know the pointer
     * belongs to MEMMGR.
     */
    if(pre_header->type_allocator != _expected_allocator){
        pthread_mutex_unlock(&mutex_main);

        MEMMGR_LOG_ERRORF(
            _filename,
            _line,
            "Allocator mismatch. Allocated with %s, freed with %s",
            MEMMGR_get_allocator_name(pre_header->type_allocator),
            MEMMGR_get_allocator_name(_expected_allocator)
        );

        return;
    }

    /*
     * Check corruption here.
     */

    g_allocated_pointer[pointer_idx] = NULL;

    base_pointer = pre_header;

    size_t total_size = SIZEOF_ALIGNED_HEADER(_alignment)
        + pre_header->size
        + sizeof(PointerPostHeapInfo);

    memset(base_pointer, MEMMGR_PATTERN_FREED, total_size);

    pthread_mutex_unlock(&mutex_main);

    free(base_pointer);
}

void *MEMMGR_realloc(void *ptr, size_t _size,  const  char  *_filename,  int  _line) {


	if (ptr==NULL) {
		// NULL ptr. realloc should act like malloc.
		return MEMMGR_malloc(_size, _filename, _line);
	}

	if(_size == 0){
	    MEMMGR_free(ptr, _filename, _line, DEFAULT_C_ALIGNMENT, MALLOC_ALLOCATOR);
	    return NULL;
	}


	PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(ptr,DEFAULT_C_ALIGNMENT);


	if ((size_t)pre_head->size >= _size) {
		// We have enough space. Could free some once we implement split.
		return ptr;
	}

	// Need to really realloc. Malloc new space and free old space.
	// Then copy old data to new space.
	void * new_ptr=	MEMMGR_malloc_alignment(_size, _filename,_line,DEFAULT_C_ALIGNMENT);

	//  memset  pointer
	memset(new_ptr,MEMMGR_PATTERN_REALLOC_OLD,_size);
	//new_ptr = MEMMGR_malloc(size, absolute_filename, line);

	if (!new_ptr) {
		return NULL; // TODO: set errno on failure.
	}

	memcpy(new_ptr, ptr, pre_head->size);
	MEMMGR_free(ptr, _filename, _line,DEFAULT_C_ALIGNMENT,MALLOC_ALLOCATOR);

	return new_ptr;
}
//----------------------------------------------------------------------------------------
void  MEMMGR_free_from_malloc(void  *p,  const  char  *_absolute_filename,  int  _line)
{
	char  filename[MEMMGR_MAX_FILENAME_LENGTH+1] = {0};
	MEMMGR_get_filename(filename,_absolute_filename);
	PointerPreHeapInfo  *preheap_allocat  =  NULL;
	PointerPostHeapInfo  *postheap_allocat  =  NULL;


	if(p == NULL)
	{
		MEMMGR_LOG_WARNING(filename,  _line,"NULL  pointer  to  deallocate");
		return;
	}

	preheap_allocat  =  GET_PREHEADER(p,DEFAULT_C_ALIGNMENT);
	postheap_allocat  =  GET_POSTHEADER(p,DEFAULT_C_ALIGNMENT);

	//  Check  headers...
	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)  //  crc  ok  :)
	{
		MEMMGR_LOG_ERROR(filename,_line,"Bad  crc  pointer");
		return;
	}

	MEMMGR_free(p,  filename,  _line, DEFAULT_C_ALIGNMENT,MALLOC_ALLOCATOR);
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
				void *pointer=GET_POINTER(preheap_allocat,DEFAULT_C_ALIGNMENT);//((char *)preheap_allocat)+sizeof(PointerPreHeapInfo);
				MEMMGR_LOG_ERRORF(preheap_allocat->filename,  preheap_allocat->line,"Allocated  pointer  NOT  DEALLOCATED (%p)",pointer);
			}
		}
	}

	//-----
	if(pointers_to_deallocate>0  ||  allocated_bytes>0)
	{
		MEMMGR_LOG_ERRORF(__FILE__,__LINE__,"Bytes  to  deallocate  =  %i  bytes",allocated_bytes);
		MEMMGR_LOG_ERRORF(__FILE__,__LINE__,"Mempointers  to  deallocate  =  %i",pointers_to_deallocate);
	}
	else
	{
		MEMMGR_LOG_INFO(__FILE__,__LINE__,"MEMRAM OK");
	}
}

