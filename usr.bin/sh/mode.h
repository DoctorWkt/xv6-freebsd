/*
 *	UNIX shell
 */

#define BYTESPERWORD	(sizeof(char *))

typedef char BOOL;
typedef int UFD;
typedef char *STRING;
typedef char MSG[];
typedef int PIPE[];

typedef char *STKPTR;
typedef char *BYTPTR;

typedef struct argnod *ARGPTR;
typedef struct blk *BLKPTR;
typedef struct comnod *COMPTR;
typedef struct dolnod *DOLPTR;
typedef struct fileblk *FILE;
typedef struct fileblk FILEBLK;
typedef struct filehdr FILEHDR;
typedef struct forknod *FORKPTR;
typedef struct fornod *FORPTR;
typedef struct ifnod *IFPTR;
typedef struct ionod *IOPTR;
typedef struct lstnod *LSTPTR;
typedef struct namnod *NAMPTR;
typedef struct namnod NAMNOD;
typedef struct parnod *PARPTR;
typedef struct regnod *REGPTR;
typedef struct stat STATBUF;	/* defined in /usr/sys/stat.h */
typedef struct swnod *SWPTR;
typedef struct sysnod *SYSPTR;
typedef struct sysnod SYSNOD;
typedef struct sysnod SYSTAB;
typedef struct trenod *TREPTR;
typedef struct whnod *WHPTR;

#define NIL	((void *)0)

/* the following nonsense is required
 * because casts turn an Lvalue
 * into an Rvalue so two cheats
 * are necessary, one for each context.
 */
#define Rcheat(a)	((size_t)(a))

/* heap storage */
struct blk {
	BLKPTR	word;
};

/* pathopen return this */
typedef union pth_ret { UFD fd; STRING *str; } pth_ret;

#define	BUFSIZ	128
struct fileblk {
	UFD	fdes;
	unsigned flin;
	BOOL	feof;
	unsigned char fsiz;
	STRING	fnxt;
	STRING	fend;
	STRING	*feval;
	FILE	fstak;

	char	fbuf[BUFSIZ];
};

/* for files not used with file descriptors */
struct filehdr {
	UFD	fdes;
	unsigned flin;
	BOOL	feof;
	unsigned char fsiz;
	STRING	fnxt;
	STRING	fend;
	STRING	*feval;
	FILE	fstak;

	char	_fbuf[1];
};

struct sysnod {
	STRING	sysnam;
	int	sysval;
};

/* this node is a proforma for those that follow */
struct trenod {
	int	tretyp;
	IOPTR	treio;
};

/* dummy for access only */
struct argnod {
	ARGPTR	argnxt;
	char	argval[1];
};

struct dolnod {
	DOLPTR	dolnxt;
	int	doluse;
	char	dolarg[1];
};

struct forknod {
	int	forktyp;
	IOPTR	forkio;
	TREPTR	forktre;
};

struct comnod {
	int	comtyp;
	IOPTR	comio;
	ARGPTR	comarg;
	ARGPTR	comset;
};

struct ifnod {
	int	iftyp;
	TREPTR	iftre;
	TREPTR	thtre;
	TREPTR	eltre;
};

struct whnod {
	int	whtyp;
	TREPTR	whtre;
	TREPTR	dotre;
};

struct fornod {
	int	fortyp;
	TREPTR	fortre;
	STRING	fornam;
	COMPTR	forlst;
};

struct swnod {
	int	swtyp;
	STRING	swarg;
	REGPTR	swlst;
};

struct regnod {
	ARGPTR	regptr;
	TREPTR	regcom;
	REGPTR	regnxt;
};

struct parnod {
	int	partyp;
	TREPTR	partre;
};

struct lstnod {
	int	lsttyp;
	TREPTR	lstlef;
	TREPTR	lstrit;
};

struct ionod {
	int	iofile;
	STRING	ioname;
	IOPTR	ionxt;
	IOPTR	iolst;
};

#define	FORKTYPE	(sizeof(struct forknod))
#define	COMTYPE		(sizeof(struct comnod))
#define	IFTYPE		(sizeof(struct ifnod))
#define	WHTYPE		(sizeof(struct whnod))
#define	FORTYPE		(sizeof(struct fornod))
#define	SWTYPE		(sizeof(struct swnod))
#define	REGTYPE		(sizeof(struct regnod))
#define	PARTYPE		(sizeof(struct parnod))
#define	LSTTYPE		(sizeof(struct lstnod))
#define	IOTYPE		(sizeof(struct ionod))
