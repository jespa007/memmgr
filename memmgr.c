#include	"memmgr.h"



//--------------------------------------------------------------------------------------------
// DEFINES
#define	MAX_MEMPOINTERS					80000
#define	MEMMGR_MAX_FILENAME_LENGTH		50
#define MAX_REGISTER_FILELINES			32

#define	GET_SIZE_PTR(p)		*((int  *)((char  *)p  -  sizeof(int) - sizeof(int)))
#define GET_PREHEADER(p)	((PointerPreHeapInfo    *)((char  *)p-sizeof(PointerPreHeapInfo)))
#define GET_POSTHEADER(p)	((PointerPostHeapInfo  *)((char  *)p+(GET_SIZE_PTR(p))))
#define	KEY_NOT_FOUND		-1

#define LOG_LEVEL_INFO(s, ...)	print(stdout,s, ##__VA_ARGS__)
#define LOG_LEVEL_INFOF(s)		LOG_LEVEL_INFO(s,NULL)

#define LOG_LEVEL_ERROR(s, ...)	print(stderr,s, ##__VA_ARGS__)
#define LOG_LEVEL_ERRORF(s)   	LOG_LEVEL_ERROR(s,NULL)

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
static bool	g_memmgr_was_init  =  false;

static void	*g_allocated_pointer[MAX_MEMPOINTERS]={0};
static int 	g_n_allocated_pointers=0;

static int 	g_idx_free_pointer_slots[MAX_MEMPOINTERS]={0};
static int 	g_n_free_pointer_slots=0;

static 	pthread_mutex_t mutex_main;

//--------------------------------------------------------------------------------------------
static void  MEMMGR_print_status(void);
void  MEMMGR_free_all_allocated_pointers(void);

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

	char command[13];

	/* Command is the control command to the terminal */
	sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
	fprintf(std_type, "%s", command);
}

//--------------------------------------------------------------------------------------------
// LOG UTILS
#ifndef  __GNUC__
#pragma  managed(push,  off)
#endif
void  print(FILE *std, const  char  *string_text, ...) {

	FILE *std_type = stderr;
	char  text[4096] = { 0 };
	va_list  ap;

	va_start(ap,  string_text);
	vsprintf(text,  string_text,  ap);
	va_end(ap);

	//  Results  Are  Stored  In  Text
#ifdef _WIN32
  SetConsoleTextAttribute(GetStdHandle(std==stderr?STD_ERROR_HANDLE:STD_OUTPUT_HANDLE), std==stderr?FOREGROUND_RED:(FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE)   | FOREGROUND_INTENSITY);
#else // ansi color
  MEMMGR_set_color_terminal(std_type, TERM_CMD_BRIGHT, std==stderr?TERM_COLOR_RED:TERM_COLOR_WHITE, TERM_COLOR_BLACK);
#endif
	fprintf(std_type, "%s", text);
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(std==stderr?STD_ERROR_HANDLE:STD_OUTPUT_HANDLE), FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE);
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
		// init allocated pointers data
		g_n_allocated_bytes  =  0;
		g_n_allocated_pointers  =  0;
		memset(g_allocated_pointer,0,sizeof(g_allocated_pointer));

		// init free pointers data
		g_n_free_pointer_slots = MAX_MEMPOINTERS-1;

		for(int i = 0; i < g_n_free_pointer_slots; i++){
			g_idx_free_pointer_slots[i]=MAX_MEMPOINTERS-1-i;
		}

		atexit(MEMMGR_print_status);
		g_memmgr_was_init  =  true;

		LOG_LEVEL_INFOF("******************************");
		LOG_LEVEL_INFOF("Memory management initialized!");
		LOG_LEVEL_INFOF("******************************");

	}
}
//--------------------------------------------------------------------------------------------
int  MEMMGR_get_free_cell_memptr_table(void)
{
	if(g_n_free_pointer_slots > 0){
		return g_idx_free_pointer_slots[g_n_free_pointer_slots];
	}
	return KEY_NOT_FOUND; // no memory free...
}

