/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 ****************************************************************************/

/*static char rcsid[] = "$Id: comm.c,v 1.150 2004/04/06 22:00:09 dotd Exp $";*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#include "mud.h"
#include "gsn.h"
#include "recylist.h"
#ifdef MXP
#include "mxp.h"
#endif
#include "christen.h"
#ifdef IRC
#include "irc.h"
#endif
#include "bugtrack.h"

/*
 * Socket and TCP/IP stuff.
 */
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#define TELOPTS
#define TELCMDS
#include <arpa/telnet.h>
#include <netdb.h>

#ifdef STRING_HASH_SEMAPHORE
#include <semaphore.h>
#endif

#define MAX_PC_RACE     19
#define MAX_CHOICES     20

#define SOLARIS 1

char * const ch_class_choice[MAX_PC_RACE][MAX_CHOICES] = {
    /* Human  */ { "m", "c", "w", "t", "d", "o", "b", "p", "r", "i", "z", 0 },
    /* Grey Elf */ { "m", "c", "w", "t", "d", "m", "r", "i", "mt", "cr", "wt", "mw", "mwt", 0 },
    /* Dwarven */ { "c", "w", "t", "p", "r", "wt", "wc", "ct", "cp", 0 },
    /* Halfling */ { "t", "m", "w", "c", "wt", "ct", "tr", "wct", 0 },
    /* Gnome  */ { "m", "c", "w", "t", "e", "wt", "mw", "mt", "mc", 0 },
    /* Orcish */ { "w", "t", "c", "b", "cb", "ct", "wt", 0 },
    /* Goblinoid */ { "w", "t", "c", "b", "wt", "tb", 0 },
    /* Trollish */ { "b", "cb", 0 },
    /* Mindflayer */ { "i", 0 },
    /* Drow   */ { "m", "c", "w", "t", "wc", "mw", "mt", "mc", "mwt", 0 },
/*rem*//* Aarakocra */ { "m", "c", "w", "t", "mt", "ct", "wt", 0 },
    /* Half Elven */ { "m", "c", "w", "t", "d", "m", "p", "r", "i", "mw", "wt", "mt", "mc", "wd", "wc", "cr", "mwt", "mwc", 0 },
/*rem*//* Half Ogre */ { "c", "w", "b", "d", "wc", "wd", 0 },
/*rem*//* Half Orc */ { "c", "w", "t", "ct", "wt", "wct", 0 },
    /* Half Giant */ { "w", "p", "r", "z", 0 },
    /* Dark Dwarf */ { "c", "w", "t", "wt", "wc", "ct", 0 },
    /* Deep Gnome */ { "m", "c", "w", "t", "e", "wt", "mw", "mt", "mc", 0 },
    /* High Elf */ { "m", "c", "w", "t", "d", "m", "p", "r", "i", "mw", "wt", "mt", "mwt", "cr", 0 },
/*rem*//* Sylvan Elf */ { "m", "c", "w", "t", "d", "m", "p", "r", "i", "mw", "wt", "mt", "cr", "mwt", "ctr", 0 }
    /* Tiefling */ /*{ "m", "c", "w", "t", "mt", "ct", "wt", 0 },*/
    /* Githzerai */ /*{ "m", "t", "w", "wm", "wt", "i", "o", 0 }*/
};

const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const char eor_on_str	[] = { IAC, WILL, TELOPT_EOR, '\0' };

#ifndef MUD_LISTENER
#define TELOPT_COMPRESS 85
#define TELOPT_COMPRESS2 86
const char compress_on_str	[] = { IAC, WILL, TELOPT_COMPRESS, '\0' };
const char compress2_on_str    [] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };
char    enable_compress 	[] = { IAC, SB, TELOPT_COMPRESS, WILL, SE, 0 };
char    enable_compress2	[] = { IAC, SB, TELOPT_COMPRESS2, IAC, SE, 0 };

#ifdef COMPRESS
bool    start_compression(DESCRIPTOR_DATA *d, unsigned char telopt);
bool    stop_compression(DESCRIPTOR_DATA *d, unsigned char telopt);
#endif
#endif

#define TELOPT_MSP 90 /* Mud Sound Protocol */
const char	msp_on_str	[] = { IAC, WILL, TELOPT_MSP, '\0' };
char	enable_msp		[] = { IAC, SB, TELOPT_MSP, IAC, SE, 0 };
#define TELOPT_MXP 91 /* Mud eXtension Protocol */
const char	mxp_on_str	[] = { IAC, WILL, TELOPT_MXP, '\0' };
char	enable_mxp		[] = { IAC, SB, TELOPT_MXP, IAC, SE, 0 };

const char 	go_ahead_str	[] = { IAC, GA, '\0' };

void    send_auth args( ( struct descriptor_data *d ) );
void    read_auth args( ( struct descriptor_data *d ) );
void    start_auth args( ( struct descriptor_data *d ) );
void    save_sysdata args( ( SYSTEM_DATA sys ) );

void    subtract_times args( ( struct timeval *etime, struct timeval *start_time ) );

/*void	mstats args( ( char *s ) );*/

/*
 * Global variables.
 */
bool                verbose_log;
DESCRIPTOR_DATA *   first_descriptor;	/* First descriptor		*/
DESCRIPTOR_DATA *   last_descriptor;	/* Last descriptor		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
int		    num_descriptors;
bool		    mud_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
time_t              boot_time;
HOUR_MIN_SEC  	    set_boot_time_struct;
HOUR_MIN_SEC *      set_boot_time;
struct tm *         new_boot_time;
struct tm           new_boot_struct;
char		    str_boot_time[MAX_INPUT_LENGTH];
char		    lastplayercmd[MAX_INPUT_LENGTH*2];
time_t		    current_time;	/* Time of this pulse		*/
#ifndef MUD_LISTENER
int                 port;               /* Port number to be used       */
int		    control;		/* Controlling descriptor	*/
#endif
int                 controls;           /* Unix socket                  */
int		    newdesc;		/* New descriptor		*/
fd_set		    in_set;		/* Set of desc's for reading	*/
fd_set		    out_set;		/* Set of desc's for writing	*/
fd_set		    exc_set;		/* Set of desc's with errors	*/
int 		    maxdesc;

STATS_DATA          stats;

/*
 * External variables
 */
extern char * help_greeting;
extern const char * con_table[];

/*
 * OS-dependent local functions.
 */
void	game_loop		args( ( void ) );
int	init_socket		args( ( int listen_port ) );
int	init_unix_socket	args( ( const char *path ) );
void	new_descriptor		args( ( int new_desc ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );

#ifdef USE_BOA
void	boa_startup		args( ( void ) );
void	boa_shutdown		args( ( void ) );
void	boa_loop		args( ( void ) );
#endif

void	free_mud_data		args( ( void ) );

/*
 * Other local functions (OS-independent).
 */
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
                                        bool fConn ) );
sh_int	check_playing		args( ( DESCRIPTOR_DATA *d, char *name, bool kick ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	flush_buffer		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void	free_desc		args( ( DESCRIPTOR_DATA *d ) );
void	display_prompt		args( ( DESCRIPTOR_DATA *d ) );
int	make_color_sequence	args( ( const char *col, char *buf,
                                        DESCRIPTOR_DATA *d ) );
void	set_pager_input		args( ( DESCRIPTOR_DATA *d,
                                        char *argument ) );
bool	pager_output		args( ( DESCRIPTOR_DATA *d ) );

void	mail_count		args( ( CHAR_DATA *ch ) );
void	process_rent		args( ( CHAR_DATA *ch ) );

void	open_mud_log		args( ( void ) );
void	last_log		args( ( char *name, DESCRIPTOR_DATA *d ) );

#if defined(USE_DB) || defined(START_DB)
void db_start(void);
void db_stop(void);
#endif

#ifdef I3
void I3_char_login( CHAR_DATA *ch );
#endif

#ifdef USE_ASPELL
AspellConfig * spell_config;
#endif

DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_return);
DECLARE_DO_FUN(do_copyover);
DECLARE_DO_FUN(do_asave);

void init_stats(void)
{
    FILE *fp;
    char *line;

    stats.bytes_in              = 0;
    stats.bytes_out             = 0;
    stats.boot_bytes_in         = 0;
    stats.boot_bytes_out        = 0;
    stats.comp_bytes_in         = 0;
    stats.comp_bytes_out        = 0;
    stats.comp_boot_bytes_in    = 0;
    stats.comp_boot_bytes_out   = 0;

    if (!(fp=fopen(SYSTEM_DIR "mud.stats", "r")))
    {
        fprintf(stderr, "Unable to open stats file.\n");
        return;
    }

    line = fread_line(fp);

    fclose(fp);

    sscanf( line, "%ld %ld %ld %ld",
            &stats.bytes_in, &stats.bytes_out,
            &stats.comp_bytes_in, &stats.comp_bytes_out );

}

void save_stats(void)
{
    FILE *fp;

    if (!(fp=fopen(SYSTEM_DIR "mud.stats", "w")))
    {
        fprintf(stderr, "Unable to open stats file.\n");
        return;
    }

    fprintf(fp, "%ld %ld %ld %ld\n",
            stats.bytes_in, stats.bytes_out,
            stats.comp_bytes_in, stats.comp_bytes_out );

    fclose(fp);
}

#ifdef STRING_HASH_SEMAPHORE
extern sem_t string_hash_semaphore[1024];
#endif

int main( int argc, char **argv )
{
    struct timeval now_time;
    struct timeval db_start_time;
    struct timeval db_finish_time;
    bool fCopyOver = !TRUE;
#ifdef STRING_HASH_SEMAPHORE
    int x;

    for (x=0;x<1024;x++)
        if (sem_init(&string_hash_semaphore[x], 0, 1) == -1)
        {
            perror("sem_init");
            exit(1);
        }
#endif

#if defined(USE_DB) || defined(START_DB)
    db_start();
#endif

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

#ifdef USE_ASPELL
    spell_config = new_aspell_config();
    aspell_config_replace(spell_config, "lang", "en_US");
#endif

    num_descriptors		= 0;
    first_descriptor		= NULL;
    last_descriptor		= NULL;
    sysdata.NO_NAME_RESOLVING	= TRUE;
    sysdata.WAIT_FOR_AUTH	= TRUE;
    sysdata.DENY_NEW_PLAYERS	= TRUE;

    init_stats();
    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    /*  gettimeofday( &boot_time, NULL);   okay, so it's kludgy, sue me :) */
    boot_time = time(0);         /*  <-- I think this is what you wanted */
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Init boot time.
     */
    set_boot_time = &set_boot_time_struct;
    /*  set_boot_time->hour   = 6;
     set_boot_time->min    = 0;
     set_boot_time->sec    = 0;*/
    set_boot_time->manual = 0;

    new_boot_time = update_time(localtime(&current_time));
    /* Copies *new_boot_time to new_boot_struct, and then points
     new_boot_time to new_boot_struct again. -- Alty */
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;
    new_boot_time->tm_mday += 1;
    if(new_boot_time->tm_hour > 12)
        new_boot_time->tm_mday += 1;
    new_boot_time->tm_sec = 0;
    new_boot_time->tm_min = 0;
    new_boot_time->tm_hour = 6;

    /* Update new_boot_time (due to day increment) */
    new_boot_time = update_time(new_boot_time);
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;

    verbose_log = 0;
    if ( argc > 1 && !str_cmp( argv[1], "-v" ) )
    {
        verbose_log = 1;
        fprintf( stderr, "Logging to stderr...\n" );
    }
    open_mud_log();

    /* Set reboot time string for do_time */
    get_reboot_string();

#ifndef MUD_LISTENER
    /*
     * Get the port number.
     */
    port = 4000;
    if ( argc > 1+verbose_log )
    {
        if ( !is_number( argv[1+verbose_log] ) )
        {
            fprintf( stderr, "Usage: %s [-v] [port #]\n", argv[0] );
            exit( 1 );
        }
        else if ( ( port = atoi( argv[1+verbose_log] ) ) <= 1024 )
        {
            fprintf( stderr, "Port number must be above 1024.\n" );
            exit( 1 );
        }

        if (argv[2+verbose_log] && argv[2+verbose_log][0])
        {
	    int x = 3 + verbose_log;
            fCopyOver = TRUE;
            control = atoi(argv[x++]);
            controls = atoi(argv[x++]);
#ifdef I3
            I3_socket = atoi(argv[x++]);
#endif
#ifdef IRC
            irc_socket = atoi(argv[x++]);
#endif
        }
        else
            fCopyOver = FALSE;
    }
#else
    /*
     * Get the listening descriptor.
     */
    if ( argc > 1+verbose_log )
    {
        if (argv[1+verbose_log] && argv[1+verbose_log][0])
        {
            fCopyOver = TRUE;
            controls = atoi(argv[1+verbose_log]);
        }
        else
            fCopyOver = FALSE;
    }
#endif

    /*
     * Run the game.
     */
    log_string_plus("Booting Database", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);

    gettimeofday( &db_start_time, NULL );
    boot_db(fCopyOver);
    gettimeofday( &db_finish_time, NULL );

    subtract_times(&db_finish_time, &db_start_time);
    boot_log("boot_db finished in %ld.%06ld seconds",
             db_finish_time.tv_sec, db_finish_time.tv_usec);

    log_string_plus("Initializing socket", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
    if (!fCopyOver) /* We have already the port if copyover'ed */
    {
#ifndef MUD_LISTENER
        control  = init_socket( port );
#else
        controls = init_unix_socket( "/tmp/bdotdlistener" );
#endif
    }

#ifdef IMC
    imc_startup( FALSE );
#endif

#ifdef I3
#ifndef MUD_LISTENER
    I3_main(FALSE, port, fCopyOver);
#else
    I3_main(FALSE, 4000, fCopyOver);
#endif
#endif

#ifdef IRC
    irc_startup(fCopyOver);
#endif

    if (fCopyOver)
    {
        boot_log("Running copyover_recover.");
        copyover_recover();
    }

#ifdef USE_BOA
    boa_startup();
#endif

#ifndef MUD_LISTENER
    sprintf( log_buf, "%s ready on port %d.", MUD_NAME, port );
#else
    sprintf( log_buf, "%s ready.", MUD_NAME );
#endif
    log_string_plus( log_buf, LOG_NORMAL, LEVEL_LOG_CSET, SEV_CRIT );
    game_loop( );

#ifdef IMC
    imc_shutdown( FALSE );
#endif

#ifdef I3
    I3_shutdown(0);
#endif

#ifdef IRC
    irc_shutdown();
#endif

#ifdef USE_BOA
    boa_shutdown();
#endif

#ifdef USE_ASPELL
    delete_aspell_config(spell_config);
#endif

#if defined(USE_DB) || defined(START_DB)
    db_stop();
#endif

#ifndef MUD_LISTENER
    close( control  );
#else
    close( controls );
#endif

    /*
     * That's all, folks.
     */
    log_string_plus( "Normal termination of game.", LOG_NORMAL, LEVEL_LOG_CSET, SEV_CRIT );
    /*    mstats("DOTDII");*/

    save_stats();

#ifdef MUD_DEBUG
    free_mud_data();
#endif

    exit( 0 );
    return 0;
}


#ifndef MUD_LISTEENER
int init_socket( int listen_port )
{
    char hostname[64];
    struct sockaddr_in	 sa;
    struct hostent	*hp;
    struct servent	*sp;
    int x = 1;
    int fd;

    gethostname(hostname, sizeof(hostname));


    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        perror( "Init_socket: socket" );
        exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
                     (void *) &x, sizeof(x) ) < 0 )
    {
        perror( "Init_socket: SO_REUSEADDR" );
        close( fd );
        exit( 1 );
    }

    x = 8192;
    if ( setsockopt( fd, SOL_SOCKET, SO_SNDBUF,
                     (void *) &x, sizeof(x) ) < 0 )
    {
        perror( "Init_socket: SO_SNDBUF" );
        close( fd );
        exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_RCVBUF,
                     (void *) &x, sizeof(x) ) < 0 )
    {
        perror( "Init_socket: SO_RCVBUF" );
        close( fd );
        exit( 1 );
    }
    x = 1;

    /*    if ( setsockopt( fd, SOL_TCP, TCP_NODELAY,
     (void *) &x, sizeof(x) ) < 0 )
     {
     perror( "Init_socket: TCP_NODELAY" );
     close( fd );
     exit( 1 );
     }*/

#if defined(SO_DONTLINGER) && !defined(SYSV) && !defined(SOLARIS)
    {
        struct	linger	ld;

        ld.l_onoff  = 1;
        ld.l_linger = 1000;

        if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
                         (void *) &ld, sizeof(ld) ) < 0 )
        {
            perror( "Init_socket: SO_DONTLINGER" );
            close( fd );
            exit( 1 );
        }
    }
#endif

    hp = gethostbyname( hostname );
    sp = getservbyname( "service", "mud" );
    memset(&sa, '\0', sizeof(sa));
    sa.sin_family	= AF_INET; /* hp->h_addrtype; */
    sa.sin_port		= htons( listen_port );
    sa.sin_addr.s_addr	= htonl(INADDR_ANY);

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) == -1 )
    {
        perror( "Init_socket: bind" );
        close( fd );
        exit( 1 );
    }

    if ( listen( fd, 50 ) < 0 )
    {
        perror( "Init_socket: listen" );
        close( fd );
        exit( 1 );
    }

    log_printf_plus(LOG_COMM, LEVEL_LOG_CSET, SEV_NOTICE, "Listening on %s, fd %d", hostname, fd);

    return fd;
}
#endif

int init_unix_socket( const char *sockfile )
{
    struct sockaddr_un sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_UNIX, SOCK_STREAM, 0 ) ) < 0 )
    {
        perror( "init_unix_socket: socket" );
        exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
                     (void *) &x, sizeof(x) ) < 0 )
    {
        perror( "init_unix_socket: SO_REUSEADDR" );
        close( fd );
        exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV) && !defined(SOLARIS)
    {
        struct	linger	ld;

        ld.l_onoff  = 1;
        ld.l_linger = 1000;

        if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
                         (void *) &ld, sizeof(ld) ) < 0 )
        {
            perror( "init_unix_socket: SO_DONTLINGER" );
            close( fd );
            exit( 1 );
        }
    }
#endif

    memset(&sa, '\0', sizeof(sa));
    sa.sun_family	= AF_UNIX;
    strcpy(sa.sun_path, sockfile);

    if ( unlink( sockfile ) == -1 && errno != ENOENT )
    {
        perror( "init_unix_socket: unlink" );
        close( fd );
        exit( 1 );
    }

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) == -1 )
    {
        perror( "init_unix_socket: bind" );
        close( fd );
        exit( 1 );
    }

    if ( listen( fd, 5 ) < 0 )
    {
        perror( "init_unix_socket: listen" );
        close( fd );
        exit( 1 );
    }

    if ( chmod(sockfile, S_IRUSR|S_IWUSR|S_IXUSR) == -1 )
    {
        perror( "init_unix_socket: chmod" );
        close( fd );
        exit( 1 );
    }

    log_printf_plus(LOG_COMM, LEVEL_LOG_CSET, SEV_NOTICE, "Listening on %s, fd %d", sockfile, fd);

    return fd;
}

