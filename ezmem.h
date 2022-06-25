#ifndef EZMEM
 #define EZMEM

//////////////////////////////////////////////////////////// srcs/init.h
// Include external functions
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
// Define useful macros
#define STR(S) #S

// Define some internal constants
#define MAIN_FOLDER ".ezmem"
#define MEM_FOLDER ".ezmem/mem"
#define SUMMARY_FILE ".ezmem/summary.memlog"
#define LEAKS_FOLDER ".ezmem/leaks"
#define IDS_FILE ".ezmem/.ids.memid"
#define README_FILE ".ezmem/README.txt"

// Define structs
typedef struct s_location
{
	size_t		line;
	const char	*func;
	const char	*file;
}	t_location;

typedef struct s_mem_blok
{
	void	*ptr;
	size_t	siz;
	t_location	loc;
}	t_memblk;

//////////////////////////////////////////////////////////// srcs/utils_io.h
static inline size_t str_len( const char* str )
{
	size_t	len;

	len = 0;
	while (str && str[len])
		len++;
	return ( len );
}


static inline void put_chr( int fd, char c )
{
	write( fd, &c, 1 );
}

void	put_str( int fd, char* str )
{
	write( fd, str, str_len( str ) );
}

//////////////////////////////////////////////////////////// srcs/utils_file.h
static inline void create_dir(char* path)
{
	int ret = 0;
	struct stat st = { 0 };

	if (stat(path, &st) == -1)
	{
		ret = mkdir(path, 0700);
	}
	if (ret)
	{
		//TODO: error
	}
}

static inline int create_file(char* path, void (*func)(int fd))
{
	int fd = 0;
	struct stat st = { 0 };

	if (stat(path, &st) == -1)
	{
		fd = open(path, O_CREAT | O_RDWR, 0700);
	}
	if (fd < 1)
	{
		// TODO: error
	}
	if (func)
	{
		func(fd);
	}

	close(fd);
}

//////////////////////////////////////////////////////////// srcs/utils_nbr.h
static inline ssize_t put_nbr( int fd, ssize_t num )
{
	ssize_t	ret;

	ret = 0;
	if (num >= 10)
	{
		ret += put_nbr( fd, num / 10 );
		ret += put_nbr( fd, num % 10 );
	}
	if (num < 10)
	{
		put_chr( fd, '0' + num );
		ret += 1;
	}
	return ( ret );
}

static inline ssize_t put_nbr_base( int fd, ssize_t num, ssize_t base, char* b_chars )
{
	ssize_t	ret;

	ret = 0;
	if (num >= base)
	{
		ret += put_nbr_base( fd, num / base, base, b_chars );
		ret += put_nbr_base( fd, num % base, base, b_chars );
	}
	if (num < base)
	{
		put_chr( fd, b_chars[num] );
		ret += 1;
	}
	return ( ret );
}

static inline int	is_wsp( char c )
{
	if (c == '\t' || c == '\v' || c == '\f' || c == '\r' || c == ' ')
		return ( 1 );
	return ( 0 );
}

static inline ssize_t str_to_nbr( char* s )
{
	ssize_t	num;
	int		sign;

	num = 0;
	sign = 1;
	while (s && *s && is_wsp( *s ))
		s++;
	while (s && *s >= '0' && *s <= '9')
		num = ( num * 10 ) + ( *s++ - '0' );
	if (num > LONG_MAX)
	{
		if (sign == 1)
			return ( -1 );
		else
			return ( 0 );
	}
	if (sign == 1)
		return ( num );
	else
		return ( -num );
	return ( 0 );
}

static inline ssize_t str_to_nbr_base( char* s, int base )
{
	ssize_t	num;
	int		sign;

	num = 0;
	sign = 1;
	while (s && *s && is_wsp( *s ))
		s++;
	while (s && *s >= '0' && *s <= '9')
		num = ( num * base ) + ( *s++ - '0' );
	if (num > LONG_MAX)
	{
		if (sign == 1)
			return ( -1 );
		else
			return ( 0 );
	}
	if (sign == 1)
		return ( num );
	else
		return ( -num );
	return ( 0 );
}