//--------------------------------------------------------------------------------------------
void 	*MEMMGR_malloc(size_t  size,  const  char  *absolute_filename,  int  line)
{
	if(size==0){
		LOG_LEVEL_ERROR("[%s:%i] Trying to allocate memory as size %i bytes",absolute_filename,line);
		return NULL;
	}

	if((g_n_allocated_pointers)>=(MAX_MEMPOINTERS)){ // array full
		LOG_LEVEL_ERRORF("Memmanager: Error full table");
		return NULL;
	}

	pthread_mutex_lock(&mutex_main);

	PointerPreHeapInfo  *heap_allocat  =  NULL;
	void  * pointer=NULL;
	int  random_number,index;


	if(!g_memmgr_was_init)  MEMMGR_init();  //  auto_inicialize  return  malloc(size);

	heap_allocat  =  (PointerPreHeapInfo  *)malloc(sizeof(PointerPreHeapInfo)  +  sizeof(PointerPostHeapInfo)  +  size);
	if(heap_allocat
			&&
			((index  =  MEMMGR_get_free_cell_memptr_table())  !=  -1))
	{
		heap_allocat->size  =  size;
		heap_allocat->offset_mempointer_table  =  index;

		MEMMGR_get_filename(heap_allocat->filename,  absolute_filename);
		heap_allocat->line  =  line;

		random_number  =  ((unsigned)(rand()%0xFFFF)  <<  16)  |  (rand()%0xFFFF);

		heap_allocat->pre_crc  =  random_number;
		heap_allocat->size        =  size;
		heap_allocat->type_allocator  =  MALLOC_ALLOCATOR;

		g_allocated_pointer[index] 	    = heap_allocat;

		*((int  *)((char  *)heap_allocat+size+sizeof(PointerPreHeapInfo)))  =  random_number;

		g_n_allocated_bytes  +=  size;

		pointer  =  ((char  *)heap_allocat+sizeof(PointerPreHeapInfo));

		// increment the new allocated slot
		g_n_allocated_pointers++;
		g_n_free_pointer_slots--;

		//  memset  pointer
		memset(pointer,0,size);
	}else{
		LOG_LEVEL_ERRORF("Table full of pointers or not enought memory");
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
	if(g_n_allocated_pointers==0){
		LOG_LEVEL_ERRORF("Error empty table");
		return;
	}

	//std::lock_guard<std::mutex> lg(mutex_main);
	pthread_mutex_lock(&mutex_main);

	PointerPreHeapInfo    *preheap_allocat    =  NULL;
	PointerPostHeapInfo  *postheap_allocat  =  NULL;
	void  *base_pointer;

	if(pointer == NULL){
		LOG_LEVEL_ERROR("ERROR:  passed  pointer  is  null  '%s'  at  row  %i.",filename,line);
		goto MEMMGR_free_continue;
	}


	//  Getheaders...
	base_pointer  =  preheap_allocat    =  GET_PREHEADER(pointer);
	postheap_allocat  =  GET_POSTHEADER(pointer);

/*	if(!MEMMGR_is_pointer_registered((intptr_t)((char *)pointer-sizeof(PointerPreHeapInfo))))
	{
		LOG_LEVEL_ERROR("(%s:%i): allocated_pointer NOT REGISTERED OR POSSIBLE MEMORY CORRUPTION?!?!",filename,  line);
		return;
	}*/

	//  Check  headers...
	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)  //  crc  ok  :)
	{
		LOG_LEVEL_ERROR("MEM  ERROR:  Bad  crc  pointer  '%s'  at  line  %i.",filename,line);
		goto MEMMGR_free_continue;
	}

	if(preheap_allocat->offset_mempointer_table  <  0  ||  preheap_allocat->offset_mempointer_table  >=  MAX_MEMPOINTERS)
	{
		LOG_LEVEL_ERROR("MEM  ERROR:  bad  index  mem  table  in  file  '%s'  at  line  %i.",filename,line);
		goto MEMMGR_free_continue;
	}

	//  deallocate  pointer  will  be  ok  :)

	//  Mark  freed...
	g_allocated_pointer[preheap_allocat->offset_mempointer_table]  =  NULL;

	if(g_n_free_pointer_slots>=(MAX_MEMPOINTERS-1)){
		LOG_LEVEL_ERRORF("reached max pointers!");
		goto MEMMGR_free_continue;
	}

	g_n_free_pointer_slots++;
	g_idx_free_pointer_slots[g_n_free_pointer_slots] = preheap_allocat->offset_mempointer_table;
	g_n_allocated_pointers--;
	g_n_allocated_bytes-=preheap_allocat->size;
	free(base_pointer);

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
void  MEMMGR_print_error_on_wrong_deallocate_method(int  allocator, const char *filename, int line)
{


	switch(allocator)
	{
	case  MALLOC_ALLOCATOR:
		LOG_LEVEL_ERROR("ERROR:  g_allocated_pointer  at  filename  '%s'  line  %i  must  freed  with  function  free().",filename,  line);
		break;
	case  NEW_ALLOCATOR:
		LOG_LEVEL_ERROR("ERROR:  g_allocated_pointer  at  filename  '%s'  line  %i  must  freed  with  operator  delete.",filename,  line);
		break;
	case  NEW_WITH_BRACETS_ALLOCATOR:
		LOG_LEVEL_ERROR("ERROR:  g_allocated_pointer  at  filename  '%s'  line  %i  must  freed  with  operator  delete[].",filename,  line);
		break;
	}
}

//----------------------------------------------------------------------------------------
void  MEMMGR_free_from_malloc(void  *p,  const  char  *absolute_filename,  int  line  )
{
	char  filename[MEMMGR_MAX_FILENAME_LENGTH];
	MEMMGR_get_filename(filename,absolute_filename);
	PointerPreHeapInfo  *preheap_allocat  =  NULL;
	PointerPostHeapInfo  *postheap_allocat  =  NULL;


	if(p == NULL)
	{
		LOG_LEVEL_ERROR("ERROR:  NULL  pointer  to  deallocate  at  filename  '%s'  line  %i.",filename,  line);
		return;
	}

	preheap_allocat  =  GET_PREHEADER(p);
	postheap_allocat  =  GET_POSTHEADER(p);

	//  Check  headers...
	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)  //  crc  ok  :)
	{
		LOG_LEVEL_ERROR("MEM  ERROR:  Bad  crc  pointer  '%s'  at  line  %i.",filename,line);
		return;
	}

	if(preheap_allocat->type_allocator  !=  MALLOC_ALLOCATOR)
	{
		MEMMGR_print_error_on_wrong_deallocate_method(preheap_allocat->type_allocator, filename, line);
		return;

	}

	MEMMGR_free(p,  filename,  line);
}
//----------------------------------------------------------------------------------------
void  MEMMGR_free_all_allocated_pointers(void)
{
	void *p;

	for(int i = 0; i < MAX_MEMPOINTERS; i++)
	{
		p = g_allocated_pointer[i];
		if(p)
		{
			//PointerPreHeapInfo  *pre_head  =  (PointerPreHeapInfo *)p;
			MEMMGR_free((char *)p+sizeof(PointerPreHeapInfo),"free_all_allocated_pointers()",0);
			g_allocated_pointer[i] = NULL;
		}
	}
}
//--------------------------------------------------------------------------------------------

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
				LOG_LEVEL_ERROR("%s:%i:Allocated  pointer  NOT  DEALLOCATED (%p).",preheap_allocat->filename,  preheap_allocat->line,g_allocated_pointer[preheap_allocat->offset_mempointer_table]);
			}
		}
	}
	//-----
	if(pointers_to_deallocate>0  ||  allocated_bytes>0)
	{
		LOG_LEVEL_ERROR("bytes  to  deallocate  =  %i  bytes",allocated_bytes);
		LOG_LEVEL_ERROR("Mempointers  to  deallocate  =  %i",pointers_to_deallocate);
	}
	else
	{
		LOG_LEVEL_INFOF("MEMRAM:ok.");
	}
}