/*
static void SegVio()
{
    exit(0);
}
*/

/*
 * LAG alarm!							-Thoric
 */
static void caught_alarm(int sig)
{
    char buf[MAX_STRING_LENGTH];
    bug( "ALARM CLOCK!" );
    strcpy( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\n\r" );
    echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
    if ( newdesc )
    {
        FD_CLR( newdesc, &in_set );
        FD_CLR( newdesc, &out_set );
        log_string_plus( "clearing newdesc", LOG_NORMAL, LEVEL_LOG_CSET, SEV_ALERT );
    }
    game_loop( );
#ifndef MUD_LISTENER
    close( control );
#else
    close( controls );
#endif

#ifdef IRC
    irc_shutdown();
#endif

    log_string_plus( "Normal termination of game.", LOG_NORMAL, LEVEL_LOG_CSET, SEV_CRIT );
    /*    mstats("DOTDII");*/
    exit( 0 );
}

static void sig_shutdown(int sig)
{
    mud_down = TRUE;
}

static void sig_copyover(int sig)
{
    do_copyover(NULL,"");
}

bool check_bad_desc( int desc )
{
    if ( FD_ISSET( desc, &exc_set ) )
    {
        FD_CLR( desc, &in_set );
        FD_CLR( desc, &out_set );
        log_string_plus( "Bad FD caught and disposed.", LOG_NORMAL, LEVEL_LOG_CSET, SEV_CRIT );
        return TRUE;
    }
    return FALSE;
}


#ifndef MUD_LISTENER
void accept_new( int ctrl )
{
    static struct timeval null_time;
    DESCRIPTOR_DATA *d;
    /* int maxdesc; Moved up for use with id.c as extern */

#if defined(MALLOC_DEBUG)
    if ( malloc_verify( ) != 1 )
        abort( );
#endif

    /*
     * Poll all active descriptors.
     */
    FD_ZERO( &in_set  );
    FD_ZERO( &out_set );
    FD_ZERO( &exc_set );
    FD_SET( ctrl, &in_set );
    maxdesc	= ctrl;
    newdesc = 0;
    for ( d = first_descriptor; d; d = d->next )
    {
        maxdesc = UMAX( maxdesc, d->descriptor );
        FD_SET( d->descriptor, &in_set  );
        FD_SET( d->descriptor, &out_set );
        FD_SET( d->descriptor, &exc_set );
        if (d->auth_fd != -1)
        {
            maxdesc = UMAX( maxdesc, d->auth_fd );
            FD_SET(d->auth_fd, &in_set);
            if (IS_SET(d->auth_state, FLAG_WRAUTH))
                FD_SET(d->auth_fd, &out_set);
        }
        if ( d == last_descriptor )
            break;
    }

    if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
    {
        perror( "accept_new: select: poll" );
        exit( 1 );
    }

    if ( FD_ISSET( ctrl, &exc_set ) )
    {
        bug( "Exception raise on controlling descriptor %d", ctrl );
        FD_CLR( ctrl, &in_set );
        FD_CLR( ctrl, &out_set );
    }
    else if ( FD_ISSET( ctrl, &in_set ) )
    {
        newdesc = ctrl;
        new_descriptor( newdesc );
    }
}
#endif

void process_listener_connection(int fd);
void listener_close_socket(DESCRIPTOR_DATA *d);

void game_loop( )
{
    struct timeval	  last_time;
    char cmdline[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    time_t	last_check = 0;

/*    signal( SIGSEGV, sig_copyover );*/
    signal( SIGPIPE, SIG_IGN );
    signal( SIGPROF, SIG_IGN );
    signal( SIGALRM, caught_alarm );
    signal( SIGUSR1, sig_shutdown );
    signal( SIGUSR2, sig_copyover );
    signal( SIGXCPU, sig_shutdown );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !mud_down )
    {
#ifndef MUD_LISTENER
        accept_new( control  );
#else
        process_listener_connection(controls);
#endif

        /*
         * Kick out descriptors with raised exceptions
         * or have been idle, then check for input.
         */
        for ( d = first_descriptor; d; d = d_next )
        {
            if ( d == d->next )
            {
                bug( "descriptor_loop: loop found & fixed" );
                d->next = NULL;
            }
            d_next = d->next;

            d->idle++;	/* make it so a descriptor can idle out */
#ifndef MUD_LISTENER
            if ( FD_ISSET( d->descriptor, &exc_set ) )
            {
                FD_CLR( d->descriptor, &in_set  );
                FD_CLR( d->descriptor, &out_set );
                if ( d->character
                     && ( d->connected == CON_PLAYING
                          ||   d->connected == CON_EDITING ) )
                    save_char_obj( d->character );
                d->outtop	= 0;
                close_socket( d, TRUE );
                continue;
            }
            else
#endif
                if ( (!d->character && d->idle > 2*60*PULSE_PER_SECOND) ||
                     ( d->connected != CON_PLAYING && d->idle > 5*60*PULSE_PER_SECOND) ||
                     ( d->idle > 60*60*PULSE_PER_SECOND && GetMaxLevel(d->character) < LEVEL_SAVIOR ) )
            {
                log_printf_plus(LOG_MONITOR, LEVEL_IMMORTAL, SEV_INFO,
                                "Idle timeout, disconnecting %s",
                                d->character?d->character->name:"(Unknown)");
                write_to_descriptor( d, "Idle timeout... disconnecting.\n\r", 0 );
                d->outtop	= 0;
                close_socket( d, TRUE );
                continue;
            }
            else if ( d->connected == CON_PLAYING &&
                      d->idle > 30*60*PULSE_PER_SECOND &&
                      !IS_SET(d->character->act, PLR_AFK) )
            {
                SET_BIT(d->character->act, PLR_AFK);
                write_to_descriptor( d, "You have been idle for 30 minutes.\n\r", 0 );
                continue;
            }
            else
            {
                d->fcommand	= FALSE;
#ifndef MUD_LISTENER
                if ( FD_ISSET( d->descriptor, &in_set ) )
                {
                    d->idle = 0;
                    if ( d->character )
                        d->character->timer = 0;
                    if ( !read_from_descriptor( d ) )
                    {
                        FD_CLR( d->descriptor, &out_set );
                        if ( d->character
                             && ( d->connected == CON_PLAYING
                                  ||   d->connected == CON_EDITING ) )
                            save_char_obj( d->character );
                        d->outtop	= 0;
                        close_socket( d, FALSE );
                        continue;
                    }
                }
                /* IDENT authentication */
                if ( ( d->auth_fd == -1 ) && ( d->atimes < 20 )
                     && !str_cmp( d->user, "unknown" ) )
                    start_auth( d );

                if ( d->auth_fd != -1)
                {
                    if ( FD_ISSET( d->auth_fd, &in_set ) )
                    {
                        read_auth( d );
                        /* if ( !d->auth_state )
                         check_ban( d );*/
                    }
                    else
                        if ( FD_ISSET( d->auth_fd, &out_set )
                             && IS_SET( d->auth_state, FLAG_WRAUTH) )
                        {
                            send_auth( d );
                            /* if ( !d->auth_state )
                             check_ban( d );*/
                        }
                }
#endif
                if ( d->character && d->character->wait > 0 )
                {
                    --d->character->wait;
                    continue;
                }

                read_from_buffer( d );
                if ( d->incomm[0] != '\0' )
                {
                    d->fcommand	= TRUE;
                    stop_idling( d->character );

                    strcpy( cmdline, d->incomm );
                    d->incomm[0] = '\0';

                    if ( d->character )
                        set_cur_char( d->character );

                    if ( d->pagepoint )
                        set_pager_input(d, cmdline);
                    else
                        switch( d->connected )
                        {
                        default:
                            nanny( d, cmdline );
                            break;
                        case CON_PLAYING:
                            d->character->cmd_recurse = 0;
                            interpret( d->character, cmdline );
                            break;
                        case CON_EDITING:
                            edit_buffer( d->character, cmdline );
                            break;
                        }
                }
            }
            if ( d == last_descriptor )
                break;
        }

        /* kick IMC */
#ifdef IMC
        imc_loop();
#endif

#ifdef I3
        I3_loop();
#endif

#ifdef IRC
	irc_loop();
#endif

#ifdef USE_BOA
        boa_loop();
#endif

        /*
         * Autonomous game motion.
         */
        update_handler( );

        /*
         * Check REQUESTS pipe
         */
        check_requests( );

        /*
         * Output.
         */
        for ( d = first_descriptor; d; d = d_next )
        {
            d_next = d->next;

            if ( ( d->fcommand || d->outtop > 0 || d->pagetop > 0 )
#ifndef MUD_LISTENER
		&& FD_ISSET(d->descriptor, &out_set)
#endif
	       )
            {
                if ( d->pagepoint )
                {
                    if ( !pager_output(d) )
                    {
                        if ( d->character &&
                             ( d->connected == CON_PLAYING ||
                               d->connected == CON_EDITING ) )
                            save_char_obj( d->character );
                        d->outtop = 0;
                        close_socket(d, FALSE);
                    }
                }
                else if ( d->character &&
                          IS_SET(d->character->act,PLR_AFK) &&
                          IS_SET(d->character->act2,PLR2_AFK_BUFFER) )
                {
                    write_to_descriptor(d, "Buffering text...\n\r",0);
                }
                else if ( !flush_buffer( d, TRUE ) )
                {
                    if ( d->character &&
                         ( d->connected == CON_PLAYING ||
                           d->connected == CON_EDITING ) )
                        save_char_obj( d->character );
                    d->outtop	= 0;
                    close_socket( d, FALSE );
                }
            }
            if ( d == last_descriptor )
                break;
        }


        /*
         * Synchronize to a clock.
         * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
         * Careful here of signed versus unsigned arithmetic.
         */
        {
            struct timeval now_time;
            long secDelta;
            long usecDelta;

            gettimeofday( &now_time, NULL );
            usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
                + 1000000 / PULSE_PER_SECOND;
            secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
            while ( usecDelta < 0 )
            {
                usecDelta += 1000000;
                secDelta  -= 1;
            }

            while ( usecDelta >= 1000000 )
            {
                usecDelta -= 1000000;
                secDelta  += 1;
            }

            if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
            {
                struct timeval stall_time;

                stall_time.tv_usec = usecDelta;
                stall_time.tv_sec  = secDelta;
                if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
                {
                    perror( "game_loop: select: stall" );
                    exit( 1 );
                }
            }
        }

        gettimeofday( &last_time, NULL );
        current_time = (time_t) last_time.tv_sec;

#ifndef __CYGWIN__
        if ( last_check+15 < current_time )
        {
            CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
                        DESCRIPTOR_DATA);
            CHECK_LINKS(first_char, last_char, next, prev, CHAR_DATA);
            CHECK_LINKS(first_object, last_object, next, prev, OBJ_DATA);
            last_check = current_time;
        }
#endif

        if ( current_time%(60*30) == 5 )
        {
            /* 5 seconds after every half hour, check uptime */
            if ( sysdata.longest_uptime < (current_time - boot_time) )
            {
                sysdata.longest_uptime = current_time - boot_time;
                save_sysdata(sysdata);
            }
        }

        if ( current_time%3600 == 0 )
            open_mud_log();

#ifdef GC_DEBUG
        CHECK_LEAKS();
#endif

#ifdef MUD_DEBUG
	hash_check_sanity();
#endif
    }
    return;
}

#ifndef MUD_LISTENER
void init_descriptor( DESCRIPTOR_DATA *dnew, int desc)
#else
void init_descriptor( DESCRIPTOR_DATA *dnew, int uid)
#endif
{
    dnew->next		= NULL;
#ifndef MUD_LISTENER
    dnew->descriptor	= desc;
#else
    dnew->uid		= uid;
#endif
    dnew->connected	= CON_GET_NAME;
    dnew->outsize	= 5000;
    dnew->idle		= 0;
    dnew->lines		= 0;
    dnew->scrlen	= 24;
    dnew->user		= STRALLOC("unknown");
    dnew->auth_fd	= -1;
    dnew->auth_inc	= 0;
    dnew->auth_state	= 0;
    dnew->newstate	= 0;
    dnew->prevcolor	= 0x07;
    dnew->conn_id	= sysdata.total_logins+1;

    CREATE( dnew->outbuf, char, dnew->outsize );
}


#ifndef MUD_LISTENER
void new_descriptor( int new_desc )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    unsigned int size;

    set_alarm( 20 );
    size = sizeof(sock);
    if ( check_bad_desc( new_desc ) )
    {
        set_alarm( 0 );
        return;
    }
    set_alarm( 20 );
    if ( ( desc = accept( new_desc, (struct sockaddr *) &sock, &size) ) < 0 )
    {
        perror( "New_descriptor: accept" );
        sprintf(log_buf, "New_descriptor: accept returned <0");
        log_string_plus( log_buf, LOG_COMM, sysdata.log_level, SEV_ERR );
        log_string_plus( log_buf, LOG_BUG, sysdata.log_level, SEV_DEBUG );
        set_alarm( 0 );
        return;
    }
    if ( check_bad_desc( new_desc ) )
    {
        set_alarm( 0 );
        return;
    }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    set_alarm( 20 );
    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
        perror( "New_descriptor: fcntl: FNDELAY" );
        set_alarm( 0 );
        return;
    }
    if ( check_bad_desc( new_desc ) )
        return;
    CREATE( dnew, DESCRIPTOR_DATA, 1 );

    init_descriptor(dnew, desc );
    dnew->port = ntohs(sock.sin_port);

#ifdef DESCRIPTOR_HAS_SIN_ADDR
    memcpy(&dnew->sin_addr, &sock.sin_addr, sizeof(dnew->sin_addr));
#endif

    strcpy( buf, inet_ntoa( sock.sin_addr ) );

    if ( sysdata.NO_NAME_RESOLVING )
    {
        dnew->host = STRALLOC( buf );
        sprintf( log_buf, "Sock.sinaddr: %s, port %d.",
                 dnew->host, dnew->port );
    }
    else
    {
        from = gethostbyaddr( (char *) &sock.sin_addr,
                              sizeof(sock.sin_addr), AF_INET );
        dnew->host = STRALLOC( (char *)( from ? from->h_name : buf) );
        sprintf( log_buf, "Sock.sinaddr: %s(%s), port %d.",
                 buf, dnew->host, dnew->port );
    }
    log_string_plus( log_buf, LOG_COMM, sysdata.log_level, SEV_NOTICE );

    for ( pban = first_ban; pban; pban = pban->next )
    {
        /* This used to use str_suffix, but in order to do bans by the
         first part of the ip, ie "ban 207.136.25" str_prefix must
         be used. -- Narn
         */
        if ( (!str_prefix( pban->name, dnew->host ) ||
              !str_suffix( pban->name, dnew->host )) &&
             pban->level >= LEVEL_SUPREME )
        {
            write_to_descriptor( dnew,
                                 "Your site has been banned from this MUD.\n\r"
                                 "For more information, email dotd@dotd.com\n\r", 0 );
            free_desc( dnew );
            set_alarm( 0 );
            return;
        }
    }

    /*
     * Init descriptor data.
     */

    if ( !last_descriptor && first_descriptor )
    {
        DESCRIPTOR_DATA *d;

        bug( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );
        for ( d = first_descriptor; d; d = d->next )
            if ( !d->next )
                last_descriptor = d;
    }

    LINK( dnew, first_descriptor, last_descriptor, next, prev );

#ifdef COMPRESS
    write_to_buffer(dnew, eor_on_str, 0);
    write_to_buffer(dnew, compress2_on_str, 0);
    write_to_buffer(dnew, compress_on_str, 0);
#endif

    write_to_buffer(dnew, eor_on_str, 0);
    write_to_buffer(dnew, mxp_on_str, 0);

    write_to_buffer(dnew, eor_on_str, 0);
    write_to_buffer(dnew, msp_on_str, 0);

    /*
     * Send the greeting.
     */
    {
        char filename[64];
        FILE *fp;
        int c;
        int num = 0;

        if ( help_greeting && *help_greeting )
        {
            if (help_greeting[0] == '.' )
                write_to_buffer( dnew, help_greeting+1, 0 );
            else
                write_to_buffer( dnew, help_greeting  , 0 );
        }

        sprintf(filename, "../system/login%d", number_range(1,6));
        if ( (fp = fopen( filename, "r" )) != NULL )
        {
            while ( !feof(fp) )
            {
                while ((buf[num]=fgetc(fp)) != EOF
                       &&      buf[num] != '\n'
                       &&      buf[num] != '\r'
                       &&      num < (MAX_STRING_LENGTH-2))
                    num++;
                c = fgetc(fp);
                if ( (c != '\n' && c != '\r') || c == buf[num] )
                    ungetc(c, fp);
                buf[num++] = '\n';
                buf[num++] = '\r';
                buf[num  ] = '\0';
                write_to_buffer( dnew, buf, num );
                num = 0;
            }
            FCLOSE(fp);
        }
    }

    write_to_buffer( dnew, "What is your name? ", 0);

    start_auth( dnew ); /* Start username authorization */

    if ( ++num_descriptors > sysdata.maxplayers )
        sysdata.maxplayers = num_descriptors;
    sysdata.total_logins++;
    if ( sysdata.maxplayers > sysdata.alltimemax )
    {
        if ( sysdata.time_of_max )
            DISPOSE(sysdata.time_of_max);
        sprintf(buf, "%24.24s", ctime(&current_time));
        sysdata.time_of_max = str_dup(buf);
        sysdata.alltimemax = sysdata.maxplayers;
        sprintf( log_buf, "Broke all-time maximum player record: %d", sysdata.alltimemax );
        log_string_plus( log_buf, LOG_MONITOR, LEVEL_IMMORTAL, SEV_NOTICE );
    }
    if ( sysdata.longest_uptime < (current_time - boot_time) )
        sysdata.longest_uptime = current_time - boot_time;
    save_sysdata( sysdata );
    set_alarm(0);
    return;
}
#endif

void free_desc( DESCRIPTOR_DATA *d )
{
#ifndef MUD_LISTENER
    close( d->descriptor );
#endif
    STRFREE( d->host );
    DISPOSE( d->outbuf );
    STRFREE( d->user );    /* identd */
    if ( d->pagebuf )
        DISPOSE( d->pagebuf );

#ifndef MUD_LISTENER
#ifdef COMPRESS
    if (d->compressing)
        stop_compression( d, d->compressing );
#endif
#endif

    DISPOSE( d );
    --num_descriptors;
    return;
}

#ifdef USE_CRBS
void do_clogoff(CHAR_DATA *ch, char *argument);
void do_clogon(CHAR_DATA *ch, char *argument);
#endif

