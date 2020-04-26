#include	"memmgr.h"


//--------------------------------------------------------------------------------------------
// DEFINES
#define	MAX_MEMPOINTERS			80000
#define	MAX_FILENAME_LENGTH		50
#define MAX_REGISTER_FILELINES	32

#define	GET_SIZE_PTR(p)		*((int  *)((char  *)p  -  sizeof(int) - sizeof(int)))
#define GET_PREHEADER(p)	((PointerPreHeapInfo    *)((char  *)p-sizeof(PointerPreHeapInfo)))
#define GET_POSTHEADER(p)	((PointerPostHeapInfo  *)((char  *)p+(GET_SIZE_PTR(p))))
#define	KEY_NOT_FOUND		-1

#define LOG_INFO(s, ...)	print_log(stdout,s, ##__VA_ARGS__)
#define LOG_ERROR(s, ...)	print_log(stderr,s, ##__VA_ARGS__)

//--------------------------------------------------------------------------------------------
//  turn  off  macros...
#undef	new
#undef	delete
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
}TERM_COLOR;

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
	char  	filename[MAX_FILENAME_LENGTH];
	int  	line;
}InfoAllocatedPointer;

typedef  struct{
	uintptr_t pointer; //this is the element to  be ordered.
	int index;
}PointerDS_Element;

typedef  struct{
	int		type_allocator;
	int		offset_mempointer_table;
	char	filename[MAX_FILENAME_LENGTH];  //  base    		-16-256
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
static PointerDS_Element g_ds_pointer_array[MAX_MEMPOINTERS]; // the same allocatedPointers it will have.




static 	pthread_mutex_t mutex_main;

//--------------------------------------------------------------------------------------------
void  MEMMGR_print_status(void);
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

			}while(*(to_down_ptr-1)  !=  '\\'  &&  *(to_down_ptr-1)  !=  '/'  &&  to_down_ptr  >  absolute_filename && i < MAX_FILENAME_LENGTH);
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
void  print_log(FILE *std, const  char  *string_text, ...) {

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
// DICOTOMIC

int MEMMGR_dicotomic_search(uintptr_t key)
{
	int idx_min=0;
	int idx_max=g_n_allocated_pointers-1;
	// continue searching while [imin,imax] is not empty
	while (idx_max >= idx_min)
	{
		// calculate the midpoint for roughly equal partition
		int idx_mid = (idx_min + idx_max ) >> 1;
		if(g_ds_pointer_array[idx_mid].pointer == key){
			// key found at index idx_mid
			return idx_mid;
		// determine which subarray to search
		}else if (g_ds_pointer_array[idx_mid].pointer < key){
			// change min index to search upper subarray
			idx_min = idx_mid + 1;
		}else{
			// change max index to search lower subarray
			idx_max = idx_mid - 1;
		}
	}
	// key was not found
	return KEY_NOT_FOUND;
}

bool MEMMGR_dicotomic_insert(uintptr_t key, int index)
{
	if(g_n_allocated_pointers==(MAX_MEMPOINTERS-2)){ // array full
		LOG_ERROR("DS Error full table");
		return false;
	}

	// PRE: array is already ordered
	int size=g_n_allocated_pointers;
	int idx_min = 0, idx_max = g_n_allocated_pointers - 1;

	if(size > 0){

		if (key > g_ds_pointer_array[idx_max].pointer){
		  idx_min=size;
		}
		else{
		  while (idx_max > idx_min) {
			int idx_mid = (idx_min + idx_max) >> 1;
			if (g_ds_pointer_array[idx_mid].pointer > key) {
				idx_max = idx_mid;
			}
			else{
				idx_min = idx_mid + 1;
			}
		  }
		}
	}

	if(idx_min >= 0){

		if(idx_min < size){
			for (int i = g_n_allocated_pointers-1; i >= idx_min; --i){
				g_ds_pointer_array[i+1] = g_ds_pointer_array[i];
			}
		}
		g_ds_pointer_array[idx_min].pointer = key;
		g_ds_pointer_array[idx_min].index = index;

		/*for(int i=0; i < (g_n_allocated_pointers-1);i++){
			if(	g_ds_pointer_array[i].pointer > g_ds_pointer_array[i+1].pointer){ // mierda
				int k=0;
				k++;
				LOG_ERROR("Cannot ");
			}
		}*/

		g_n_allocated_pointers++;
		return true;
	}else{
		LOG_ERROR("Cannot insert pointer");
	}
	return false;
}

bool MEMMGR_dicotomic_delete(uintptr_t key)
{
	if(g_n_allocated_pointers==0){
		LOG_ERROR("DS Error empty table");
		return false;
	}

	int pos=MEMMGR_dicotomic_search(key);
	//printf("(pos:%i)",pos);
	if(pos != KEY_NOT_FOUND){
		//memcpy(&g_ds_pointer_array[pos],&g_ds_pointer_array[pos+1],(g_n_allocated_pointers-pos)*sizeof(PointerDS_Element));
		for (int i = pos; i < g_n_allocated_pointers; i++){
			g_ds_pointer_array[i] = g_ds_pointer_array[i+1];
		}
		g_ds_pointer_array[g_n_allocated_pointers-1].pointer=0;
		g_ds_pointer_array[g_n_allocated_pointers-1].index=0;
		g_n_allocated_pointers--;
		return true;
	}else{
		/*for(int i = 0; i < g_n_allocated_pointers; i++){
			printf("\n%p",(void *)g_ds_pointer_array[i].pointer);
		}*/

		LOG_ERROR("Pointer %p not found",(void *)key);
	}
	return false;

}
//--------------------------------------------------------------------------------------------
// MEMMGR Functions
void  MEMMGR_init(void)
{

	if(!g_memmgr_was_init)
	{
		g_n_allocated_bytes  =  0;
		memset(g_allocated_pointer,0,sizeof(g_allocated_pointer));
		g_n_allocated_pointers  =  0;

		g_memmgr_was_init  =  true;
		g_n_free_pointers = MAX_MEMPOINTERS-1;
		memset(&g_ds_pointer_array,0,sizeof(g_ds_pointer_array));


		for(int i = 0; i < g_n_free_pointers; i++){
			g_free_pointer_idx[i]=MAX_MEMPOINTERS-1-i;
		}


		LOG_INFO("******************************");
		LOG_INFO("Memory management initialized!");
		LOG_INFO("******************************");

	}
}

bool  MEMMGR_is_pointer_registered(uintptr_t pointer)
{
	pthread_mutex_lock(&mutex_main);

	int pos = MEMMGR_dicotomic_search(pointer);

	if(pos >=0){
		if(g_ds_pointer_array[pos].pointer == pointer){
			return true;
		}
	}

	pthread_mutex_unlock(&mutex_main);

	return false;
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
void 	*MEMMGR_malloc(size_t  size,  const  char  *absolute_filename,  int  line)
{

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

		//------------------------------------------------------------
		// insert to get pointer faster through dicotomic search...
		MEMMGR_dicotomic_insert((uintptr_t)heap_allocat, index);
		//------------------------------------------------------------

		g_n_free_pointers--;

		//  memset  pointer
		memset(pointer,0,size);
	}else{
		LOG_ERROR("Table full of pointers or not enought memory");
	}


	//malloc_mutex.unlock();
	pthread_mutex_unlock(&mutex_main);

	return pointer;
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

	if(pointer)
	{

		//  Getheaders...
		base_pointer  =  preheap_allocat    =  GET_PREHEADER(pointer);
		postheap_allocat  =  GET_POSTHEADER(pointer);

		//  Check  headers...
		if(preheap_allocat->pre_crc  ==  postheap_allocat->post_crc)  //  crc  ok  :)
		{

			if(preheap_allocat->offset_mempointer_table  >=  0  &&  preheap_allocat->offset_mempointer_table  <=  MAX_MEMPOINTERS)
			{
				//  deallocate  pointer  will  be  ok  :)


				//  Mark  freed...
				g_allocated_pointer[preheap_allocat->offset_mempointer_table]  =  NULL;

				if(g_n_free_pointers<(MAX_MEMPOINTERS-1)){
					g_n_free_pointers++;
					g_free_pointer_idx[g_n_free_pointers] = preheap_allocat->offset_mempointer_table;
				}
				else {
					LOG_ERROR("reached max pointers!");
				}


				//-----------------------------------------------------------------
				// DS delete element ...
				MEMMGR_dicotomic_delete((uintptr_t)base_pointer);
				//----------------------------------------------

				g_n_allocated_bytes-=preheap_allocat->size;
				free(base_pointer);

			}
			else
			{
				LOG_ERROR("MEM  ERROR:  bad  index  mem  table  in  file  \"%s\"  at  line  %i.",filename,line);
			}
		}
		else
		{
			LOG_ERROR("MEM  ERROR:  Bad  crc  pointer  \"%s\"  at  line  %i.",filename,line);
		}
	}
	else
	{
		 LOG_ERROR("ERROR:  passed  pointer  is  null  \"%s\"  at  row  %i.",filename,line);
	}

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
		LOG_ERROR("ERROR:  g_allocated_pointer  at  filename  \"%s\"  line  %i  must  freed  with  function  free().",filename,  line);
		break;
	case  NEW_ALLOCATOR:
		LOG_ERROR("ERROR:  g_allocated_pointer  at  filename  \"%s\"  line  %i  must  freed  with  operator  delete.",filename,  line);
		break;
	case  NEW_WITH_BRACETS_ALLOCATOR:
		LOG_ERROR("ERROR:  g_allocated_pointer  at  filename  \"%s\"  line  %i  must  freed  with  operator  delete[].",filename,  line);
		break;
	}
}

