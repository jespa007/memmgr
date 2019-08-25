#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<memory.h>
#include	<stdarg.h>
#include	<atomic>
#include	<mutex>

#ifdef _WIN32
#include	<windows.h>
#endif

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

#ifdef __MEMMGR__

	//--------------------------------------------------------------------------------------------
	//  turn  off  macros...
	#undef	new
	#undef	delete
	#undef    malloc
	#undef    free


	typedef enum{
		RESET = 0,
		BRIGHT = 1,
		DIM = 2,
		UNDERLINE = 3,
		BLINK = 4,
		REVERSE = 7,
		HIDDEN = 8

	}TERM_CMD;

	typedef enum{

		BLACK = 0,
		RED = 1,
		GREEN = 2,
		YELLOW = 3,
		BLUE = 4,
		MAGENTA = 5,
		CYAN = 6,
		WHITE = 7
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
		void  	*ptr;
		char  	filename[MAX_FILENAME_LENGTH];
		int  	line;
	}InfoAllocatedPointer;

	typedef  struct{
		void *pointer; //this is the element to  be ordered.
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

	static int	n_allocated_bytes  =  0;
	static int	n_allocated_pointers  =  0;
	static bool	memmgr_was_init  =  false;

	static void	*allocated_pointer[MAX_MEMPOINTERS];
	static int 	free_pointer_idx[MAX_MEMPOINTERS]={0};
	static int 	n_free_pointers=0;
	static PointerDS_Element ds_pointer_array[MAX_MEMPOINTERS]; // the same allocatedPointers it will have.


	static char registered_file[MAX_REGISTER_FILELINES][MAX_FILENAME_LENGTH]={0};
	static int registered_line[MAX_REGISTER_FILELINES]={-1};
	static int n_registered_file_line=0;

	static 	std::mutex alloc_dealloc_mutex;

	//--------------------------------------------------------------------------------------------
	void  MEMMGR_print_status(void);
	void  MEMMGR_free_all_allocated_pointers( );

	//--------------------------------------------------------------------------------------------
	// DICOTOMIC
	int MEMMGR_search_index_to_insert(PointerDS_Element *array, size_t size, void * key)
	{
		// PRE: array is already ordered
		int idx_min = 0, idx_max = size - 1;

		if (key <= array[idx_max].pointer){
		  idx_min=size;
		}
		else{
		  while (idx_max > idx_min) {
			int idx_mid = (idx_min + idx_max) >> 1;
			if (array[idx_mid].pointer > key) {
				idx_max = idx_mid;
			}
			else{
				idx_min = idx_mid + 1;
			}
		  }
		}

		return idx_min;
	}

	int MEMMGR_dicotomic_search(PointerDS_Element * array, void * key, int idx_min, int idx_max)
	{
		// continue searching while [imin,imax] is not empty
		while (idx_max >= idx_min)
		{
			// calculate the midpoint for roughly equal partition
			int idx_mid = (idx_min + idx_max ) >> 1;
			if(array[idx_mid].pointer == key){
				// key found at index idx_mid
				return idx_mid;
			// determine which subarray to search
			}else if (array[idx_mid].pointer < key){
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
	//--------------------------------------------------------------------------------------------
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
	  SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), FOREGROUND_RED   | FOREGROUND_INTENSITY);
	#else // ansi color
	  MEMMGR_set_color_terminal(std_type, TERM_CMD::BRIGHT, TERM_COLOR::RED, TERM_COLOR::BLACK);
	#endif
		fprintf(std_type, "%s", text);
	#ifdef _WIN32
		SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE);
	#else // ansi color
		MEMMGR_set_color_terminal(std_type, TERM_CMD::BRIGHT, TERM_COLOR::WHITE, TERM_COLOR::BLACK);
	#endif


		fprintf(std_type, "\n");
		fflush(std_type);
	}

	#ifndef  __GNUC__
	#pragma  managed(pop)
	#endif

	void  MEMMGR_init()
	{

		if(!memmgr_was_init)
		{
			n_allocated_bytes  =  0;
			memset(allocated_pointer,0,sizeof(allocated_pointer));
			n_allocated_pointers  =  0;

			memmgr_was_init  =  true;
			n_free_pointers = MAX_MEMPOINTERS-1;
			memset(&ds_pointer_array,-1,sizeof(ds_pointer_array));


			for(int i = 0; i < n_free_pointers; i++){
				free_pointer_idx[i]=MAX_MEMPOINTERS-1-i;
			}


			LOG_INFO("*******************************");
			LOG_INFO("Memory mannagement initialized!");
			LOG_INFO("*******************************");

		}
	}

	bool  MEMMGR_is_pointer_registered(void *pointer)
	{
		int pos = MEMMGR_dicotomic_search(ds_pointer_array,pointer, 0, n_allocated_pointers);

		if(pos >=0){
			if(ds_pointer_array[pos].pointer == pointer)
				return true;
		}

		return false;
	}
	//--------------------------------------------------------------------------------------------
	int  MEMMGR_get_free_cell_memptr_table()
	{
		if(n_free_pointers > 0){
			return free_pointer_idx[n_free_pointers];
		}
		return KEY_NOT_FOUND; // no memory free...
	}

	//--------------------------------------------------------------------------------------------
	void 	*MEMMGR_malloc(size_t  size,  const  char  *absolute_filename,  int  line, bool already_performed_mutex)
	{
		if(!already_performed_mutex){
			alloc_dealloc_mutex.lock();
		}

		PointerPreHeapInfo  *heap_allocat  =  NULL;
		std::atomic<void  *> pointer(NULL);
		int  random_number,index;


		if(!memmgr_was_init)  MEMMGR_init();  //  auto_inicialize  return  malloc(size);

		heap_allocat  =  (PointerPreHeapInfo  *)malloc(sizeof(PointerPreHeapInfo)  +  sizeof(PointerPostHeapInfo)  +  size);
		if(heap_allocat
				&&
				((index  =  MEMMGR_get_free_cell_memptr_table())  !=  -1))
		{
			heap_allocat->size  =  size;
			heap_allocat->offset_mempointer_table  =  index;

			MEMMGR_get_filename(heap_allocat->filename,  absolute_filename);
			heap_allocat->line  =  line;

			random_number  =  (unsigned(rand()%0xFFFF)  <<  16)  |  (rand()%0xFFFF);

			heap_allocat->pre_crc  =  random_number;
			heap_allocat->size        =  size;
			heap_allocat->type_allocator  =  MALLOC_ALLOCATOR;

			allocated_pointer[index] 	    = heap_allocat;

			*((int  *)((char  *)heap_allocat+size+sizeof(PointerPreHeapInfo)))  =  random_number;

			n_allocated_bytes  +=  size;
			n_allocated_pointers++;

			pointer  =  ((char  *)heap_allocat+sizeof(PointerPreHeapInfo));

			//------------------------------------------------------------
			// insert to get pointer faster through dicotomic search...
			int pos=MEMMGR_search_index_to_insert(ds_pointer_array,n_allocated_pointers,pointer);
			if(pos >= 0){
				for (int i = n_allocated_pointers-1; i >= pos; --i){
					ds_pointer_array[i+1] = ds_pointer_array[i];
				}
				ds_pointer_array[pos].pointer = heap_allocat;
				ds_pointer_array[pos].index = index;
			}
			//------------------------------------------------------------

			n_free_pointers--;

			//  memset  pointer
			memset(pointer,0,size);
		}else{
			LOG_ERROR("Table full of pointers or not enought memory");
		}

		if(!already_performed_mutex){
			alloc_dealloc_mutex.unlock();
		}

		return pointer;
	}
	//--------------------------------------------------------------------------------------------
	void  MEMMGR_free(void  *pointer,  const  char  *filename,  int  line)
	{

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
					allocated_pointer[preheap_allocat->offset_mempointer_table]  =  NULL;

					if(n_free_pointers<(MAX_MEMPOINTERS-1)){
						n_free_pointers++;
						free_pointer_idx[n_free_pointers] = preheap_allocat->offset_mempointer_table;
					}
					else {
						LOG_ERROR("reached max pointers!");
					}


					//-----------------------------------------------------------------
					// DS delete element ...
					int pos = MEMMGR_dicotomic_search(ds_pointer_array,base_pointer, 0, n_allocated_pointers);
					if(pos >= 0){
						memcpy(&ds_pointer_array[pos],&ds_pointer_array[pos+1],(n_allocated_pointers-pos)*sizeof(PointerDS_Element));
					}
					//----------------------------------------------

					n_allocated_bytes-=preheap_allocat->size;
					n_allocated_pointers--;
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
	}

	void *MEMMGR_realloc(void *ptr, size_t size,  const  char  *absolute_filename,  int  line) {


		alloc_dealloc_mutex.lock();


		if (ptr==NULL) {
			// NULL ptr. realloc should act like malloc.
			return MEMMGR_malloc(size, absolute_filename, line,true);
		}

		PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(ptr);


		if ((size_t)pre_head->size >= size) {
			// We have enough space. Could free some once we implement split.
			return ptr;
		}

		// Need to really realloc. Malloc new space and free old space.
		// Then copy old data to new space.
		std::atomic<void *>new_ptr(NULL);
		new_ptr = MEMMGR_malloc(size, absolute_filename, line,true);

		if (!new_ptr) {
			return NULL; // TODO: set errno on failure.
		}
		memcpy(new_ptr, ptr, pre_head->size);
		MEMMGR_free(ptr, absolute_filename, line);

		alloc_dealloc_mutex.unlock();

		return new_ptr;
	}
	//--------------------------------------------------------------------------------------------
	void  MEMMGR_print_error_on_wrong_deallocate_method(int  allocator, const char *filename, int line)
	{


		switch(allocator)
		{
		case  MALLOC_ALLOCATOR:
			LOG_ERROR("ERROR:  allocated_pointer  at  filename  \"%s\"  line  %i  must  freed  with  function  free().",filename,  line);
			break;
		case  NEW_ALLOCATOR:
			LOG_ERROR("ERROR:  allocated_pointer  at  filename  \"%s\"  line  %i  must  freed  with  operator  delete.",filename,  line);
			break;
		case  NEW_WITH_BRACETS_ALLOCATOR:
			LOG_ERROR("ERROR:  allocated_pointer  at  filename  \"%s\"  line  %i  must  freed  with  operator  delete[].",filename,  line);
			break;
		}
	}

	//----------------------------------------------------------------------------------------
	void  MEMMGR_pre_check_and_free(void  *p,  const  char  *absolute_filename,  int  line  )
	{

		alloc_dealloc_mutex.lock();

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

		alloc_dealloc_mutex.unlock();

	}
	//----------------------------------------------------------------------------------------
	void  MEMMGR_free_all_allocated_pointers()
	{

		void *p;

		for(int i = 0; i < MAX_MEMPOINTERS; i++)
		{
			p = allocated_pointer[i];
			if(p)
			{
				//PointerPreHeapInfo  *pre_head  =  (PointerPreHeapInfo *)p;
				MEMMGR_free((char *)p+sizeof(PointerPreHeapInfo),"free_all_allocated_pointers()",0);
				allocated_pointer[i] = NULL;
			}
		}
	}
	//--------------------------------------------------------------------------------------------
	#ifdef  __cplusplus
	bool	MEMMGR_push_file_line(const  char  *absolute_filename,   int   line)
	{

		if(n_registered_file_line==0){ // The first! can be locked!
			alloc_dealloc_mutex.lock();
		}

		if(n_registered_file_line < MAX_REGISTER_FILELINES)
		{
			MEMMGR_get_filename(registered_file[n_registered_file_line],absolute_filename);
			registered_line[n_registered_file_line]=line;
			n_registered_file_line++;
		}
		else
		{
			LOG_INFO("reached max stacked files!");
			//return false;
		}

		return true;
	}

	void*  operator  new(size_t  size)
	{
		const char *source_file = "??";
		int source_line = 0;

		if(n_registered_file_line > 0)
		{
			source_file = registered_file[n_registered_file_line-1];
			source_line = registered_line[n_registered_file_line-1];
		}

		std::atomic<void *> ret_ptr(NULL);
		void * pointer  =  MEMMGR_malloc(size,source_file,source_line,true);

		if(pointer!=NULL)
		{
			PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer);
			pre_head->type_allocator  =  NEW_ALLOCATOR;
		}

		// set as not registered pointer ...
		if(n_registered_file_line > 0){
			n_registered_file_line--;
		}

		ret_ptr  = pointer;
		if(n_registered_file_line == 0){ // 0 can unlock
			alloc_dealloc_mutex.unlock();
		}

		return  ret_ptr;
	}
	//--------------------------------------------------------------------------------------------
	void*  operator  new[](size_t  size)
	{

		const char *source_file = "??";
		int source_line = 0;

		if(n_registered_file_line > 0)
		{
			source_file = registered_file[n_registered_file_line-1];
			source_line = registered_line[n_registered_file_line-1];
		}

		std::atomic<void *> ret_ptr(NULL);
		void *pointer = NULL;

		pointer  =  MEMMGR_malloc(size,source_file, source_line);
		PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer);
		pre_head->type_allocator  =  NEW_WITH_BRACETS_ALLOCATOR;

		// set as not registered pointer ...
		if(n_registered_file_line > 0){
			n_registered_file_line--;
		}

		ret_ptr=pointer;

		if(n_registered_file_line == 0){ // 0 can unlock
			alloc_dealloc_mutex.unlock();
		}

		return  ret_ptr;

	}
	//--------------------------------------------------------------------------------------------
	void  operator  delete(void  *pointer) throw()
	{
		const char *source_file = "??";
		int source_line = 0;

		if(n_registered_file_line > 0)
		{
			source_file = registered_file[n_registered_file_line-1];
			source_line = registered_line[n_registered_file_line-1];
		}

		if(pointer)
		{
			if(!MEMMGR_is_pointer_registered((char *)pointer-sizeof(PointerPreHeapInfo)))
			{
				LOG_ERROR("(%s:%i): allocated_pointer NOT REGISTERED OR POSSIBLE MEMORY CORRUPTION?!?!",source_file,  source_line);
			}
			else
			{

				PointerPreHeapInfo   *pre_head   =  GET_PREHEADER(pointer);
				PointerPostHeapInfo  *post_head  =  GET_POSTHEADER(pointer);

				if(pre_head->pre_crc  ==  post_head->post_crc)
				{
					if(pre_head->type_allocator  ==  NEW_ALLOCATOR)
					{
						MEMMGR_free(pointer,  source_file,  source_line);
					}
					else  //  error
					{
						MEMMGR_print_error_on_wrong_deallocate_method(pre_head->type_allocator,  source_file,  source_line);
					}
				}
				else
				{
					LOG_ERROR("(%s:%i): CRC  error!",source_file,  source_line);
				}
			}
		}
		else
		{
			LOG_ERROR("ERROR:  NULL  pointer  to  deallocate  at  filename  \"%s\"  line  %i.",source_file,  source_line);
		}

		if(n_registered_file_line > 0){
			n_registered_file_line--;
		}

		if(n_registered_file_line == 0){ // 0 can unlock
			alloc_dealloc_mutex.unlock();
		}

	}
	//--------------------------------------------------------------------------------------------
	void  operator  delete[](void  *pointer) throw()
	{
		const char *source_file = "??";
		int source_line = 0;

		if(n_registered_file_line > 0)
		{
			source_file = registered_file[n_registered_file_line-1];
			source_line = registered_line[n_registered_file_line-1];
		}

		if(pointer!=NULL)
		{

			if(!MEMMGR_is_pointer_registered((char *)pointer-sizeof(PointerPreHeapInfo)))
			{
				LOG_ERROR("(%s:%i): allocated_pointer NOT REGISTERED WITH MALLOC OR NEW!",source_file,  source_line);
			}
			else
			{

				PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer);

				if(pre_head->type_allocator  ==  NEW_WITH_BRACETS_ALLOCATOR)
				{
					MEMMGR_free(pointer,  source_file,  source_line);
				}
				else  //  error
				{
					MEMMGR_print_error_on_wrong_deallocate_method(pre_head->type_allocator,  source_file,  source_line);
				}
			}
		}
		else
		{
			LOG_ERROR("ERROR:  NULL  pointer  to  deallocate  at  filename  \"%s\"  line  %i",source_file,  source_line);
		}

		if(n_registered_file_line > 0){
			n_registered_file_line--;
		}

		if(n_registered_file_line == 0){ // 0 can unlock
			alloc_dealloc_mutex.unlock();
		}

	}
	#endif
	//--------------------------------------------------------------------------------------------
	void  MEMMGR_print_status(void)
	{
		PointerPreHeapInfo    *preheap_allocat;
		int  i;

		for(i  =  0;  i  <  MAX_MEMPOINTERS;  i++)
		{
			if((preheap_allocat  =  (PointerPreHeapInfo    *)allocated_pointer[i]))
			{
				if(preheap_allocat->line>0 && (strcmp("??",preheap_allocat->filename)!=0)) // leak from others libs
				{
					LOG_ERROR("%s:%i:Allocated  pointer  NOT  DEALLOCATED (%p).",preheap_allocat->filename,  preheap_allocat->line,allocated_pointer[preheap_allocat->offset_mempointer_table]);
				}
			}
		}
		//-----
		if(n_allocated_pointers>0  ||  n_allocated_bytes>0)
		{
			LOG_ERROR("bytes  to  deallocate  =  %i  bytes",n_allocated_bytes);
			LOG_ERROR("Mempointers  to  deallocate  =  %i",n_allocated_pointers);
		}
		else
		{
			LOG_INFO("MEMRAM:ok.");
		}
	}

#endif