void close_socket( DESCRIPTOR_DATA *dclose, bool force )
{
    CHAR_DATA *ch;
    DESCRIPTOR_DATA *d;
    bool DoNotUnlink = FALSE;

    /* flush outbuf */
    if ( !force && dclose->outtop > 0 )
        flush_buffer( dclose, FALSE );

    /* say bye to whoever's snooping this descriptor */
    if ( dclose->snoop_by )
        write_to_buffer( dclose->snoop_by,
                         "Your victim has left the game.\n\r", 0 );

    /* stop snooping everyone else */
    for ( d = first_descriptor; d; d = d->next )
        if ( d->snoop_by == dclose )
            d->snoop_by = NULL;

#ifdef MUD_LISTENER
    listener_close_socket(dclose);
#endif

    /* Check for switched people who go link-dead. -- Altrag */
    if ( dclose->original )
    {
        if ( ( ch = dclose->character ) != NULL )
            do_return(ch, "");
        else
        {
            bug( "Close_socket: dclose->original without character %s",
                 (dclose->original->name ? dclose->original->name : "unknown") );
            dclose->character = dclose->original;
            dclose->original = NULL;
        }
    }

    ch = dclose->character;

    /* sanity check :( */
    if ( !dclose->prev && dclose != first_descriptor )
    {
        DESCRIPTOR_DATA *dp, *dn;
        bug( "Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!",
             ch ? ch->name : d->host, dclose, first_descriptor );
        dp = NULL;
        for ( d = first_descriptor; d; d = dn )
        {
            dn = d->next;
            if ( d == dclose )
            {
                bug( "Close_socket: %s desc:%p found, prev should be:%p, fixing.",
                     ch ? ch->name : d->host, dclose, dp );
                dclose->prev = dp;
                break;
            }
            dp = d;
        }
        if ( !dclose->prev )
        {
            bug( "Close_socket: %s desc:%p could not be found!.",
                 ch ? ch->name : dclose->host, dclose );
            DoNotUnlink = TRUE;
        }
    }
    if ( !dclose->next && dclose != last_descriptor )
    {
        DESCRIPTOR_DATA *dp, *dn;
        bug( "Close_socket: %s desc:%p != last_desc:%p and desc->next = NULL!",
             ch ? ch->name : d->host, dclose, last_descriptor );
        dn = NULL;
        for ( d = last_descriptor; d; d = dp )
        {
            dp = d->prev;
            if ( d == dclose )
            {
                bug( "Close_socket: %s desc:%p found, next should be:%p, fixing.",
                     ch ? ch->name : d->host, dclose, dn );
                dclose->next = dn;
                break;
            }
            dn = d;
        }
        if ( !dclose->next )
        {
            bug( "Close_socket: %s desc:%p could not be found!.",
                 ch ? ch->name : dclose->host, dclose );
            DoNotUnlink = TRUE;
        }
    }

    if ( dclose->character )
    {
#ifdef USE_CRBS
        do_clogoff(ch, NULL); /* CRBS logoff */
#endif

#ifdef IRC
	irc_logoff(ch);
#endif
        sprintf( log_buf, "Closing link to %s (played for %s).",
                 ch->name, sec_to_hms_short(current_time-ch->logon) );
        log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, GetMaxLevel(ch) ), SEV_NOTICE );

        if ( dclose->connected == CON_PLAYING
             ||   dclose->connected == CON_EDITING )
        {
            act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
            ch->desc = NULL;
        }
        else
        {
            /* clear descriptor pointer to get rid of bug message in log */
            dclose->character->desc = NULL;
            free_char( dclose->character );
        }
    }

    if ( !DoNotUnlink )
    {
        /* make sure loop doesn't get messed up */
        if ( d_next == dclose )
            d_next = d_next->next;
        UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
    }

#ifndef MUD_LISTENER
#ifdef COMPRESS
    if ( dclose->compressing )
        stop_compression(dclose, dclose->compressing);
#endif

    if ( dclose->descriptor == maxdesc )
        --maxdesc;
    if ( dclose->auth_fd != -1 )
        close( dclose->auth_fd );
#endif

    free_desc( dclose );
    return;
}

#ifndef MUD_LISTENER
bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
        return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= (signed int)sizeof(d->inbuf) - 10 )
    {
        sprintf( log_buf, "%s input overflow!", d->host );
        log_string_plus( log_buf, LOG_MONITOR, LEVEL_LOG_CSET, SEV_NOTICE );
        write_to_descriptor( d, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
        return FALSE;
    }

    for ( ; ; )
    {
        int nRead;

        nRead = read( d->descriptor, d->inbuf + iStart,
                      sizeof(d->inbuf) - 10 - iStart );
        if ( nRead > 0 )
        {
            iStart += nRead;

            stats.boot_bytes_in      += nRead;
            stats.bytes_in           += nRead;
            stats.comp_boot_bytes_in += nRead;
            stats.comp_bytes_in      += nRead;

            if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
                break;
        }
        else if ( nRead == 0 )
        {
            log_string_plus( "EOF encountered on read.", LOG_COMM, sysdata.log_level, SEV_NOTICE );
            return FALSE;
        }
        else if ( errno == EWOULDBLOCK )
            break;
        else
        {
            perror( "Read_from_descriptor" );
            return FALSE;
        }
    }

    d->inbuf[iStart] = '\0';
    return TRUE;
}
#endif


/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;
    int iac = 0;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
        return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i<MAX_INBUF_SIZE;
          i++ )
    {
        if ( d->inbuf[i] == '\0' )
            return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
        if ( k >= MAX_INBUF_SIZE-1 )
        {
            write_to_descriptor( d, "Line too long.\n\r", 0 );

            /* skip the rest of the line */
            /*
             for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
             {
             if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
             break;
             }
             */
            d->inbuf[i]   = '\n';
            d->inbuf[i+1] = '\0';
            break;
        }


        if ( d->inbuf[i] == (signed char)IAC )
            iac=1;
        else if ( iac==1 && (d->inbuf[i] == (signed char)DO || d->inbuf[i] == (signed char)DONT) )
            iac=2;
        else if ( iac==2 )
        {
            iac = 0;
            if ( d->inbuf[i] == (signed char)TELOPT_MXP )
            {
                if ( d->inbuf[i-1] == (signed char)DO )
                {
#ifdef MXP
#ifndef MUD_LISTENER
                    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d supports MXP, enabling", d->descriptor);
#else
                    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d supports MXP, enabling", d->uid);
#endif
                    d->mxp_detected = TRUE;
                    send_mxp_stylesheet(d);
#else
#ifndef MUD_LISTENER
                    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d supports MXP, but we don't", d->descriptor);
#else
                    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d supports MXP, but we don't", d->uid);
#endif
#endif
                }
            }
            else if ( d->inbuf[i] == (signed char)TELOPT_MSP )
            {
                if ( d->inbuf[i-1] == (signed char)DO )
                {
#ifdef MSP
#ifndef MUD_LISTENER
                    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d supports MSP, enabling", d->descriptor);
#else
                    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d supports MSP, enabling", d->uid);
#endif
                    d->msp_detected = TRUE;
#else
#ifndef MUD_LISTENER
                    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d supports MSP, but we don't", d->descriptor);
#else
                    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d supports MSP, but we don't", d->uid);
#endif
#endif
                }
            }
#ifndef MUD_LISTENER
#ifdef COMPRESS
            else if ( d->inbuf[i] == (signed char)TELOPT_COMPRESS )
            {
                if ( d->inbuf[i-1] == (signed char)DO && !d->compressing )
                    start_compression(d, TELOPT_COMPRESS);
                else if ( d->inbuf[i-1] == (signed char)DONT && d->compressing==TELOPT_COMPRESS )
                    stop_compression(d, TELOPT_COMPRESS);
            }
            else if ( d->inbuf[i] == (signed char)TELOPT_COMPRESS2 )
            {
                if ( d->inbuf[i-1] == (signed char)DO && !d->compressing )
                    start_compression(d, TELOPT_COMPRESS2);
                else if ( d->inbuf[i-1] == (signed char)DONT && d->compressing==TELOPT_COMPRESS2 )
                    stop_compression(d, TELOPT_COMPRESS2);
            }
#endif
#endif
        }
        else
            if ( d->inbuf[i] == '\b' && k > 0 )
                --k;
            else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
                d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
        d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' )
    {
        if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
        {
            d->repeat = 0;
        }
        else
        {
#if 0
            if ( ++d->repeat >= 100 )
            {
                /*		sprintf( log_buf, "%s input spamming!", d->host );
                 log_string( log_buf );
                 */
                write_to_descriptor( d, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
                strcpy( d->incomm, "quit yes" );
            }
#endif
        }
    }

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
        strcpy( d->incomm, d->inlast );
    else
        strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
        i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
        ;
    return;
}



/*
 * Low level output function.
 */
bool flush_buffer( DESCRIPTOR_DATA *d, bool fPrompt )
{
    char buf[MAX_STRING_LENGTH];

    /*
     * If buffer has more than 4K inside, spit out 4K at a time   -Thoric
     */
    if ( !mud_down && d->outtop > 4096 )
    {
        memcpy( buf, d->outbuf, 4096 );
        memmove( d->outbuf, d->outbuf + 4096, d->outtop - 4096 );
        d->outtop -= 4096;
        if ( d->snoop_by )
        {
            buf[4096] = '\0';
            if ( d->character && d->character->name )
            {
                char snoopbuf[4096];

                if (d->original && d->original->name)
                    sprintf( snoopbuf, "%s (%s)", d->character->name, d->original->name );
                else
                    sprintf( snoopbuf, "%s", d->character->name);
                write_to_buffer( d->snoop_by, snoopbuf, 0);
            }
            write_to_buffer( d->snoop_by, "% ", 2 );
            write_to_buffer( d->snoop_by, buf, 0 );
        }
        if ( !write_to_descriptor( d, buf, 4096 ) )
        {
            d->outtop = 0;
            return FALSE;
        }
        return TRUE;
    }

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by && d->outtop != 0 )
    {
        /* without check, 'force mortal quit' while snooped caused crash, -h */
        if ( d->character && d->character->name )
        {
            char snoopbuf[MAX_INPUT_LENGTH];

            /* Show original snooped names. -- Altrag */
            if ( d->original && d->original->name )
                sprintf( snoopbuf, "%s (%s)", d->character->name, d->original->name );
            else
                sprintf( snoopbuf, "%s", d->character->name);
            write_to_buffer( d->snoop_by, snoopbuf, 0);
        }
        write_to_buffer( d->snoop_by, "% ", 2 );
        write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * Bust a prompt.
     */
    if ( fPrompt && !mud_down && d->connected == CON_PLAYING )
    {
        CHAR_DATA *ch;

        ch = d->original ? d->original : d->character;
        if ( IS_SET(ch->act, PLR_BLANK) )
            write_to_buffer( d, "\n\r", 2 );

        if ( IS_SET(ch->act, PLR_PROMPT) )
            display_prompt(d);
        if ( IS_SET(ch->act, PLR_TELNET_GA) )
            write_to_buffer( d, go_ahead_str, 0 );
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
        return TRUE;

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d, d->outbuf, d->outtop ) )
    {
        d->outtop = 0;
        return FALSE;
    }
    else
    {
        d->outtop = 0;
        return TRUE;
    }
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, unsigned int length )
{
    if ( !d )
    {
        bug( "Write_to_buffer: NULL descriptor" );
        return;
    }

    /*
     * Normally a bug... but can happen if loadup is used.
     */
    if ( !d->outbuf )
        return;

    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
        length = strlen(txt);
    /*
     if ( length != strlen(txt) )
     {
     bug( "Write_to_buffer: length(%d) != strlen(txt)!", length );
     length = strlen(txt);
     }
     */
    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
        d->outbuf[0]	= '\n';
        d->outbuf[1]	= '\r';
        d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
        if (d->outsize > 64000)
        {
            d->outtop = 0;
            bug("Buffer overflow. Closing (%s).", d->character ? d->character->name : "???" );
            close_socket(d, TRUE);
            return;
        }
        d->outsize *= 2;
        RECREATE( d->outbuf, char, d->outsize );
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    d->outbuf[d->outtop] = '\0';
    return;
}


#ifndef MUD_LISTENER
/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
#ifndef COMPRESS
bool write_to_descriptor( DESCRIPTOR_DATA *d, char *txt, int length )
{
    int iStart;
    int nWrite = 0;
    int nBlock;

    if ( length <= 0 )
        length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
        nBlock = UMIN( length - iStart, 4096 );
        if ( ( nWrite = write( d->descriptor, txt + iStart, nBlock ) ) < 0 )
        { perror( "Write_to_descriptor" ); return FALSE; }

        stats.boot_bytes_out      += nWrite;
        stats.bytes_out           += nWrite;
        stats.comp_boot_bytes_out += nWrite;
        stats.comp_bytes_out      += nWrite;
    }

    return TRUE;
}
#else
bool write_to_descriptor( DESCRIPTOR_DATA *d, char *txt, int length )
{
    int     iStart = 0;
    int     nWrite = 0;
    int     nBlock;
    int len;

    if (length <= 0)
        length = strlen(txt);

    /* Check for output compression */

    if (d && d->out_compress)
    {
        d->out_compress->next_in = (unsigned char *)txt;
        d->out_compress->avail_in = length;

        while (d->out_compress->avail_in)
        {
            d->out_compress->avail_out = COMPRESS_BUF_SIZE - (d->out_compress->next_out - d->out_compress_buf);

            if (d->out_compress->avail_out)
            {
                int status = deflate(d->out_compress, Z_SYNC_FLUSH);

                if (status != Z_OK)
                {
                    /* Boom */
                    return FALSE;
                }
            }

            /* Try to write out some data.. */
            len = d->out_compress->next_out - d->out_compress_buf;
            if (len > 0)
            {
                /* we have some data to write */

                for (iStart = 0; iStart < len; iStart += nWrite)
                {
                    nBlock = UMIN (len - iStart, 4096);
                    if ((nWrite = write(d->descriptor, d->out_compress_buf + iStart, nBlock)) < 0)
                    {
/*                        if (errno == EAGAIN ||
                            errno == ENOSR)
                            break; */
                        perror( "Write_to_descriptor: compressed" );
                        return FALSE; /* write error */
                    }

                    if (!nWrite)
                        break;

/*                    stats.boot_bytes_out      += nWrite;*/
/*                    stats.bytes_out           += nWrite;*/
                    stats.comp_boot_bytes_out += nWrite;
                    stats.comp_bytes_out      += nWrite;
                }

                if (!iStart)
                    break; /* Can't write any more */

                /* We wrote "iStart" bytes */
                if (iStart < len)
                    memmove(d->out_compress_buf, d->out_compress_buf+iStart, len - iStart);

                d->out_compress->next_out = d->out_compress_buf + len - iStart;
            }

            /* Loop */
        }

        /* Done. */
        stats.boot_bytes_out += length - d->out_compress->avail_in;
        stats.bytes_out      += length - d->out_compress->avail_in;
        /*        return length - d->out_compress->avail_in; */
        return TRUE;
    }

    for (iStart = 0; iStart < length; iStart += nWrite)
    {
        nBlock = UMIN (length - iStart, 4096);
        if ((nWrite = write(d->descriptor, txt + iStart, nBlock)) < 0)
        {
            perror( "Write_to_descriptor" );
            return FALSE;
        }
        stats.boot_bytes_out      += nWrite;
        stats.bytes_out           += nWrite;
        stats.comp_boot_bytes_out += nWrite;
        stats.comp_bytes_out      += nWrite;
    }

    return TRUE;
}
#endif
#endif

void show_title( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch;

    ch = d->character;

    if ( !IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) )
    {
        if (IS_SET(ch->act, PLR_RIP))
            send_rip_title(ch);
        else
            if (IS_SET(ch->act, PLR_ANSI))
                send_ansi_title(ch);
            else
                send_ascii_title(ch);
    }
    write_to_buffer( d, "Press [ENTER] ", 0 );
}

bool unplayable_class(int iClass)
{
    switch (iClass)
    {
    case CLASS_VAMPIRE:
    case CLASS_ANTIPALADIN:
    case CLASS_ARTIFICER:
    case CLASS_NECROMANCER:
        return TRUE;
        break;
    }

    return FALSE;
}

char *race_choices(void)
{
    int iClass, iRace, iCnt;
    char buf[MAX_STRING_LENGTH];
    static char retbuf[MAX_STRING_LENGTH];

    sprintf(retbuf,"Level Limits:\n\r%-4s %-15s","#","Race");

    for (iClass = 0; iClass < MAX_CLASS; ++iClass)
    {
        if (unplayable_class(iClass))
            continue;
        sprintf(buf, "%-3s", short_pc_class[iClass]);
	strcat(retbuf,buf);
    }
    strcat(retbuf,"\n\r");

    for (iRace = 0, iCnt = 0; iRace < MAX_RACE; ++iRace)
    {
        if (race_table[iRace].is_pc_race)
        {
	    sprintf(buf,"%-3d] %-15s", iCnt, race_table[iRace].race_name);
	    strcat(retbuf,buf);
            for (iClass = 0; iClass < MAX_CLASS; ++iClass)
            {
                if (unplayable_class(iClass))
                    continue;
                sprintf(buf, "%-3d", RacialMax[iRace][iClass]);
		strcat(retbuf,buf);
            }
	    strcat(retbuf,"\n\r");
            ++iCnt;
        }
    }
    return retbuf;
}


int show_race_choice(DESCRIPTOR_DATA *d)
{
    char *buf;

    buf = race_choices();

    write_to_buffer(d, buf, 0);

    return(0);
}

char *class_choices(int chrace)
{
    int i;
    char buf[MAX_STRING_LENGTH];
    static char retbuf[MAX_STRING_LENGTH];
    int tmp;
    int race = 0;

    retbuf[0] = buf[0] = '\0';

    for (i = 0; i < chrace; ++i)
        if (race_table[i].is_pc_race)
            ++race;
    for (i = 0; ch_class_choice[race][i]; i++)
    {
        tmp = 0;
	sprintf(buf, "%-3d] ", i);
        strcat(retbuf, buf);
        while (ch_class_choice[race][i][tmp]!='\0')
        {
	    if (tmp>0)
                strcat(retbuf, "/");
            switch(ch_class_choice[race][i][tmp])
            {
            case 'w':
                strcat(retbuf, "Warrior"); break;
            case 'm':
                strcat(retbuf, "Mage"); break;
            case 't':
                strcat(retbuf, "Thief"); break;
            case 'c':
                strcat(retbuf, "Cleric"); break;
            case 'r':
                strcat(retbuf, "Ranger"); break;
            case 'v':
                strcat(retbuf, "Vampire"); break;
            case 'd':
                strcat(retbuf, "Druid"); break;
            case 'z':
                strcat(retbuf, "Amazon"); break;
            case 'p':
                strcat(retbuf, "Paladin"); break;
            case 'b':
                strcat(retbuf, "Barbarian"); break;
            case 's':
                strcat(retbuf, "Necromancer"); break;
            case 'o':
                strcat(retbuf, "Monk"); break;
            case 'i':
                strcat(retbuf, "Psionist"); break;
            case 'l':
                strcat(retbuf, "Artificer"); break;
            case 'n':
                strcat(retbuf, "Anti-Paladin"); break;
            case 'e':
		strcat(retbuf, "Sorcerer"); break;
	    default:
                log_string_plus("Invalid class value in ch_class_choice", LOG_BUG, LEVEL_LOG_CSET, SEV_ERR);
                break;
            }
            ++tmp;
	}
        strcat(retbuf, "\n\r");
    }
    return retbuf;
}