//----------------------------------------------------------------------------------------
void  MEMMGR_free_from_malloc(void  *p,  const  char  *absolute_filename,  int  line  )
{
	char  filename[MAX_FILENAME_LENGTH];
	MEMMGR_get_filename(filename,absolute_filename);

	if(p)
	{
		PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(p);

		if(pre_head->type_allocator  ==  MALLOC_ALLOCATOR)
		{
			MEMMGR_free(p,  filename,  line);
		}
		else  //  error
		{
			MEMMGR_print_error_on_wrong_deallocate_method(pre_head->type_allocator, filename, line);
		}
	}
	else
	{
		LOG_ERROR("ERROR:  NULL  pointer  to  deallocate  at  filename  \"%s\"  line  %i.",filename,  line);
	}
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

	for(i  =  0;  i  <  MAX_MEMPOINTERS;  i++)
	{
		if((preheap_allocat  =  (PointerPreHeapInfo    *)g_allocated_pointer[i]))
		{
			if(preheap_allocat->line>0 && (strcmp("??",preheap_allocat->filename)!=0)) // leak from others libs
			{
				LOG_ERROR("%s:%i:Allocated  pointer  NOT  DEALLOCATED (%p).",preheap_allocat->filename,  preheap_allocat->line,g_allocated_pointer[preheap_allocat->offset_mempointer_table]);
			}
		}
	}
	//-----
	if(g_n_allocated_pointers>0  ||  g_n_allocated_bytes>0)
	{
		LOG_ERROR("bytes  to  deallocate  =  %i  bytes",g_n_allocated_bytes);
		LOG_ERROR("Mempointers  to  deallocate  =  %i",g_n_allocated_pointers);
	}
	else
	{
		LOG_INFO("MEMRAM:ok.");
	}
}