//////////////////////////////////////////////////////////// srcs/id_management.h

#define ID_MAX_LEN 42

static inline int get_curr_id( size_t* num_ptr )
{
	size_t		number = 0;
	char		input[ID_MAX_LEN] = { [0 ... ( ID_MAX_LEN - 1 )] = 0 }; // Use designated initializers to zero the array
	int			fd = -1;
	int			ret = 0;

	fd = open( IDS_FILE, O_RDONLY );
	if (fd < 0)
		return ( 1 );

	ret = read( fd, &input, ID_MAX_LEN ); // read from file into buffer
	if (ret < 0)
		return ( 1 );

	input[ret] = '\0'; // NULL terminate input buffer

	number = str_to_nbr( input );
	*num_ptr = number;
	close( fd );
	return( 0 );
}

static inline int update_id( size_t curr_id )
{
	int			fd = -1;
	int			ret = 0;

	fd = open( IDS_FILE, O_CREAT | O_TRUNC, 0600 );
	if (fd < 0)
		return ( 1 );

	put_nbr( fd, curr_id );

	close( fd );
	return( 0 );
}

//////////////////////////////////////////////////////////// srcs/constructor.h
static inline void	constructor() __attribute__( ( constructor ) );

static inline void writ_readme( int fd )
{
	static char str[] = "\
		Content of the EZMEM folder : \n\
			- mem/ :\n\
				Contains one file for each memory block\n\
			- leaks/ :\n\
				Contains one file per memory blocks that has never been freed\n\
			- summary.memlog :\n\
				Contains log of memory alloc / frees during execution\n\
			- .ids.memid :\n\
				Helper internal file tracking current ID\n\
		";

	put_str( fd, str );
}

static inline void writ_init_id( int fd )
{
	put_str( fd, "0" );
}

static inline int get_memid( long long* num_ptr );

static inline void	constructor()
{
	struct stat st = { 0 };
	if (stat( MAIN_FOLDER, &st ) != -1)
	{
		system( "rm -rf ./" MAIN_FOLDER );
	}

	create_dir( MAIN_FOLDER );
	create_dir( MEM_FOLDER );
	create_dir( LEAKS_FOLDER );

	create_file( SUMMARY_FILE, NULL );
	create_file( IDS_FILE, writ_init_id );
	create_file( README_FILE, writ_readme );
}

//////////////////////////////////////////////////////////// srcs/destructor.h
static inline void	destructor() __attribute__((destructor));

static inline void	destructor()
{
	// code here
}

//////////////////////////////////////////////////////////// srcs/wrap.h
typedef enum e_allo_or_free
{
	ALLO,
	FREE
}	t_aof;

static inline void output_data( t_memblk *mem, t_aof aof )
{
	size_t id = 0;

	// ID management
	//	- GET ID
	if (get_curr_id( &id ))
	{
		// TODO: error
	}
	//	- INCREMENT ID

	// Open file
	// Write DATA to file
	//	- ID
	//	- SIZ
	//	- LOCATION
}


static inline void *_WRAPPED_malloc( size_t size, size_t line, const char *func, const char *file )
{
	t_memblk	mem = ( t_memblk ){ NULL, size, ( t_location ) { line, func, file } };
	void *ptr = NULL;

	mem.ptr = malloc( size ); // Call real malloc

	output_data( &mem, ALLO );

	return ( mem.ptr );
}

static inline void	_WRAPPED_free( void *ptr, int line, const char *func, const char *file )
{
	// code here
	t_memblk	mem = ( t_memblk ){ NULL, 0, ( t_location ) { line, func, file } };

	output_data( &mem, FREE );

	free( ptr );
}

# define malloc(x) _WRAPPED_malloc(x, __LINE__, __FUNCTION__, __FILE__)
# define free(x) _WRAPPED_free(x, __LINE__, __FUNCTION__, __FILE__)

#endif //EZMEM