void show_class_choice(DESCRIPTOR_DATA *d)
{
    char *buf;

    buf = class_choices(d->character->race);

    write_to_buffer(d, buf, 0);
}
/*
 * Deal with sockets that haven't logged in yet.
 */

#define SEPERATOR_LINE "\n\r------------------------------------------------------------\n\r\n\r"

void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int pstats[MAX_STATS];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int iClass;
    int iRace;
    BAN_DATA *pban;
    bool fOld;
    sh_int chk;
    int iCnt, i, j;
    int SaveRace = 0;

    while ( isspace(*argument) )
        argument++;

    ch = d->character;

    if (d->connected != CON_PLAYING || d->connected != CON_EDITING)
        log_printf_plus(LOG_NORMAL, LEVEL_LOG_CSET, SEV_DEBUG, "Nanny: %d/%s  Input: %s",
                   d->connected,
                   con_table[d->connected+99],
                   (d->connected==CON_GET_OLD_PASSWORD ||
                    d->connected==CON_GET_NEW_PASSWORD ||
                    d->connected==CON_CONFIRM_NEW_PASSWORD)?"(not shown)":argument);

    switch ( d->connected )
    {
    case CON_INVALID:
    case CON_ACCEPTED:
    case CON_COPYOVER_RECOVER:
    case CON_GET_INTERFACE:
    case CON_EDITING:
    case CON_PLAYING:
    case CON_WAIT_1:
    case CON_WAIT_2:
    case CON_WAIT_3:

        bug( "Nanny: bad d->connected %d.", d->connected );
        close_socket( d, TRUE );
        return;

    case CON_GET_NAME:
        if ( argument[0] == '\0' )
        {
            close_socket( d, FALSE );
            return;
        }

        argument[0] = UPPER(argument[0]);

        if ( !check_parse_name( argument ) )
        {
            write_to_buffer( d, "\n\rInvalid name, follow these guidelines when choosing a name:\n\r"
                             "1) No real words.\n\r"
                             "2) No obscene words.\n\r"
                             "3) No compound words.\n\r"
                             "4) Alpha characters only.\n\r", 0 );
            write_to_buffer( d, "What is your name? ", 0);
            return;
        }

        if ( check_playing( d, argument, FALSE ) == BERR )
        {
            write_to_buffer( d, "What is your name? ", 0);
            return;
        }

        fOld = load_char_obj( d, argument, TRUE, NULL, FALSE );
        if ( !d->character )
        {
            sprintf( log_buf, "Bad player file %s@%s.", argument, d->host );
            log_string_plus( log_buf, LOG_BUG, LEVEL_LOG_CSET, SEV_ERR );
            write_to_buffer( d, "Your playerfile is corrupt...Please notify an immortal.\n\r", 0 );
            close_socket( d, FALSE );
            return;
        }
        ch   = d->character;

        for ( pban = first_ban; pban; pban = pban->next )
        {
            /* This used to use str_suffix, but in order to do bans by the
             first part of the ip, ie "ban 207.136.25" str_prefix must
             be used. -- Narn
             */
            if ( (!str_prefix( pban->name, d->host ) ||
                  !str_suffix( pban->name, d->host )) &&
                 pban->level >= GetMaxLevel(ch) )
            {
                write_to_buffer( d,
                                 "Your site has been banned from this MUD.\n\r"
                                 "For more information, email dotd@dotd.com\n\r", 0 );
                close_socket( d, FALSE );
                return;
            }
        }
        if ( IS_SET(ch->act, PLR_DENY) )
        {
            sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
            log_string_plus( log_buf, LOG_COMM, sysdata.log_level, SEV_CRIT );
            if (d->newstate != 0)
            {
                write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
                d->connected = CON_GET_NAME;
                d->character->desc = NULL;
                free_char( d->character );
                d->character = NULL;
                return;
            }
            write_to_buffer( d, "You are denied access.\n\r", 0 );
            close_socket( d, FALSE );
            return;
        }

        chk = check_reconnect( d, argument, FALSE );
        if ( chk == BERR )
            return;

        if ( chk )
        {
            fOld = TRUE;
        }
        else
        {
            if ( wizlock && !IS_IMMORTAL(ch) )
            {
                write_to_buffer( d, "The game is wizlocked.  Only immortals can connect now.\n\r", 0 );
                write_to_buffer( d, "Please try back later.\n\r", 0 );
                close_socket( d, FALSE );
                return;
            }
        }

        if ( fOld )
        {
            if (d->newstate != 0)
            {
                write_to_buffer( d, SEPERATOR_LINE, 0 );
                write_to_buffer( d, "That name is already taken, please choose another.\n\r", 0 );
                write_to_buffer( d, "What is your name? ", 0);
                d->connected = CON_GET_NAME;
                d->character->desc = NULL;
                free_char( d->character );
                d->character = NULL;
                return;
            }
            /* Old player */
            write_to_buffer( d, "Password: ", 0 );
            write_to_buffer( d, echo_off_str, 0 );
            d->connected = CON_GET_OLD_PASSWORD;
            return;
        }
        else
        {
            if (sysdata.DENY_NEW_PLAYERS == TRUE)
            {
                write_to_buffer( d, "Currently no new players are allowed.\n\r", 0);
                close_socket(d, FALSE);
                return;
            }
            if (d->newstate == 0)
            {
                /* No such player */
                write_to_buffer( d, "\n\rPlease make sure your name follows these guidelines.\n\r"
                                 "1) No real words.\n\r"
                                 "2) No obscene words.\n\r"
                                 "3) No compound words.\n\r"
                                 "4) Alpha characters only.\n\r\n\r", 0 );
                sprintf( buf, "Are you sure you want to be called '%s' (Y/N)? ", argument);
                write_to_buffer( d, buf, 0 );
                d->connected = CON_CONFIRM_NEW_NAME;
                return;
            }

            return;
        }
        break;

    case CON_GET_OLD_PASSWORD:
        write_to_buffer( d, "\n\r", 2 );

        if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
        {
            log_string_plus( "Entered wrong password.", LOG_COMM, sysdata.log_level, SEV_ERR );
            write_to_buffer( d, "Wrong password.\n\r", 0 );
            /* clear descriptor pointer to get rid of bug message in log */
            d->character->desc = NULL;
            close_socket( d, FALSE );
            return;
        }

        write_to_buffer( d, echo_on_str, 0 );

        if ( check_playing( d, ch->name, TRUE ) )
            return;

        chk = check_reconnect( d, ch->name, TRUE );
        if ( chk == BERR )
        {
            if ( d->character && d->character->desc )
                d->character->desc = NULL;
            close_socket( d, FALSE );
            return;
        }
        if ( chk == TRUE )
            return;

        sprintf( buf, "%s", ch->name );
        d->character->desc = NULL;
        free_char( d->character );
        fOld = load_char_obj( d, buf, FALSE, NULL, TRUE );
        ch = d->character;
        last_log( ch->name, d );
        sprintf( log_buf, "%s@%s(%s) has connected.",
                 ch->name, d->host, d->user );
        log_string_plus( log_buf, LOG_COMM, UMAX(sysdata.log_level, GetMaxLevel(ch)), SEV_INFO );
        sprintf( log_buf, "%s has connected.", ch->name );
        log_string_plus( log_buf, LOG_MONITOR, UMAX(LEVEL_IMMORTAL, GetMaxLevel(ch)), SEV_NOTICE );
        show_title(d);
        d->connected = CON_PRESS_ENTER;
        break;

    case CON_CONFIRM_NEW_NAME:
        switch ( *argument )
        {
        case 'y': case 'Y':
            write_to_buffer( d, SEPERATOR_LINE, 0 );
            sprintf( buf, "Make sure to use a password that won't be easily guessed by someone else."
                     "\n\rPick a good password for %s: %s",
                     ch->name, echo_off_str );
            write_to_buffer( d, buf, 0 );
            d->connected = CON_GET_NEW_PASSWORD;
            break;

        case 'n': case 'N':
            write_to_buffer( d, "Ok, what IS your name, then? ", 0 );
            /* clear descriptor pointer to get rid of bug message in log */
            d->character->desc = NULL;
            free_char( d->character );
            d->character = NULL;
            d->connected = CON_GET_NAME;
            break;

        default:
            write_to_buffer( d, "Invalid answer, please type Y for yes or N for no.\n\r", 0 );
            sprintf( buf, "Did I get that right, %s (Y/N)? ", argument);
            write_to_buffer( d, buf, 0 );
            break;
        }
        break;

    case CON_GET_NEW_PASSWORD:
        write_to_buffer( d, "\n\r", 2 );
        if ( strlen(argument) < 5 )
        {
            write_to_buffer( d,
                             "Password must be at least five characters long.\n\rPassword: ",
                             0 );
            return;
        }
        if ( !str_cmp(ch->name,argument) )
        {
            write_to_buffer( d,
                             "You cannot use your name as your password.\n\rPassword: ",
                             0 );
            return;
        }

        pwdnew = crypt( argument, ch->name );
        for ( p = pwdnew; *p != '\0'; p++ )
        {
            if ( *p == '~' )
            {
                write_to_buffer( d,
                                 "New password not acceptable, try again.\n\rPassword: ",
                                 0 );
                return;
            }
        }

        DISPOSE( ch->pcdata->pwd );
        ch->pcdata->pwd	= str_dup( pwdnew );
        write_to_buffer( d, "Please retype the password to confirm: ", 0 );
        d->connected = CON_CONFIRM_NEW_PASSWORD;
        break;

    case CON_CONFIRM_NEW_PASSWORD:
        write_to_buffer( d, "\n\r", 2 );
        if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
        {
            write_to_buffer( d, "Invalid answer: Passwords don't match.\n\r", 0 );
            sprintf( buf, "Make sure to use a password that won't be easily guessed by someone else."
                     "\n\rPick a good password for %s: %s",
                     ch->name, echo_off_str );
            write_to_buffer( d, buf, 0 );
            d->connected = CON_GET_NEW_PASSWORD;
            return;
        }

        write_to_buffer( d, echo_on_str, 0 );
        write_to_buffer( d, SEPERATOR_LINE, 0 );
        write_to_buffer( d, "You can chose from the following races:\n\r\n\r", 0 );
        show_race_choice(d);
        write_to_buffer( d, "\n\rType help [race] to learn more, or\n\r", 0 );
        write_to_buffer( d, "Enter the name or number of the race you wish to be: ", 0);
        d->connected = CON_GET_NEW_RACE;
        break;

    case CON_GET_NEW_RACE:
        one_argument(argument, arg);
        if (!str_cmp( arg, "help") )
        {
            argument = one_argument(argument, arg);
            for ( iRace = 0; iRace < MAX_RACE; iRace++ )
            {
                if ( toupper(argument[0]) == toupper(race_table[iRace].race_name[0])
                     &&  !str_prefix( argument, race_table[iRace].race_name) )
                {
                    do_help(ch, argument);
                    write_to_buffer( d, "\n\rEnter the name or number of the race you wish to be: ", 0);
                    return;
                }
            }
            write_to_buffer( d, "I'm sorry, there is no help on that race.\n\r", 0 );
            write_to_buffer( d, "\n\rEnter the name or number of the race you wish to be: ", 0);
            return;
        }

        if (!is_number(argument))
        {
            for ( i = 0; i < MAX_RACE; i++ )
                if ( race_table[i].is_pc_race &&
                     !str_cmp( argument, race_table[i].race_name ) )
                    break;
            if ( i >= MAX_RACE || str_cmp( argument, race_table[i].race_name ) )
            {
                write_to_buffer( d, "Invalid answer, please try again.\n\r", 0 );
                write_to_buffer( d, "\n\rEnter the name or number of the race you wish to be: ", 0);
                return;
            }

            iCnt = 0;
            for ( iRace = 0; iRace < i; iRace++ )
                if (race_table[iRace].is_pc_race)
                    iCnt++;
            i = iCnt;
        }
        else
            i = atoi(argument);

        iCnt = 0;

        for ( iRace = 0; iRace < MAX_RACE; iRace++ )
        {
            if (race_table[iRace].is_pc_race) {
                if (i == iCnt) {
                    ch->race = iRace;
                    break;
                } else {
                    ++iCnt;
                }
            }
        }

        if (i != iCnt || i>MAX_PC_RACE)
        {
            write_to_buffer( d, "Invalid answer, please try again.\n\r", 0 );
            write_to_buffer( d, "\n\rEnter the name or number of the race you wish to be: ", 0);
            return;
        }

        /*
        if (GET_RACE(ch) == RACE_HUMAN ||
            GET_RACE(ch) == RACE_HALF_ELF ||
            GET_RACE(ch) == RACE_MFLAYER)
        {
            write_to_buffer( d, SEPERATOR_LINE, 0 );
            write_to_buffer( d,
                             "A planar character is one from another plane of existance.  For more\n\r"
                             "detailed information please visit our website at www.dotd.com.  If you\n\r"
                             "don't know the answer to this, just press enter.\n\r\n\r"
                             "Pros:\n\r"
                             "  You can see hidden portals between the planes, and use them freely.\n\r"
                             "Cons:\n\r"
                             "  You can be summoned via a 'monster summoning' spell cast on the\n\r"
                             "  prime material plane (note that if you die when summoned like this\r"
                             "  you won't loose equipment, experience, or anything, and you will be\n\r"
                             "  returned to your home plane)\n\r"
                             "\n\rDo you want to be a planar character, press enter for the default (Y/N)? ", 0 );
            d->connected = CON_GET_PLANAR;
            return;
        }
        */

        if (GET_RACE(ch) == RACE_GITHZERAI ||
            GET_RACE(ch) == RACE_TIEFLING)
            SET_PLR2_FLAG(ch, PLR2_PLANAR);

        write_to_buffer( d, SEPERATOR_LINE, 0 );
        write_to_buffer( d, "What is your gender (M/F)? ", 0 );
        d->connected = CON_GET_NEW_SEX;
        break;


    case CON_GET_PLANAR:
        switch ( argument[0] )
        {
        case 'y': case 'Y':
            SET_PLR2_FLAG(ch, PLR2_PLANAR);
            break;
        default:
            break;
        }

        write_to_buffer( d, SEPERATOR_LINE, 0 );
        write_to_buffer( d, "What is your gender (M/F)? ", 0 );
        d->connected = CON_GET_NEW_SEX;
        break;

    case CON_GET_NEW_SEX:
        switch ( argument[0] )
        {
        case 'm': case 'M': ch->sex = SEX_MALE;   break;
        case 'f': case 'F': ch->sex = SEX_FEMALE; break;
        case 'b': case 'B':
            d->connected = CON_GET_NEW_RACE;
            return;
            break;
        default:
            ch->sex = SEX_MALE;
            write_to_buffer( d, "Defaulting to Male.\n\r", 0 );
            break;
        }

        d->connected = CON_GET_NEW_CLASS;
        write_to_buffer( d, SEPERATOR_LINE, 0 );
        show_class_choice(d);
        write_to_buffer(d, "\n\rPlease enter the number of your class choice: ", 0);

        break;

    case CON_GET_NEW_CLASS:
        argument = one_argument(argument, arg);

        if (!str_cmp(arg, "help"))
        {

            for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
            {
                if ( toupper(argument[0]) == toupper(class_table[iClass]->who_name[0])
                     &&   !str_prefix( argument, class_table[iClass]->who_name ) )
                {
                    do_help(ch, argument);
                    write_to_buffer(d, "\n\rPlease enter the name or number of your class choice: ", 0);
                    return;
                }
            }
            write_to_buffer( d, "I'm sorry, but there is no help on that class.\n\r", 0 );
            write_to_buffer(d, "Please enter the number of your class choice: ", 0);
            return;
        }

        if (!is_number(arg))
        {
            write_to_buffer(d, "Invalid answer, that is not a class.\n\r", 0);
            write_to_buffer(d, "Please enter the number of your class choice: ", 0);
            return;
        }

        SaveRace = 0;
        for (i = 0; i < d->character->race; ++i)
            if (race_table[i].is_pc_race)
                ++SaveRace;

        i = atoi(arg);

        if (i>=MAX_CHOICES || SaveRace>=MAX_PC_RACE ||
            !ch_class_choice[SaveRace][i])
        {
            write_to_buffer(d, "Invalid answer, that is not a class, please try again.\n\r", 0);
            write_to_buffer(d, "Please enter the number of your class choice: ", 0);
            return;
        }

        sprintf(buf, "%s", ch_class_choice[SaveRace][i]);

        if (strstr(buf, "z") && d->character->sex!=SEX_FEMALE)
        {
            write_to_buffer(d, "I'm sorry, but the Amazon class is only for female characters, try again.\n\r", 0);
            write_to_buffer(d, "Please enter the number of your class choice: ", 0);
            return;
        }

        for (j = 0; buf[j]; ++j)
        {
            switch (buf[j])
            {
            case 'w': d->character->classes[CLASS_WARRIOR] = STAT_ACTCLASS;
            break;
            case 'm': d->character->classes[CLASS_MAGE] = STAT_ACTCLASS;
            break;
            case 'c': d->character->classes[CLASS_CLERIC] = STAT_ACTCLASS;
            break;
            case 't': d->character->classes[CLASS_THIEF] = STAT_ACTCLASS;
            break;
            case 'v': d->character->classes[CLASS_VAMPIRE] = STAT_ACTCLASS;
            break;
            case 'd': d->character->classes[CLASS_DRUID] = STAT_ACTCLASS;
            break;
            case 'z': d->character->classes[CLASS_AMAZON] = STAT_ACTCLASS;
            break;
            case 'r': d->character->classes[CLASS_RANGER] = STAT_ACTCLASS;
            break;
            case 'p': d->character->classes[CLASS_PALADIN] = STAT_ACTCLASS;
            break;
            case 'b': d->character->classes[CLASS_BARBARIAN] = STAT_ACTCLASS;
            break;
            case 's': d->character->classes[CLASS_NECROMANCER] = STAT_ACTCLASS;
            break;
            case 'o': d->character->classes[CLASS_MONK] = STAT_ACTCLASS;
            break;
            case 'i': d->character->classes[CLASS_PSIONIST] = STAT_ACTCLASS;
            break;
            case 'l': d->character->classes[CLASS_ARTIFICER] = STAT_ACTCLASS;
            break;
            case 'n': d->character->classes[CLASS_ANTIPALADIN] = STAT_ACTCLASS;
            break;
            case 'e': d->character->classes[CLASS_SORCERER] = STAT_ACTCLASS;
            break;
            default: log_string_plus("Invalid class value in ch_class_choice", LOG_BUG, LEVEL_LOG_CSET, SEV_ERR);
            break;
            }
        }

        write_to_buffer( d, SEPERATOR_LINE, 0 );
        write_to_buffer( d, "Do you want to continue with advance character generation (Y/N)? ", 0 );
        d->connected = CON_SIMPLE_ADVANCED;

        break;

    case CON_SIMPLE_ADVANCED:
        switch ( argument[0] )
        {
        case 'y': case 'Y':
            write_to_buffer( d, SEPERATOR_LINE, 0 );

            if (IsNeutralSide(ch))
            {
                write_to_buffer( d, "Press [ENTER]", 0 );
                d->connected = CON_GET_START_TOWN;
                return;
            }

            write_to_buffer( d, "Select your stat priority, by listing from highest to lowest.\n\r\n\r"
                             "Use the following values, separated by spaces, on ONE line:\n\r", 0);
            write_to_buffer( d, "s  - Strength       Increases attack strength, prime for warriors\n\r"
                             "i  - Intelligence   Allows you to learn faster, and use mind/magic better\n\r"
                             "w  - Wisdom         Prime requisite for magic users\n\r"
                             "d  - Dexterity      Good for thieves and warriors\n\r"
                             "co - Constitution   Determines HP gain at level, prime for warrior types\n\r"
                             "ch - Charisma       Good for thieves, and others who want to influence people\n\r"
                             "l  - Luck           Just about anybody can use more luck\n\r\n\r", 0 );
            write_to_buffer(d, "For example, enter: s i w d co ch l\n\r", 0);
            write_to_buffer(d, "Your choices (press enter for default)? ", 0);

            d->connected = CON_GET_NEW_SCORES;
            return;
        }

        SET_BIT(ch->act,PLR_ANSI);

        ch->perm_str = number_range(10,18);
        ch->perm_int = number_range(9,18);
        ch->perm_wis = number_range(9,18);
        ch->perm_dex = number_range(9,18);
        ch->perm_lck = number_range(8,18);
        ch->perm_cha = number_range(8,18);
        ch->perm_con = number_range(9,18);

        write_to_buffer( d, SEPERATOR_LINE, 0 );
        show_title(d);
        d->connected = CON_PRESS_ENTER;
        break;

    case CON_GET_START_TOWN:
        switch ( argument[0] )
        {
        case '1':
            ch->pcdata->home = ROOM_START_GOOD;
            break;
        case '2':
            ch->pcdata->home = ROOM_START_EVIL;
            break;
        case '3':
            ch->pcdata->home = ROOM_START_NEUTRAL;
            break;
        default:
            ch->pcdata->home = ROOM_START_GOOD;
            break;
        }
        write_to_buffer( d, "Select your stat priority, by listing from highest to lowest.\n\r\n\r"
                         "Use the following values, separated by spaces:\n\r", 0);
        write_to_buffer( d, "s  - Strength       Increases attack strength, prime for warriors\n\r"
                         "i  - Intelligence   Allows you to learn faster, and use mind/magic better\n\r"
                         "w  - Wisdom         Prime requisite for magic users\n\r"
                         "d  - Dexterity      Good for thieves and warriors\n\r"
                         "co - Constitution   Determines HP gain at level, prime for warrior types\n\r"
                         "ch - Charisma       Good for thieves, and others who want to influence people\n\r"
                         "l  - Luck           Just about anybody can use more luck\n\r\n\r", 0 );
        write_to_buffer(d, "For example, enter: s i w d co ch l\n\r", 0);
        write_to_buffer(d, "Your choices (press enter for default)? ", 0);
        d->connected = CON_GET_NEW_SCORES;
        break;

    case CON_GET_NEW_SCORES:
        if (*argument == '\0')
        {
            write_to_buffer( d, SEPERATOR_LINE, 0 );
            write_to_buffer(d, "Do you want to be a player killer (Y/N)? ", 0);
            d->connected = CON_GET_DEADLY;
            return;
        }

        pstats[0] = number_range(12, 18);
        pstats[1] = number_range(10, 18);
        for (i = 2; i < MAX_STATS; ++i)
            pstats[i] = number_range(8, 18);

        for (i = 1; i < MAX_STATS; ++i)
            for(j = MAX_STATS-1; j >= i; --j)
                if (pstats[j - 1] < pstats[j])
                {
                    iCnt = pstats[j - 1];
                    pstats[j - 1] = pstats[j];
                    pstats[j] = iCnt;
                }

	log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_DEBUG, "%s rolling: %d %d %d %d %d %d %d\n\r", GET_NAME(ch), pstats[0], pstats[1], pstats[2], pstats[3], pstats[4], pstats[5], pstats[6]);

        iCnt = i = j = 0;
        while (*argument)
        {
            argument = one_argument(argument, arg);
            switch(toupper(arg[0]))
            {
            case 'S':
                ch->perm_str = pstats[j];
                SET_BIT(iCnt, BV00);
                break;
            case 'I':
                ch->perm_int = pstats[j];
                SET_BIT(iCnt, BV01);
                break;
            case 'W':
                ch->perm_wis = pstats[j];
                SET_BIT(iCnt, BV02);
                break;
            case 'D':
                ch->perm_dex = pstats[j];
                SET_BIT(iCnt, BV03);
                break;
            case 'L':
                ch->perm_lck = pstats[j];
                SET_BIT(iCnt, BV04);
                break;
            case 'C':
                switch(toupper(arg[1]))
                {
                case 'H':
                    ch->perm_cha = pstats[j];
                    SET_BIT(iCnt, BV05);
                    break;
                case 'O':
                    ch->perm_con = pstats[j];
                    SET_BIT(iCnt, BV06);
                    break;
                }
                break;
            }
	    j++;
        }
        SET_BIT(i, BV00);
        SET_BIT(i, BV01);
        SET_BIT(i, BV02);
        SET_BIT(i, BV03);
        SET_BIT(i, BV04);
        SET_BIT(i, BV05);
        SET_BIT(i, BV06);
        if (iCnt != i)
        {
            write_to_buffer( d, "Invalid answer, please try again.\n\r\n\r"
                             "Use the following values, separated by spaces:\n\r", 0);
            write_to_buffer( d, "s  - Strength       Increases attack strength, prime for warriors\n\r"
                             "i  - Intelligence   Allows you to learn faster, and use mind/magic better\n\r"
                             "w  - Wisdom         Prime requisite for magic users\n\r"
                             "d  - Dexterity      Good for thieves and warriors\n\r"
                             "co - Constitution   Determines HP gain at level, prime for warrior types\n\r"
                             "ch - Charisma       Good for thieves, and others who want to influence people\n\r"
                             "l  - Luck           Just about anybody can use more luck\n\r\n\r", 0 );
            write_to_buffer(d, "Example     : s co d i l ch w\n\r", 0);
            write_to_buffer(d, "Your choices? ", 0);
            return;
        }

        write_to_buffer( d, SEPERATOR_LINE, 0 );
        write_to_buffer(d, "Do you want to be a player killer (Y/N)? ", 0);
        d->connected = CON_GET_DEADLY;
        break;

    case CON_GET_DEADLY:
        switch ( argument[0] )
        {
        case 'y': case 'Y':
            SET_PC_FLAG( ch, PCFLAG_DEADLY );
            break;
        }

        write_to_buffer( d, SEPERATOR_LINE, 0 );
        write_to_buffer(d, "Do you want RIP, ANSI, or Normal display (R/A/N)? ", 0);
        d->connected = CON_GET_WANT_RIPANSI;
        break;

    case CON_GET_WANT_RIPANSI:
        switch ( argument[0] )
        {
        case 'r': case 'R':
            SET_BIT(ch->act,PLR_RIP);
            SET_BIT(ch->act,PLR_ANSI);
            break;
        case 'n': case 'N':
            break;
        default:
            write_to_buffer( d, "Defaulting to ANSI.\n\r", 0 );
        case 'a': case 'A':
            SET_BIT(ch->act,PLR_ANSI);
            break;
        }
#if 0 /* enable for interface selection */
        write_to_buffer( d, SEPERATOR_LINE, 0 );
        write_to_buffer( d,
                         "1] Dale / Silly / DOTD (default)\n\r"
                         "2] SMAUG\n\r"
                         "3] MERC / ENVY\n\r"
                         "\n\rWhich interface do you prefer (enter for default): ", 0 );
        d->connected = CON_GET_INTERFACE;
        break;

    case CON_GET_INTERFACE:
        switch ( argument[0] )
        {
        case '1':
            do_interface(ch, "dale"); break;
        case '2':
            do_interface(ch, "dale"); break;
        case '3':
            do_interface(ch, "dale"); break;
        default:
            do_interface(ch, "dale"); break;
        }
#endif
        show_title(d);
        d->connected = CON_PRESS_ENTER;
        break;

        /*	}

         write_to_buffer( d, "\n\rYou now have to wait for a god to authorize you... please be patient...\n\r", 0 );
         sprintf( log_buf, "(1) %s@%s new %s %s applying for authorization...",
         ch->name, d->host,
         race_table[ch->race].race_name,
         GetClassString(ch));
         log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL);
         d->connected = CON_WAIT_1;
         break;

         case CON_WAIT_1:
         write_to_buffer( d, "\n\rTwo more tries... please be patient...\n\r", 0 );
         sprintf( log_buf, "(2) %s@%s new %s %s applying for authorization...",
         ch->name, d->host,
         race_table[ch->race].race_name,
         GetClassString(ch) );
         log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL);
         d->connected = CON_WAIT_2;
         break;

         case CON_WAIT_2:
         write_to_buffer( d, "\n\rThis is your last try...\n\r", 0 );
         sprintf( log_buf, "(3) %s@%s new %s %s applying for authorization...",
         ch->name, d->host,
         race_table[ch->race].race_name,
         GetClassString(ch));
         log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL);
         d->connected = CON_WAIT_3;
         break;

         case CON_WAIT_3:
         write_to_buffer( d, "Sorry... try again later.\n\r", 0 );
         close_socket( d, FALSE );
         return;
         break;

         case CON_ACCEPTED:

         sprintf( log_buf, "%s@%s new %s %s.", ch->name, d->host,
         race_table[ch->race].race_name,
         GetClassString(ch) );
         log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
         log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL);
         write_to_buffer( d, "\n\r", 2 );
         show_title(d);

         ch->position = POS_STANDING;
         d->connected = CON_PRESS_ENTER;
         break;
         */
    case CON_PRESS_ENTER:
        if ( IS_SET(ch->act, PLR_RIP) )
            send_rip_screen(ch);
        set_pager_color( AT_PLAIN, ch );
        send_to_pager( "\n\r", ch );
        if ( IS_IMMORTAL(ch) )
            do_help( ch, "imotd" );
        if ( GetMaxLevel(ch) >= 50)
            do_help( ch, "amotd" );
        if ( GetMaxLevel(ch) < 50 && GetMaxLevel(ch) > 0 )
            do_help( ch, "motd" );
        if ( GetMaxLevel(ch) == 0 )
            do_help( ch, "nmotd" );
        set_pager_color( AT_PLAIN, ch );
        send_to_pager( "\n\rPress [ENTER] ", ch );
        d->connected = CON_READ_MOTD;
        break;

    case CON_READ_MOTD:
        write_to_buffer( d, SEPERATOR_LINE, 0 );
        write_to_buffer( d, "You wake up and open your eyes...\n\r\n\r", 0);
        add_char( ch );
        d->connected = CON_PLAYING;
        ch->position = POS_STANDING;
        if ( GetMaxLevel(ch) == 0 )
        {
            int iLang;

            sprintf( log_buf, "New char: %s@%s", ch->name, d->host);
            log_string_plus( log_buf, LOG_MONITOR, LEVEL_IMMORTAL, SEV_NOTICE );
            write_to_buffer( d, SEPERATOR_LINE, 0 );

            ch->pcdata->clan_name = STRALLOC( "" );
            ch->pcdata->clan	  = NULL;

            ch->perm_str	 += race_table[ch->race].str_plus;
            ch->perm_int	 += race_table[ch->race].int_plus;
            ch->perm_wis	 += race_table[ch->race].wis_plus;
            ch->perm_dex	 += race_table[ch->race].dex_plus;
            ch->perm_con	 += race_table[ch->race].con_plus;
            ch->perm_cha	 += race_table[ch->race].cha_plus;
            ch->affected_by	  = race_table[ch->race].affected;
            ch->perm_lck	 += race_table[ch->race].lck_plus;

            if ( (iLang = skill_lookup( "common" )) < 0 )
            {
                bug( "Nanny: cannot find common language." );
            }
            else
                ch->pcdata->learned[iLang] = GET_ADEPT(ch, iLang);

            for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
                if ( lang_array[iLang] == race_table[ch->race].language )
                    break;
            if ( lang_array[iLang] == LANG_UNKNOWN )
            {
                bug( "Nanny: invalid racial language." );
            }
            else
            {
                if ( (iLang = skill_lookup( lang_names[iLang] )) < 0 )
                {
                    bug( "Nanny: cannot find racial language." );
                }
                else
                    ch->pcdata->learned[iLang] = GET_ADEPT(ch, iLang);
            }

            if (gsn_pray)
                ch->pcdata->learned[gsn_pray] = GET_ADEPT(ch, gsn_pray);
            if (gsn_drinking)
                ch->pcdata->learned[gsn_drinking] = 1;

            /* ch->resist           += race_table[ch->race].resist;    drats */
            /* ch->susceptible     += race_table[ch->race].suscept;    drats */

            StartLevels(ch);
            sprintf(buf, "the %s %s", get_race_name(ch), GetTitleString(ch));
            set_title(ch, buf);
            /* Added below as nanny option 11/27/98 by Heath */
            /*          ch->pcdata->interface = INT_DEFAULT; */
            GET_EXP(ch)	 = 0;
            ch->antimagicp      = 0;
            GET_MONEY(ch,DEFAULT_CURR)  = 150;
            GET_BALANCE(ch,DEFAULT_CURR) = 300;
            GET_PRACS(ch) = wis_app[get_curr_wis(ch)].practice;
            ch->max_hit		+= race_table[ch->race].hit;
            ch->max_mana	+= race_table[ch->race].mana;
            GET_HIT(ch)		= GET_MAX_HIT(ch);
            GET_MANA(ch)	= GET_MAX_MANA(ch);
            GET_MOVE(ch)	= GET_MAX_MOVE(ch);
            if (HAS_CLASS(ch, CLASS_PALADIN))
                ch->alignment = 1000;
            else if (HAS_CLASS(ch, CLASS_ANTIPALADIN) ||
                     HAS_CLASS(ch, CLASS_VAMPIRE))
                ch->alignment = -1000;
            else if (!HAS_CLASS(ch, CLASS_DRUID))
            {
                if (IsGoodSide(ch))
                    ch->alignment = 250;
                else if (IsBadSide(ch))
                    ch->alignment = -250;
            }

            SET_PLR_FLAG( ch, PLR_AUTOGOLD );
            SET_PLR_FLAG( ch, PLR_AUTOLOOT );
#ifdef PLR2_AUTOGAIN
            SET_PLR2_FLAG( ch, PLR2_AUTOGAIN );
#endif
            SET_PLR_FLAG( ch, PLR_AUTOSAC );
            SET_PLR_FLAG( ch, PLR_AUTOEXIT );
            SET_PC_FLAG ( ch, PCFLAG_PAGERON );

            if (IsBadSide(ch))
                ch->pcdata->home = ROOM_START_EVIL;
            else if (IsGoodSide(ch) || !ch->pcdata->home)
                ch->pcdata->home = ROOM_START_GOOD;

            if (!sysdata.WAIT_FOR_AUTH)
	    {
                load_area_demand( ch->pcdata->home );
                char_to_room( ch, get_room_index( ch->pcdata->home ) );
            }
            else
            {
                load_area_demand( ROOM_AUTH_START );
                char_to_room( ch, get_room_index( ROOM_AUTH_START ) );
                ch->pcdata->auth_state = 0;
                SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
            }
            /* Display_prompt interprets blank as default */
            ch->pcdata->prompt = STRALLOC("");
            if (ch->pcdata->rank)
                DISPOSE(ch->pcdata->rank);
            sprintf(buf, "%s %s", race_table[GET_RACE(ch)].race_name, GetClassString(ch));
            ch->pcdata->rank = str_dup(buf);
        }
        else
        {
            if ( !IS_IMMORTAL(ch) && ch->pcdata->release_date > current_time )
            {
                char_to_room( ch, get_room_index(ROOM_VNUM_HELL) );
            }
            else if ( ch->in_room && ( IS_IMMORTAL( ch )
                                       || !IS_SET( ch->in_room->room_flags, ROOM_PROTOTYPE ) ) )
            {
                if ( ch->in_room->vnum == ROOM_VNUM_LIMBO )
                {
		    if (IsBadSide(ch))
		    {
			load_area_demand( ROOM_START_EVIL );
			char_to_room( ch, get_room_index( ROOM_START_EVIL ) );
		    }
		    else
		    {
			load_area_demand( ROOM_START_GOOD );
			char_to_room( ch, get_room_index( ROOM_START_GOOD ) );
		    }
                }
                else
                {
                    char_to_room( ch, ch->in_room );
                }
            }
            else if ( IS_IMMORTAL(ch) )
            {
		load_area_demand( ROOM_VNUM_CHAT );
                char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
            }
            else
            {
                if (IsBadSide(ch))
		{
		    load_area_demand( ROOM_START_EVIL );
		    char_to_room( ch, get_room_index( ROOM_START_EVIL ) );
		}
		else
		{
		    load_area_demand( ROOM_START_GOOD );
		    char_to_room( ch, get_room_index( ROOM_START_GOOD ) );
		}
            }

            process_rent(ch);

            if (ch->pcdata->time_to_die > 0)
            {
                if (ch->pcdata->time_to_die > current_time)
                    ch_printf(ch, "Your death sentence is in %s.\n\r",
                              sec_to_hms(ch->pcdata->time_to_die-current_time));
                else
                {
                    send_to_char("Your death sentence is up!\n\r", ch);
                    sprintf(log_buf, "%s's death sentence is up", GET_NAME(ch));
                    log_string_plus(log_buf, LOG_MONITOR, UMAX(GetMaxLevel(ch), LEVEL_IMMORTAL), SEV_CRIT);
               }
            }
        }

        if (gsn_pray && !LEARNED(ch, gsn_pray))
            ch->pcdata->learned[gsn_pray] = GET_ADEPT(ch, gsn_pray);
        if (gsn_drinking && !LEARNED(ch, gsn_drinking))
            ch->pcdata->learned[gsn_drinking] = 1;

        if ( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
            remove_timer( ch, TIMER_SHOVEDRAG );

        if ( get_timer( ch, TIMER_PKILLED ) > 0 )
            remove_timer( ch, TIMER_PKILLED );

        mail_count(ch);
        bugtrack_check(ch);

        act( AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
        do_look( ch, "auto" );
#ifdef I3
        I3_char_login(ch);
#endif

#ifdef IRC
    irc_logon(ch);
#endif

#ifdef USE_CRBS
        do_clogon(ch, NULL); /* CRBS login */
#endif

        log_string_plus("\a*beep*", LOG_MONITOR, GetMaxLevel(d->character), SEV_SPAM+9);
        break;
    }

    return;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    if (!isalpha(*name))
        return FALSE;

    /*
     * Reserved words.
     */
    if ( is_name( name, "all auto immortal nobody self someone god supreme demigod dog guard cityguard cat death ass fuck shit piss crap quit cum cunt pussy testes vagina butt" ) )
        return FALSE;

    /*
     * Length restrictions.
     */
    if ( strlen(name) < 3 )
        return FALSE;

    if ( strlen(name) > 12 )
        return FALSE;

#ifdef USE_DICT
    if (num_words_in_dict(name))
        return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
        char *pc;
        bool fIll;

        fIll = TRUE;
        for ( pc = name; *pc != '\0'; pc++ )
        {
            if ( !isalpha(*pc) || *pc == ' ' )
                return FALSE;
            if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
                fIll = FALSE;
        }

        if ( fIll )
            return FALSE;
    }

    /*
     * Code that followed here used to prevent players from naming
     * themselves after mobs... this caused much havoc when new areas
     * would go in...
     */

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = first_char; ch; ch = ch->next )
    {
        if ( !IS_NPC(ch)
             && ( !fConn || !ch->desc )
             &&    ch->name
             &&   !str_cmp( name, ch->name ) )
        {
            if ( fConn && ch->switched )
            {
                write_to_buffer( d, "Already playing.\n\rName: ", 0 );
                d->connected = CON_GET_NAME;
                if ( d->character )
                {
                    /* clear descriptor pointer to get rid of bug message in log */
                    d->character->desc = NULL;
                    free_char( d->character );
                    d->character = NULL;
                }
                return BERR;
            }
            if ( fConn == FALSE )
            {
                DISPOSE( d->character->pcdata->pwd );
                d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
            }
            else
            {
                /* clear descriptor pointer to get rid of bug message in log */
                d->character->desc = NULL;
                free_char( d->character );
                d->character = ch;
                ch->desc	 = d;
                ch->timer	 = 0;
                send_to_char( "Reconnecting.\n\r", ch );
                act( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
                sprintf( log_buf, "%s@%s(%s) reconnected.", ch->name, d->host, d->user );
                log_string_plus( log_buf, LOG_COMM, UMAX(sysdata.log_level, GetMaxLevel(ch) ), SEV_INFO );
                d->connected = CON_PLAYING;
#ifdef I3
                I3_char_login(ch);
#endif

#ifdef IRC
    irc_logon(ch);
#endif


#ifdef USE_CRBS
                do_clogon(ch, NULL); /* CRBS login */
#endif
            }
            return TRUE;
        }
    }

    return FALSE;
}


/*
 * Check if already playing.
 */
sh_int check_playing( DESCRIPTOR_DATA *d, char *name, bool kick )
{
    CHAR_DATA *ch;

    DESCRIPTOR_DATA *dold;
    int	cstate;

    for ( dold = first_descriptor; dold; dold = dold->next )
    {
        if ( dold != d
             && (  dold->character || dold->original )
             &&   !str_cmp( name, dold->original
                            ? dold->original->name : dold->character->name ) )
        {
            cstate = dold->connected;
            ch = dold->original ? dold->original : dold->character;
            if ( !ch->name
                 || ( cstate != CON_PLAYING && cstate != CON_EDITING ) )
            {
                write_to_buffer( d, "Already connected - try again.\n\r", 0 );
                sprintf( log_buf, "%s already connected.", ch->name );
                log_string_plus( log_buf, LOG_COMM, sysdata.log_level, SEV_INFO );
                return BERR;
            }
            if ( !kick )
                return TRUE;
            write_to_buffer( d, "Already playing... Kicking off old connection.\n\r", 0 );
            write_to_buffer( dold, "Kicking off old connection... bye!\n\r", 0 );
            close_socket( dold, FALSE );
            /* clear descriptor pointer to get rid of bug message in log */
            d->character->desc = NULL;
            free_char( d->character );
            d->character = ch;
            ch->desc	 = d;
            ch->timer	 = 0;
            if ( ch->switched )
                do_return( ch->switched, "" );
            ch->switched = NULL;
            send_to_char( "Reconnecting.\n\r", ch );
            act( AT_ACTION, "$n has reconnected, kicking off old link.",
                 ch, NULL, NULL, TO_ROOM );
            sprintf( log_buf, "%s@%s reconnected, kicking off old link.",
                     ch->name, d->host );
            log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, GetMaxLevel(ch) ), SEV_INFO );
            d->connected = cstate;
#ifdef I3
            I3_char_login(ch);
#endif

#ifdef IRC
    irc_logon(ch);
#endif

#ifdef USE_CRBS
            do_clogon(ch, NULL); /* CRBS login */
#endif
            return TRUE;
        }
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( !ch
         ||   !ch->desc
         ||    ch->desc->connected != CON_PLAYING
         ||   !ch->idle_room
         ||    ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
        return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->idle_room );
    ch->idle_room	= NULL;
    act( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}


void send_to_room( const char *txt, ROOM_INDEX_DATA *rm )
{
    CHAR_DATA *ch;

    if (!txt || !rm)
        return;

    for (ch=rm->first_person;ch;ch=ch->next_in_room)
        if (!char_died(ch) && ch->desc)
            send_to_char(txt,ch);
}

void send_to_area( const char *txt, AREA_DATA *area )
{
    CHAR_DATA *ch;

    if (!txt || !area)
        return;

    for (ch=first_char;ch;ch=ch->next)
        if (!char_died(ch) && ch->desc &&
            ch->in_room && ch->in_room->area == area)
            send_to_char(txt,ch);
}

/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
#if 1
    send_to_char_color(txt,ch);
#else
    char *buf;
    char *prevstr;

    if ( !ch )
    {
        bug( "Send_to_char: NULL *ch" );
        return;
    }

    if ( !txt )
        return;

#ifdef USE_BOA
    if ( IS_WEB(ch) )
    {
        send_to_web(txt,ch);
    }
#endif

    if ( ch->desc )
    {
        prevstr = str_dup(txt);
        buf = str_dup(ParseAnsiColors(IS_SET(ch->act, PLR_ANSI), prevstr));
        write_to_buffer( ch->desc, buf, strlen(buf) );
        DISPOSE(prevstr);
        DISPOSE(buf);
    }

    return;
#endif
}

/*
 * Same as above, but converts &color codes to ANSI sequences..
 */
void send_to_char_color( const char *txt, CHAR_DATA *ch )
{
    DESCRIPTOR_DATA *d;
    char *colstr;
    const char *prevstr = txt;
    char colbuf[40];
    int ln;

    if ( !ch )
    {
        bug( "Send_to_char_color: NULL *ch" );
        return;
    }
    if ( !txt )
        return;

#ifdef USE_BOA
    if ( IS_WEB(ch) )
    {
        send_to_web(txt,ch);
    }
#endif

    d = ch->desc;
    if ( !d )
        return;

    while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
    {
        /* if we pass length=0, then write_to_buffer will do a strlen! */
        if (colstr > prevstr)
            write_to_buffer(d, prevstr, (colstr-prevstr));
        ln = make_color_sequence(colstr, colbuf, d);
        if ( ln < 0 )
        {
            prevstr = colstr+1;
            break;
        }
        else if ( ln > 0 )
            write_to_buffer(d, colbuf, ln);
        prevstr = colstr+2;
    }
    if ( *prevstr )
        write_to_buffer(d, prevstr, 0);
    return;
}

void write_to_pager( DESCRIPTOR_DATA *d, const char *txt, unsigned int length )
{
    int pageroffset;

    if ( length <= 0 )
        length = strlen(txt);
    if ( length == 0 )
        return;
    if ( !d->pagebuf )
    {
        d->pagesize = MAX_STRING_LENGTH;
        CREATE( d->pagebuf, char, d->pagesize );
    }
    if ( !d->pagepoint )
    {
        d->pagepoint = d->pagebuf;
        d->pagetop = 0;
        d->pagecmd = '\0';
    }
    if ( d->pagetop == 0 && !d->fcommand )
    {
        d->pagebuf[0] = '\n';
        d->pagebuf[1] = '\r';
        d->pagetop = 2;
    }
    pageroffset = d->pagepoint - d->pagebuf;
    while ( d->pagetop + length >= d->pagesize )
    {
        if ( d->pagesize > 64000 )
        {
            bug( "Pager overflow, freeing pager" );
            d->pagetop = 0;
            d->pagepoint = NULL;
            DISPOSE(d->pagebuf);
            d->pagesize = MAX_STRING_LENGTH;
            return;
        }
        d->pagesize *= 2;
        RECREATE(d->pagebuf, char, d->pagesize);
    }
    d->pagepoint = d->pagebuf + pageroffset;
    strncpy(d->pagebuf+d->pagetop, txt, length);
    d->pagetop += length;
    d->pagebuf[d->pagetop] = '\0';
    return;
}

void send_to_pager( const char *txt, CHAR_DATA *ch )
{
#if 1
    send_to_pager_color(txt,ch);
#else
    if ( !ch )
    {
        bug( "Send_to_pager: NULL *ch" );
        return;
    }
    if ( txt && ch->desc )
    {
        DESCRIPTOR_DATA *d = ch->desc;

        ch = d->original ? d->original : d->character;
        if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
        {
            send_to_char(txt, d->character);
            return;
        }
        write_to_pager(d, txt, 0);
    }
    return;
#endif
}

void send_to_pager_color( const char *txt, CHAR_DATA *ch )
{
    DESCRIPTOR_DATA *d;
    char *colstr;
    const char *prevstr = txt;
    char colbuf[40];
    int ln;

    if ( !ch )
    {
        bug( "Send_to_pager_color: NULL *ch" );
        return;
    }
    if ( !txt || !ch->desc )
        return;
    d = ch->desc;
    ch = d->original ? d->original : d->character;
    if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
    {
        send_to_char_color(txt, d->character);
        return;
    }
    /* Clear out old color stuff */
    /*  make_color_sequence(NULL, NULL, NULL);*/
    while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
    {
        if ( colstr > prevstr )
            write_to_pager(d, prevstr, (colstr-prevstr));
        ln = make_color_sequence(colstr, colbuf, d);
        if ( ln < 0 )
        {
            prevstr = colstr+1;
            break;
        }
        else if ( ln > 0 )
            write_to_pager(d, colbuf, ln);
        prevstr = colstr+2;
    }
    if ( *prevstr )
        write_to_pager(d, prevstr, 0);
    return;
}

void set_char_color( sh_int AType, CHAR_DATA *ch )
{
    if ( !ch || !ch->desc )
        return;

    write_to_buffer( ch->desc, color_str(AType,ch), 0 );
}

void set_pager_color( sh_int AType, CHAR_DATA *ch )
{
    if ( !ch || !ch->desc )
        return;

    send_to_pager( color_str(AType, ch), ch );
    ch->desc->pagecolor = ch->colors[AType];
}


/* source: EOD, by John Booth <???> */
void ch_printf(CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];	/* better safe than sorry */
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_char(buf, ch);
}

void paint(sh_int AType, CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];	/* better safe than sorry */
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    set_char_color(AType, ch);
    send_to_char(buf, ch);
    set_char_color(AType, ch);
}


void pager_printf(CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_pager(buf, ch);
}

/*  From Erwin  */
void log_printf_plus(sh_int log_type, sh_int level, sh_int severity, char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(log_buf, fmt, args);
    va_end(args);

    log_string_plus(log_buf,log_type,level,severity);
}

/*
 * Function to strip off the "a" or "an" or "the" or "some" from an object's
 * short description for the purpose of using it in a sentence sent to
 * the owner of the object.  (Ie: an object with the short description
 * "a long dark blade" would return "long dark blade" for use in a sentence
 * like "Your long dark blade".  The object name isn't always appropriate
 * since it contains keywords that may not look proper.		-Thoric
 */
char *myobj( OBJ_DATA *obj )
{
    if ( !str_prefix("a ", obj->short_descr) )
        return obj->short_descr + 2;
    if ( !str_prefix("an ", obj->short_descr) )
        return obj->short_descr + 3;
    if ( !str_prefix("the ", obj->short_descr) )
        return obj->short_descr + 4;
    if ( !str_prefix("some ", obj->short_descr) )
        return obj->short_descr + 5;
    return obj->short_descr;
}

char *obj_short( OBJ_DATA *obj )
{
    static char buf[MAX_STRING_LENGTH];

    if ( obj->count > 1 )
    {
        if ( obj->christened )
            sprintf( buf, "%s (%d)", get_christen_name(obj), obj->count );
        else
            sprintf( buf, "%s (%d)", obj->short_descr, obj->count );
        return buf;
    }
    if ( obj->christened )
        return get_christen_name(obj);
    else
        return obj->short_descr;
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */
#define NAME(ch)	(IS_NPC(ch) ? ch->short_descr : ch->name)
char *act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,
                 const void *arg1, const void *arg2)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
    static char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    char *point = buf;
    const char *str = format;
    const char *i;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;

    while ( *str != '\0' )
    {
        if ( *str != '$' )
        {
            *point++ = *str++;
            continue;
        }
        ++str;
        if ( !arg2 && *str >= 'A' && *str <= 'Z' )
        {
            bug( "Act: missing arg2 for code %c: %s", *str, format );
            sprintf(fname, "$%c", *str);
            i = fname;
        }
        else
        {
            switch ( *str )
            {
            default:/*  bug( "Act: bad code %c.", *str );*/
                sprintf(fname, "$%c", *str);
                i = fname;					break;
                /*		i = " <@@@> ";					break;*/
            case '$': i = "$";					break;
            case 'c': i = "$c";					break;
            case 'C': i = "$c";					break;
            case 't': if ( !arg1 )
            {
                bug("act_string: $t with NULL arg1!");
                i = "$t";
            }
            else
                i = (char *) arg1;
            break;
            case 'T': if ( !arg2 )
            {
                bug("act_string: $T with NULL arg2!");
                i = "$t";
            }
            else
                i = (char *) arg2;
            break;
            case 'n': i = (to ? PERS(ch, to) : NAME( ch));	break;
            case 'N': i = (to ? PERS(vch, to) : NAME(vch));	break;
            case 'e': if (ch->sex > 2 || ch->sex < 0)
            {
                bug("act_string: player %s has sex set at %d!", ch->name,
                    ch->sex);
                i = "it";
            }
            else
                i = he_she [URANGE(0,  ch->sex, 2)];
            break;
            case 'E': if (vch->sex > 2 || vch->sex < 0)
            {
                bug("act_string: player %s has sex set at %d!", vch->name,
                    vch->sex);
                i = "it";
            }
            else
                i = he_she [URANGE(0, vch->sex, 2)];
            break;
            case 'm': if (ch->sex > 2 || ch->sex < 0)
            {
                bug("act_string: player %s has sex set at %d!", ch->name,
                    ch->sex);
                i = "it";
            }
            else
                i = him_her[URANGE(0,  ch->sex, 2)];
            break;
            case 'M': if (vch->sex > 2 || vch->sex < 0)
            {
                bug("act_string: player %s has sex set at %d!", vch->name,
                    vch->sex);
                i = "it";
            }
            else
                i = him_her[URANGE(0, vch->sex, 2)];
            break;
            case 's': if (ch->sex > 2 || ch->sex < 0)
            {
                bug("act_string: player %s has sex set at %d!", ch->name,
                    ch->sex);
                i = "its";
            }
            else
                i = his_her[URANGE(0,  ch->sex, 2)];
            break;
            case 'S': if (vch->sex > 2 || vch->sex < 0)
            {
                bug("act_string: player %s has sex set at %d!", vch->name,
                    vch->sex);
                i = "its";
            }
            else
                i = his_her[URANGE(0, vch->sex, 2)];
            break;
            case 'q': i = (to == ch) ? "" : "s";				break;
            case 'Q': i = (to == ch) ? "your" :
                his_her[URANGE(0,  ch->sex, 2)];			break;
            case 'p': if ( !obj1 )
            {
                bug("act_string: $p without object." );
                i = "something";
            }
            else
                i = (!to || can_see_obj(to, obj1)
                     ? obj_short(obj1) : "something");
            break;
            case 'P': if ( !obj2 )
            {
                bug("act_string: $P without object." );
                i = "something";
            }
            else
                i = (!to || can_see_obj(to, obj2)
                     ? obj_short(obj2) : "something");
            break;
            case 'd':
                if ( !arg2 || ((char *) arg2)[0] == '\0' )
                    i = "door";
                else
                {
                    one_argument((char *) arg2, fname);
                    i = fname;
                }
                break;
            }
        }
        ++str;
        while ( (*point = *i) != '\0' )
            ++point, ++i;
    }
    strcpy(point, "\n\r");
    buf[0] = UPPER(buf[0]);
    return buf;
}
#undef NAME

void act( sh_int AType, const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
    char *txt, *buf;
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *)arg2;
    CHAR_DATA *third = (CHAR_DATA *)arg1;

    /*
     * Discard null and zero-length messages.
     */
    if ( !format || format[0] == '\0' )
        return;

    if ( !ch )
    {
        bug( "Act: null ch. (%s)", format );
        return;
    }

    if ( char_died(ch) )
    {
        bug( "Act: dead ch. (%s)", format );
        return;
    }

    if ( !ch->in_room )
        to = NULL;
    else if ( type == TO_CHAR )
        to = ch;
    else if ( type == TO_VICT )
        to = vch;
    else if ( type == TO_THIRD )
        to = third;
    else
        to = ch->in_room->first_person;


    if ( to && char_died(to) )
    {
        bug( "Act: dead to. (%s)", format );
        return;
    }

    /*
     * ACT_SECRETIVE handling
     */
    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_SECRETIVE) && type != TO_CHAR )
        return;

    if ( type == TO_VICT )
    {
        if ( !vch )
        {
            bug( "Act: null vch with TO_VICT." );
            bug( "%s (%s)", ch->name, format );
            return;
        }
        if ( !vch->in_room )
        {
            bug( "Act: vch in NULL room!" );
            bug( "%s -> %s (%s)", ch->name, vch->name, format );
            return;
        }
        to = vch;
        /*	to = vch->in_room->first_person;*/
    }

    if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
    {
        OBJ_DATA *to_obj;

        txt = act_string(format, NULL, ch, arg1, arg2);
        if ( HAS_PROG(to->in_room, ACT_PROG) )
            rprog_act_trigger(ParseAnsiColors(0, txt), to->in_room, ch, (OBJ_DATA *)arg1, (void *)arg2);
        for ( to_obj = to->in_room->first_content; to_obj;
              to_obj = to_obj->next_content )
            if ( HAS_PROG(to_obj->pIndexData, ACT_PROG) )
                oprog_act_trigger(ParseAnsiColors(0, txt), to_obj, ch, (OBJ_DATA *)arg1, (void *)arg2);
    }

    /* Anyone feel like telling me the point of looping through the whole
     room when we're only sending to one char anyways..? -- Alty */
    for ( ; to; to = (type == TO_CHAR || type == TO_VICT || type == TO_THIRD)
          ? NULL : to->next_in_room )
    {
        if ( ( !to->desc &&
               IS_NPC(to) &&
               !HAS_PROG(to->pIndexData, ACT_PROG) ) ||
/*             !IS_AWAKE(to) )*/
             GET_POS(to) == POS_SLEEPING )
            continue;

        if ( type == TO_CHAR && to != ch )
            continue;
        if ( type == TO_THIRD && to != third )
            continue;
        if ( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if ( type == TO_ROOM && to == ch )
            continue;
        if ( type == TO_NOTVICT && (to == ch || to == vch) )
            continue;

        txt = act_string(format, to, ch, arg1, arg2);
        if ( IS_WEB(ch) )
            send_to_char(txt,ch);
        if (to->desc)
        {
            set_char_color(to->colors[AType], to);
            buf = str_dup(ParseAnsiColors(IS_SET(to->act,PLR_ANSI), txt));
            /*	  write_to_buffer( to->desc, buf, strlen(buf) );*/
            send_to_char_color(buf,to);
            DISPOSE(buf);
            set_char_color(to->colors[AType], to);
        }
        /* Note: use original string, not string with ANSI. -- Alty */
        if (MOBtrigger)
        {
            mprog_act_trigger( ParseAnsiColors(0, txt), to, ch, (OBJ_DATA *)arg1, (void *)arg2 );
        }
    }
    MOBtrigger = TRUE;
    return;
}

void do_name( CHAR_DATA *ch, char *argument )
{
    char fname[1024];
    struct stat fst;
    CHAR_DATA *tmp;

    if ( !NOT_AUTHED(ch) || ch->pcdata->auth_state != 2)
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    argument[0] = UPPER(argument[0]);

    if (!check_parse_name(argument))
    {
        send_to_char( "Illegal name, follow these guidelines:\n\r"
                      "1) No real words.\n\r"
                      "2) No obscene words.\n\r"
                      "3) No compound words.\n\r"
                      "4) Alpha characters only.\n\r"
                      "Name: ",ch );
        return;
    }

    if (!str_cmp(ch->name, argument))
    {
        send_to_char("That's already your name!\n\r", ch);
        return;
    }

    for ( tmp = first_char; tmp; tmp = tmp->next )
    {
        if (!str_cmp(argument, tmp->name))
            break;
    }

    if ( tmp )
    {
        send_to_char("That name is already taken.  Please choose another.\n\r", ch);
        return;
    }

    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(argument[0]),
             capitalize( argument ) );
    if ( stat( fname, &fst ) != -1 )
    {
        send_to_char("That name is already taken.  Please choose another.\n\r", ch);
        return;
    }

    STRFREE( ch->name );
    ch->name = STRALLOC( argument );
    send_to_char("Your name has been changed.  Please apply again.\n\r", ch);
    ch->pcdata->auth_state = 0;
    return;
}

char *default_prompt( CHAR_DATA *ch )
{
    static char buf[60];

    strcpy(buf, "%hhp ");
    if ( IS_VAMPIRE(ch) )
        strcat(buf, "%bbp");
    else
        strcat(buf, "%mm");
    strcat(buf, " %vmv> ");
    if (IS_NPC(ch) || IS_IMMORTAL(ch) )
        strcat(buf, "%i%R");
    return buf;
}

int getcolor(char clr)
{
    static const char colors[17] = "xrgObpcwzRGYBPCW";
    int r;

    for ( r = 0; r < 16; r++ )
        if ( clr == colors[r] )
            return r;
    return -1;
}

char *showcond( CHAR_DATA *ch)
{
    static char buf[MAX_STRING_LENGTH];
    int percent;

    if ( GET_MAX_HIT(ch) > 0 )
        percent = ( 100 * GET_HIT(ch) ) / GET_MAX_HIT(ch);
    else
        percent = -1;

    if ( percent >= 100 )      strcpy( buf, "perfect health"     );
    else if ( percent >=  90 ) strcpy( buf, "slightly scratched" );
    else if ( percent >=  80 ) strcpy( buf, "few bruises"        );
    else if ( percent >=  70 ) strcpy( buf, "some cuts"          );
    else if ( percent >=  60 ) strcpy( buf, "several wounds"     );
    else if ( percent >=  50 ) strcpy( buf, "nasty wounds"       );
    else if ( percent >=  40 ) strcpy( buf, "bleeding freely"    );
    else if ( percent >=  30 ) strcpy( buf, "covered in blood"   );
    else if ( percent >=  20 ) strcpy( buf, "leaking guts"       );
    else if ( percent >=  10 ) strcpy( buf, "almost dead"        );
    else                       strcpy( buf, "DYING"              );

    return(buf);
}

void display_prompt( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch = d->character;
    CHAR_DATA *och = (d->original ? d->original : d->character);
    bool ansi = (!IS_NPC(och) && IS_SET(och->act, PLR_ANSI));
    const char *prompt;
    char buf[MAX_STRING_LENGTH];
    char *pbuf = buf;
    int pstat;
    int i, least;
    bool flag = FALSE;

    if ( !ch )
    {
        bug( "display_prompt: NULL ch" );
        return;
    }

    if ( !IS_NPC(ch) && ch->substate != SUB_NONE && ch->pcdata->subprompt
         &&   ch->pcdata->subprompt[0] != '\0' )
        prompt = ch->pcdata->subprompt;
    else
        if ( IS_NPC(ch) || !ch->pcdata->prompt || !*ch->pcdata->prompt )
            prompt = default_prompt(ch);
        else
            prompt = ch->pcdata->prompt;

    if ( ansi )
    {
        strcpy(pbuf, "\033[m");
        d->prevcolor = 0x07;
        pbuf += 3;
    }
    /* Clear out old color stuff */
    /*  make_color_sequence(NULL, NULL, NULL);*/
    for ( ; *prompt; prompt++ )
    {
        /*
         * '&' = foreground color/intensity bit
         * '^' = background color/blink bit
         * '%' = prompt commands
         * Note: foreground changes will revert background to 0 (black)
         */
        if ( *prompt != '&' && *prompt != '^' && *prompt != '%' )
        {
            *(pbuf++) = *prompt;
            continue;
        }
        ++prompt;
        if ( !*prompt )
            break;
        if ( *prompt == *(prompt-1) )
        {
            *(pbuf++) = *prompt;
            continue;
        }
        switch(*(prompt-1))
        {
        default:
            bug( "Display_prompt: bad command char '%c'.", *(prompt-1) );
            break;
        case '&':
        case '^':
            pstat = make_color_sequence(&prompt[-1], pbuf, d);
            if ( pstat < 0 )
                --prompt;
            else if ( pstat > 0 )
                pbuf += pstat;
            break;
        case '%':
            *pbuf = '\0';
            pstat = 0x80000000;
            switch(*prompt)
            {
            case '%':
                *pbuf++ = '%';
                *pbuf = '\0';
                break;
            case '*':
                strcpy(pbuf, "\033");
                break;
            case 'a':
                if ( IS_GOOD(ch) )
                    strcpy(pbuf, "good");
                else if ( IS_EVIL(ch) )
                    strcpy(pbuf, "evil");
                else
                    strcpy(pbuf, "neutral");
                break;
            case 'f':
                if ( ch->pcdata->deity )
                    pstat = ch->pcdata->favor;
                else
                    strcpy(pbuf, "no favor");
                break;
            case 'h':
#ifdef MXP
                if (MXP_ON(ch))
                    sprintf(pbuf, MXP_TAG_HP"%d"MXP_TAG_HP_CLOSE, GET_HIT(ch));
                else
#endif
                    pstat = GET_HIT(ch);
                break;
            case 'H':
#ifdef MXP
                if (MXP_ON(ch))
                    sprintf(pbuf, MXP_TAG_MAXHP"%d"MXP_TAG_MAXHP_CLOSE, GET_MAX_HIT(ch));
                else
#endif
                    pstat = GET_MAX_HIT(ch);
                break;
            case 'm':
#ifdef MXP
                if (MXP_ON(ch))
                    sprintf(pbuf, MXP_TAG_MANA"%d"MXP_TAG_MANA_CLOSE, GET_MANA(ch));
                else
#endif
                    pstat = GET_MANA(ch);
                break;
            case 'M':
#ifdef MXP
                if (MXP_ON(ch))
                    sprintf(pbuf, MXP_TAG_MAXMANA"%d"MXP_TAG_MAXMANA_CLOSE, GET_MAX_MANA(ch));
                else
#endif
                    pstat = GET_MAX_MANA(ch);
                break;
            case 'b':
                if ( IS_VAMPIRE(ch) )
                    pstat = GET_COND(ch, COND_BLOODTHIRST);
                else
                    pstat = 0;
                break;
            case 'B':
                if ( IS_VAMPIRE(ch) )
                    pstat = GET_MAX_BLOOD(ch);
                else
                    pstat = 0;
                break;
            case 'u':
                pstat = num_descriptors;
                break;
            case 'U':
                pstat = sysdata.maxplayers;
                break;
            case 'v':
                pstat = GET_MOVE(ch);
                break;
            case 'V':
                pstat = GET_MAX_MOVE(ch);
                break;
            case 'g':
                pstat = GET_MONEY(ch,DEFAULT_CURR);
                break;
            case 'r':
                if ( IS_IMMORTAL(och) )
                    pstat = ch->in_room->vnum;
                break;
            case 'R':
                if ( IS_SET(och->act, PLR_ROOMVNUM) )
                    sprintf(pbuf, "#%d ", ch->in_room->vnum);
                break;
            case 'x':
                pstat = GET_EXP(ch);
                break;
            case 'X':
                least = exp_level(ch, GET_LEVEL(ch, FirstActive(ch))+1,
                                  FirstActive(ch)) - ch->exp;
                for (i = FirstActive(ch) + 1; i < MAX_CLASS; ++i)
                    if (IS_ACTIVE(ch, i) && !HAD_CLASS(ch, i))
                        least = UMIN(least, exp_level(ch, GET_LEVEL(ch, i)+1, i) - ch->exp);
                pstat = least;
                break;
            case 'y':			/* Multiclass XP to level. -- Torin (22-JULY-1999) */
		flag = FALSE;
                for (i=0; i < MAX_CLASS; i++) {
                    if (IS_ACTIVE(ch, i) && !HAD_CLASS(ch, i)) {
			if (flag) {	/* keeps us from printing a leading '/' */
			    sprintf(pbuf, "/%ld",exp_level(ch, GET_LEVEL(ch, i)+1, i) - GET_EXP(ch));
			} else
			    sprintf(pbuf, "%ld", exp_level(ch, GET_LEVEL(ch, i)+1, i) - GET_EXP(ch));
			flag = TRUE;
			pbuf += strlen(pbuf);
		    }
		}
		break;
            case 'i':
                if ( (!IS_NPC(ch) && ch->pcdata->wizinvis) ||
                     (IS_NPC(ch) && IS_SET(ch->act, ACT_MOBINVIS)) )
                    sprintf(pbuf, "(Invis %d) ", (IS_NPC(ch) ? ch->mobinvis : ch->pcdata->wizinvis));
                else
                    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
                        sprintf(pbuf, "(Invis) " );
                break;
            case 'I':
                pstat = (IS_NPC(ch) ? (IS_SET(ch->act, ACT_MOBINVIS) ? ch->mobinvis : 0)
                        : ch->pcdata->wizinvis);
                break;
            case 'c':
                if (who_fighting(ch))
                {
                    if (who_fighting(who_fighting(ch)))
                    {
                        sprintf(pbuf, "%s", showcond(who_fighting(who_fighting(ch))));
                    }
                }
                else sprintf(pbuf, "*");
                break;
            case 'C':
                if (who_fighting(ch))
                {
                    sprintf(pbuf, "%s", showcond(who_fighting(ch)));
                }
                else sprintf(pbuf, "*");
                break;
            case 'T':	/* Show the hour - Torin */
                sprintf(pbuf, "%d %s",
			(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
			 time_info.hour >= 12 ? "pm" : "am");
                break;
            case 'n':                /* support for newline by Torin */
                sprintf(pbuf, "\n");
                break;
            case 's':
            case 'S':
                if (IS_AFFECTED(ch, AFF_FIRESHIELD))
                    *pbuf++ = 'F';
                else if (*prompt == 'S')
                    *pbuf++ = '-';
                if (IS_AFFECTED(ch, AFF_SANCTUARY))
                    *pbuf++ = 'S';
                else if (*prompt == 'S')
                    *pbuf++ = '-';
                if (IS_AFFECTED(ch, AFF_INVISIBLE))
                    *pbuf++ = 'I';
                else if (*prompt == 'S')
                    *pbuf++ = '-';
                if (IS_AFFECTED(ch, AFF_TRUESIGHT))
                    *pbuf++ = 'T';
                else if (*prompt == 'S')
                    *pbuf++ = '-';
                if (is_affected(ch, gsn_protection_from_energy_drain))
                    *pbuf++ = 'D';
                else if (*prompt == 'S')
                    *pbuf++ = '-';
                if (is_affected(ch, gsn_anti_magic_shell))
                    *pbuf++ = 'A';
                else if (*prompt == 'S')
                    *pbuf++ = '-';
                *pbuf = '\0';
                break;
            }
            if ( pstat != 0x80000000 )
                sprintf(pbuf, "%d", pstat);
            pbuf += strlen(pbuf);
            break;
        }
    }
    *pbuf = '\0';
#ifdef MXP
    if (MXP_ON(ch))
        write_to_buffer(d, MXP_TAG_PROMPT, 0);
#endif
    write_to_buffer(d, buf, (pbuf-buf));
#ifdef MXP
    if (MXP_ON(ch))
        write_to_buffer(d, MXP_TAG_PROMPT_CLOSE, 0);
#endif
    return;
}

int make_color_sequence(const char *col, char *buf, DESCRIPTOR_DATA *d)
{
    int ln = 0;
    const char *ctype = col;
    unsigned char cl;
    CHAR_DATA *och;
    bool ansi;

    och = (d->original ? d->original : d->character);
    ansi = (!IS_NPC(och) && IS_SET(och->act, PLR_ANSI));
    buf[0] = '\0';
    col++;
    if ( !*col )
        ln = -1;
    else if ( *ctype != '&' && *ctype != '^' )
    {
        bug("Make_color_sequence: command '%c' not '&' or '^'.", *ctype);
        ln = -1;
    }
    else if ( *col == *ctype )
    {
        buf[0] = *col;
        buf[1] = '\0';
        ln = 1;
    }
    else if ( !ansi )
        ln = 0;
    else
    {
        cl = d->prevcolor;
        switch(*ctype)
        {
        default:
            bug( "Make_color_sequence: bad command char '%c'.", *ctype );
            ln = -1;
            break;
        case '&':
            if ( *col == '-' )
            {
                buf[0] = '~';
                buf[1] = '\0';
                ln = 1;
                break;
            }
        case '^':
            {
                int newcol;

                if ( (newcol = getcolor(*col)) < 0 )
                {
                    buf[0] = *ctype;
                    buf[1] = *col;
                    buf[2] = '\0';
                    ln = 2;
                    break;
                }
#if 0
            }
#else
                else if ( *ctype == '&' )
                    cl = (cl & 0xF0) | newcol;
                else
                    cl = (cl & 0x0F) | (newcol << 4);
            }
            if ( cl == d->prevcolor )
            {
                ln = 0;
                break;
            }
            strcat(buf, "\033[0;");
/*            if ( (cl & 0x88) != (d->prevcolor & 0x88) )*/
            {
                if ( (cl & 0x08) )
                    strcat(buf, "1;"); /* light colors */
                else
                    strcat(buf, "0;"); /* dark colors */
                if ( (cl & 0x80) )
                    strcat(buf, "5;"); /* blinking */
                d->prevcolor = 0x07 | (cl & 0x88);
            }
/*            if ( (cl & 0x07) != (d->prevcolor & 0x07) )*/
            {
                sprintf(buf+strlen(buf), "3%d", cl & 0x07); /* normal colors */
            }
            if ( (cl & 0x70) != (d->prevcolor & 0x70) )
            {
                sprintf(buf+strlen(buf), ";4%d", (cl & 0x70) >> 4); /* background colors */
            }
            strcat(buf,"m");
            ln = strlen(buf);
            d->prevcolor = cl;
#endif
        }
    }
    if ( ln <= 0 )
        *buf = '\0';
    return ln;
}

void set_pager_input( DESCRIPTOR_DATA *d, char *argument )
{
    while ( isspace(*argument) )
        argument++;
    d->pagecmd = *argument;
    return;
}

bool pager_output( DESCRIPTOR_DATA *d )
{
    register char *last;
    CHAR_DATA *ch;
    int pclines;
    register int lines;
    bool ret;

    if ( !d || !d->pagepoint || d->pagecmd == -1 )
        return TRUE;
    ch = d->original ? d->original : d->character;
    pclines = UMAX(ch->pcdata->pagerlen, 5) - 1;
    switch(LOWER(d->pagecmd))
    {
    default:
        lines = 0;
        break;
    case 'b':
        lines = -1-(pclines*2);
        break;
    case 'l':
        d->pagepoint = d->pagebuf+strlen(d->pagebuf);
    case 'r':
        lines = -1-pclines;
        break;
    case 'q':
        d->pagetop = 0;
        d->pagepoint = NULL;
        flush_buffer(d, TRUE);
        DISPOSE(d->pagebuf);
        d->pagesize = MAX_STRING_LENGTH;
        return TRUE;
    }
    while ( lines < 0 && d->pagepoint >= d->pagebuf )
        if ( *(--d->pagepoint) == '\n' )
            ++lines;
    if ( *d->pagepoint == '\n' && *(++d->pagepoint) == '\r' )
        ++d->pagepoint;
    if ( d->pagepoint < d->pagebuf )
        d->pagepoint = d->pagebuf;
    for ( lines = 0, last = d->pagepoint; lines < pclines; ++last )
        if ( !*last )
            break;
        else if ( *last == '\n' )
            ++lines;
    if ( *last == '\r' )
        ++last;
    if ( last != d->pagepoint )
    {
        if ( !write_to_descriptor(d, d->pagepoint, (last-d->pagepoint)) )
            return FALSE;

        if ( d->snoop_by )
        {
            /* without check, 'force mortal quit' while snooped caused crash, -h */
            if ( d->character && d->character->name )
            {
                char snoopbuf[MAX_INPUT_LENGTH];

                /* Show original snooped names. -- Altrag */
                if ( d->original && d->original->name )
                    sprintf( snoopbuf, "%s (%s)", d->character->name, d->original->name );
                else
                    sprintf( snoopbuf, "%s", d->character->name);
                write_to_buffer( d->snoop_by, snoopbuf, 0);
            }
            write_to_buffer( d->snoop_by, "% ", 2 );
            write_to_buffer( d->snoop_by, d->pagepoint, (last-d->pagepoint) );
        }

        d->pagepoint = last;
    }
    while ( isspace(*last) )
        ++last;
    if ( !*last )
    {
        d->pagetop = 0;
        d->pagepoint = NULL;
        flush_buffer(d, TRUE);
        DISPOSE(d->pagebuf);
        d->pagesize = MAX_STRING_LENGTH;
        return TRUE;
    }
    d->pagecmd = -1;
    if ( IS_SET( ch->act, PLR_ANSI ) )
        if ( write_to_descriptor(d, "\033[1;36m", 7) == FALSE )
            return FALSE;
    if ( (ret=write_to_descriptor(d, "(C)ontinue, (R)efresh, (B)ack, (L)ast, (Q)uit: [C] ", 0)) == FALSE )
        return FALSE;
    if ( IS_SET(ch->act, PLR_TELNET_GA) )
        write_to_descriptor( d, (char *)go_ahead_str, 0 );
    if ( IS_SET( ch->act, PLR_ANSI ) )
    {
        char buf[32];

        if ( d->pagecolor == 7 )
            strcpy( buf, "\033[m" );
        else
            sprintf(buf, "\033[0;%d;%s%dm", (d->pagecolor & 8) == 8,
                    (d->pagecolor > 15 ? "5;" : ""), (d->pagecolor & 7)+30);
        ret = write_to_descriptor( d, buf, 0 );
    }
    return ret;
}

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */

void do_copyover (CHAR_DATA *ch, char * argument)
{
    FILE *fp;
    DESCRIPTOR_DATA *d;
    char buf[16], buf2[16], buf3[16], buf4[16], buf5[16];

    if (get_char_world(ch, "psitrain"))
    {
        send_to_char("Not while people are psitraining...\n\r", ch);
        return;
    }

    fp = fopen (COPYOVER_FILE, "w");

    if (!fp)
    {
        send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
        log_printf_plus(LOG_NORMAL, LEVEL_LOG_CSET, SEV_ERR, "Could not write to copyover file: %s", COPYOVER_FILE);
        perror ("do_copyover:fopen");
        return;
    }

    /* Consider changing all saved areas here, if you use OLC */

    do_asave (NULL, ""); /* autosave changed areas */
    save_stats();
    if ( sysdata.longest_uptime < (current_time - boot_time) )
        sysdata.longest_uptime = current_time - boot_time;
    save_sysdata( sysdata );

#ifdef IMC
    imc_shutdown( FALSE );
#endif

#ifdef IRC
    irc_shutdown();
#endif

#ifdef USE_BOA
    boa_shutdown();
#endif

#if defined(USE_DB) || defined(START_DB)
    db_stop();
#endif

    if (ch && ch->desc)
        write_to_descriptor(ch->desc,"\033[0;0;37m",0);
    sprintf (buf, "\n\r%s causes the world to shift around you!\n\r", ch?ch->name:"General Failure");
    /* For each playing descriptor, save its state */
    for (d = first_descriptor; d ; d = d_next)
    {
        CHAR_DATA * och = CH(d);
        d_next = d->next; /* We delete from the list , so need to save this */
        if (!d->character || d->connected < 0) /* drop those logging on */
        {
            write_to_descriptor (d, "\n\rSorry, we are rebooting."
                                 " Come back in a few minutes.\n\r", 0);
            close_socket (d, FALSE); /* throw'em out */
        }
        else
        {
            fprintf (fp, "%d %d %d %d %d %d %s %s\n",
#ifndef MUD_LISTENER
                     d->descriptor,
#else
                     d->uid,
#endif
#if !defined(MUD_LISTENER) && defined(COMPRESS)
                     d->compressing,
#else
                     0,
#endif
                     och->in_room->vnum, d->port, d->idle, d->conn_id,
                     och->name, d->host);
	    if (GetMaxLevel(och)<2)
	    {
		log_printf_plus(LOG_NORMAL, LEVEL_LOG_CSET, SEV_NOTICE, "%s given level 2 for copyover.", GET_NAME(och));
		GET_LEVEL(och, GetMaxClass(och)) = 2;
	    }
            save_char_obj(och);

#ifdef USE_CRBS
            do_clogoff(och, NULL); /* CRBS logoff */
#endif

            write_to_descriptor (d, buf, 0);
#ifndef MUD_LISTENER
#ifdef COMPRESS
            if (d->compressing)
                stop_compression(d, d->compressing);
#endif
#endif
        }
    }
    fprintf (fp, "-1\n");
    FCLOSE (fp);

#ifdef I3
    if (I3_is_connected())
    {
        I3_savechanlist();
        I3_savemudlist();
    }
#endif

    /* exec - descriptors are inherited */

#ifndef MUD_LISTENER
    snprintf(buf, 16, "%d", port);
    snprintf(buf2, 16, "%d", control);
    snprintf(buf3, 16, "%d", controls);
#ifdef I3
    snprintf(buf4, 16, "%d", I3_socket);
#else
    strcpy(buf4, "-1");
#endif
#ifdef IRC
    snprintf(buf5, 16, "%d", irc_socket);
#else
    strcpy(buf5, "-1");
#endif

    execl(EXE_FILE, "dotd", buf, "-", buf2, buf3, buf4, buf5, (char *) NULL);
#else
    snprintf(buf, 16, "%d", controls);

    execl(EXE_FILE, "dotd", "0", "-", buf, (char *) NULL);
#endif


    /* Failed - sucessful exec will not return */

    perror ("do_copyover: execl");
    send_to_char ("Copyover FAILED!\n\r",ch);
}

/* Recover from a copyover - load players */
void copyover_recover (void)
{
    DESCRIPTOR_DATA *d = NULL;
    FILE *fp;
    char name [100];
    char host[MAX_STRING_LENGTH];
#ifndef MUD_LISTENER
    int desc, compress, room, lport, idle, cid, fdret;
#else
    int uid, room, lport, idle, cid, fdret;
#endif
    bool fOld;

    log_string_plus("Copyover recovery initiated", LOG_NORMAL, LEVEL_LOG_CSET, SEV_CRIT);

    fp = fopen (COPYOVER_FILE, "r");

    if (!fp) /* there are some descriptors open which will hang forever then ? */
    {
        perror ("copyover_recover:fopen");
        log_string_plus("Copyover file not found. Exitting.\n\r", LOG_NORMAL, LEVEL_LOG_CSET, SEV_EMERG);
        exit (1);
    }

    unlink (COPYOVER_FILE); /* In case something crashes
    - doesn't prevent reading */
    for (;;)
    {
        d = NULL;

#ifndef MUD_LISTENER
        fdret = fscanf (fp, "%d %d %d %d %d %d %s %s\n", &desc, &compress, &room, &lport, &idle, &cid, name, host);
        if (desc == -1 || fdret != 8 || feof(fp))
            break;
#else
        fdret = fscanf (fp, "%d %d %d %d %d %s %s\n", &uid, &room, &lport, &idle, &cid, name, host);
        if (uid == -1 || fdret != 7 || feof(fp))
            break;
#endif

        CREATE(d, DESCRIPTOR_DATA, 1);
#ifndef MUD_LISTENER
        init_descriptor (d, desc); /* set up various stuff */

        /* Write something, and check if it goes error-free */
        if (!compress && !write_to_descriptor (d, "\n\rThe world starts to fall back into place.\n\r", 0))
        {
            free_desc(d);
            continue;
        }
#ifdef COMPRESS
        d->compressing = compress;
        if (d->compressing)
            start_compression(d, d->compressing);
#endif
#else
        init_descriptor (d, uid); /* set up various stuff */
#endif
        d->host = STRALLOC( host );
        d->port = lport;
        d->idle = idle;
        d->conn_id = cid;

        LINK( d, first_descriptor, last_descriptor, next, prev );
        d->connected = CON_COPYOVER_RECOVER; /* negative so close_socket
        will cut them off */

        /* Now, find the pfile */

        fOld = load_char_obj (d, name, FALSE, NULL, TRUE);

        if (!fOld) /* Player file not found?! */
        {
            write_to_descriptor (d, "\n\rSomehow, your character was lost in the copyover sorry.\n\r", 0);
            close_socket (d, FALSE);
        }
        else /* ok! */
        {
            write_to_descriptor (d, "\n\rThe world is finished shifting.\n\r",0);

            /* Just In Case,  Someone said this isn't necassary, but _why_
	     do we want to dump someone in limbo? */
	    load_area_demand(room);

            d->character->in_room = get_room_index (room);
            if (!d->character->in_room)
                d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

            /* Insert in the char_list */
            add_char(d->character);

            char_to_room (d->character, d->character->in_room);
            do_look (d->character, "auto noprog");
            act (AT_ACTION, "$n materializes!", d->character, NULL, NULL, TO_ROOM);
            d->connected = CON_PLAYING;
            num_descriptors++;
#ifdef I3
            I3_char_login(d->character);
#endif

#ifdef IRC
    irc_logon(d->character);
#endif

#ifdef USE_CRBS
            do_clogon(d->character, NULL); /* CRBS login */
#endif
        }

    }
    FCLOSE (fp);
}

/* converts time_t (seconds) into a x hours, x minutes, x seconds string
 -Garil */
char *sec_to_hms(time_t time_val)
{
    time_t t_time;
    static char buf[MAX_INPUT_LENGTH];
    char tstr[MAX_INPUT_LENGTH];
    int i=0;

    if (time_val==0)
    {
        sprintf(buf, "no time at all");
        return(buf);
    }

    t_time=time_val;
    buf[0]='\0';

    i=0;
    while (t_time-(60*60*24*365)>=0) {
        i++;
        t_time-=(60*60*24*365);
    }
    if (i>0) {
        sprintf(tstr,"%d %s ",i,i==1?"year":"years");
        strcat(buf,tstr);
    }
    i=0;
    while (t_time-(60*60*24)>=0) {
        i++;
        t_time-=(60*60*24);
    }
    if (i>0) {
        sprintf(tstr,"%d %s ",i,i==1?"day":"days");
        strcat(buf,tstr);
    }
    i=0;
    while (t_time-(60*60)>=0) {
        i++;
        t_time-=(60*60);
    }
    if (i>0) {
        sprintf(tstr,"%d %s ",i,i==1?"hour":"hours");
        strcat(buf,tstr);
    }
    i=0;
    while (t_time-(60)>=0) {
        i++;
        t_time-=(60);
    }
    if (i>0) {
        sprintf(tstr,"%d %s ",i,i==1?"minute":"minutes");
        strcat(buf,tstr);
    }
    i=t_time;
    if (i>0) {
        sprintf(tstr,"%d %s",i,i==1?"second":"seconds");
        strcat(buf,tstr);
    }

    return(buf);
}

/* converts time_t (seconds) into a hh:mm:ss string
 -Garil */
char *sec_to_hms_short(time_t time_val)
{
    time_t t_time;
    static char buf[MAX_INPUT_LENGTH];
    char tstr[MAX_INPUT_LENGTH];
    int i=0;

    t_time=time_val;
    buf[0]='\0';

    i=0;
    while (t_time-(60*60*24*365)>=0) {
        i++;
        t_time-=(60*60*24*365);
    }
    if (i>0) {
        sprintf(tstr,"%d:",i); /* years */
        strcat(buf,tstr);
    }
    i=0;
    while (t_time-(60*60*24)>=0) {
        i++;
        t_time-=(60*60*24);
    }
    if (i>0) {
        sprintf(tstr,"%d:",i); /* days */
        strcat(buf,tstr);
    }
    i=0;
    while (t_time-(60*60)>=0) {
        i++;
        t_time-=(60*60);
    }
    sprintf(tstr,"%2.2d:",i); /* hours */
    strcat(buf,tstr);

    i=0;
    while (t_time-(60)>=0) {
        i++;
        t_time-=(60);
    }
    sprintf(tstr,"%2.2d:",i); /* minutes */
    strcat(buf,tstr);

    i=t_time;
    sprintf(tstr,"%2.2d",i); /* seconds */
    strcat(buf,tstr);

    return(buf);
}

void open_mud_log()
{
    FILE *error_log;
    struct tm *now_time;
    char buf[MAX_INPUT_LENGTH];

    now_time = localtime(&current_time);

    sprintf(buf, LOG_DIR "%d-%d.log",
            now_time->tm_mon+1, now_time->tm_mday);

    if (!(error_log=fopen(buf,"a")))
    {
        fprintf(stderr,"Unable to append to %s.",buf);
        exit(1);
    }

    if (!verbose_log)
        dup2(fileno(error_log),STDERR_FILENO);
    FCLOSE(error_log);
}

void last_log(char *name, DESCRIPTOR_DATA *d)
{
    char str[MAX_INPUT_LENGTH];

    snprintf(str, sizeof(str), "%.24s %-12s%-11.11s@%s",
             ctime(&current_time),
             name,
             d->user,
             d->host);

    append_to_file(LASTLOG_FILE, str);
}

#ifndef MUD_LISTENER
#ifdef COMPRESS
void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
    return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
    DISPOSE(address);
}

bool process_compressed(DESCRIPTOR_DATA *d)
{
    int iStart = 0, nBlock, nWrite, len;

    if (!d->out_compress)
        return TRUE;

    /* Try to write out some data..*/
    len = d->out_compress->next_out - d->out_compress_buf;

    if (len > 0)
    {
        /* we have some data to write */
        for (iStart = 0; iStart < len; iStart += nWrite)
        {
            nBlock = UMIN (len - iStart, 4096);
            if ((nWrite = write(d->descriptor, d->out_compress_buf + iStart, nBlock)) < 0)
            {
                if (errno == EAGAIN
#ifdef ENOSR
                    || errno == ENOSR
#endif
                   )
                    break;

                return FALSE;
            }

            if (!nWrite)
                break;

            stats.boot_bytes_out      += nWrite;
            stats.bytes_out           += nWrite;
            stats.comp_boot_bytes_out += nWrite;
            stats.comp_bytes_out      += nWrite;
        }

        if (iStart)
        {
            /* We wrote "iStart" bytes */
            if (iStart < len)
                memmove(d->out_compress_buf, d->out_compress_buf+iStart, len - iStart);

            d->out_compress->next_out = d->out_compress_buf + len - iStart;
        }
    }

    return TRUE;
}

bool start_compression(DESCRIPTOR_DATA *d, unsigned char telopt)
{
    z_stream *s;

    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d supports MCCP-%d, enabling", d->descriptor, telopt);

    if (d->out_compress)
        return TRUE;

    CREATE(s, z_stream, 1);
    CREATE(d->out_compress_buf, unsigned char, COMPRESS_BUF_SIZE);

    s->next_in = NULL;
    s->avail_in = 0;

    s->next_out = d->out_compress_buf;
    s->avail_out = COMPRESS_BUF_SIZE;

    s->zalloc = zlib_alloc;
    s->zfree  = zlib_free;
    s->opaque = NULL;

    if (deflateInit(s, 9) != Z_OK)
    {
        DISPOSE(d->out_compress_buf);
        DISPOSE(s);
        return FALSE;
    }

    if (telopt == TELOPT_COMPRESS)
        write_to_descriptor(d, enable_compress, 0);
    else if (telopt == TELOPT_COMPRESS2)
        write_to_descriptor(d, enable_compress2, 0);
    else
        bug("compressStart: bad TELOPT passed");

    d->compressing = telopt;
    d->out_compress = s;

    return TRUE;
}

bool stop_compression(DESCRIPTOR_DATA *d, unsigned char telopt)
{
    unsigned char dummy[1];

    log_printf_plus(LOG_COMM, LEVEL_IMMORTAL, SEV_DEBUG, "Descriptor %d, stopping MCCP-%d", d->descriptor, telopt);

    if (!d->out_compress)
        return TRUE;

    d->out_compress->avail_in = 0;
    d->out_compress->next_in = dummy;

    /* No terminating signature is needed - receiver will get Z_STREAM_END */

    if (deflate(d->out_compress, Z_FINISH) != Z_STREAM_END)
        return FALSE;

    if (!process_compressed(d)) /* try to send any residual data */
        return FALSE;

    deflateEnd(d->out_compress);
    DISPOSE(d->out_compress_buf);
    DISPOSE(d->out_compress);

    d->compressing = FALSE;

    return TRUE;
}

#endif
#endif

void do_stats(CHAR_DATA *ch, char *argument)
{
    if (!str_cmp(argument, "reset by garil"))
    {
        stats.boot_bytes_in = 0;
        stats.boot_bytes_out = 0;
        stats.comp_boot_bytes_in = 0;
        stats.comp_boot_bytes_out = 0;
        stats.bytes_in = 0;
        stats.bytes_out = 0;
        stats.comp_bytes_in = 0;
        stats.comp_bytes_out = 0;
    }

    send_to_char("Since last reboot:\n\r", ch);

    ch_printf(ch, "Bytes in:               %ld\n\r", stats.boot_bytes_in);
    ch_printf(ch, "Compressed bytes in:    %ld\n\r", stats.comp_boot_bytes_in);
    ch_printf(ch, "Compression percentage: %.2f%%\n\r",
              100.0-(100.0*((float)stats.comp_boot_bytes_in/(float)stats.boot_bytes_in)));
    ch_printf(ch, "Bytes out:              %ld\n\r", stats.boot_bytes_out);
    ch_printf(ch, "Compressed bytes out:   %ld\n\r", stats.comp_boot_bytes_out);
    ch_printf(ch, "Compression percentage: %.2f%%\n\r",
              100.0-(100.0*((float)stats.comp_boot_bytes_out/(float)stats.boot_bytes_out)));

    send_to_char("\n\rTotals:\n\r", ch);

    ch_printf(ch, "Bytes in:               %ld\n\r", stats.bytes_in);
    ch_printf(ch, "Compressed bytes in:    %ld\n\r", stats.comp_bytes_in);
    ch_printf(ch, "Compression percentage: %.2f%%\n\r",
              100.0-(100.0*((float)stats.comp_bytes_in/(float)stats.bytes_in)));
    ch_printf(ch, "Bytes out:              %ld\n\r", stats.bytes_out);
    ch_printf(ch, "Compressed bytes out:   %ld\n\r", stats.comp_bytes_out);
    ch_printf(ch, "Compression percentage: %.2f%%\n\r",
              100.0-(100.0*((float)stats.comp_bytes_out/(float)stats.bytes_out)));

    save_stats();
}
