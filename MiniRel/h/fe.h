#ifndef __FE_H__
#define __FE_H__

/****************************************************************************
 * fe.h: External interface for the FE (UT and QU) layers
 ****************************************************************************/

/*
 * maximum length of an identifier (relation or attribute name)
 */
#define MAXNAME		12

/*
 * maximum length of a string
 */
#ifdef DEPRECATED
#define MAXSTRINGLEN	255
#endif

/*
 * ATTR_DESCR: attribute descriptor used in CreateTable
 */
typedef struct{
    char *attrName;	/* relation name	*/
    int attrType;	/* type of attribute	*/
    int attrLen;	/* length of attribute	*/
} ATTR_DESCR;

/*
 * REL_ATTR: describes a qualified attribute (relName.attrName)
 */
typedef struct{
    char *relName;	/* relation name	*/
    char *attrName;	/* attribute name	*/
} REL_ATTR;

/*
 * ATTR_VAL: <attribute, value> pair
 */
typedef struct{
    char *attrName;	/* attribute name	*/
    int valType;	/* type of value	*/
    int valLength;	/* length if type = STRING_TYPE */
    char *value;	/* value for attribute	*/
} ATTR_VAL;


/*
 * Prototypes for FE layer functions
 * They start with UT because FE layer functions are 
 * divided in utilities (UT) and query (QU) functions.       
 */

int  CreateTable(const char *relName,	/* name	of relation to create	*/
		int numAttrs,		/* number of attributes		*/
		ATTR_DESCR attrs[],	/* attribute descriptors	*/
		const char *primAttrName);/* primary index attribute	*/

int  DestroyTable(const char *relName);	/* name of relation to destroy	*/

int  BuildIndex(const char *relName,	/* relation name		*/
		const char *attrName);	/* name of attr to be indexed	*/

int  DropIndex(const char *relname,	/* relation name		*/
		const char *attrName);	/* name of indexed attribute	*/

int  PrintTable(const char *relName);	/* name of relation to print	*/

int  LoadTable(const char *relName,	/* name of target relation	*/
		const char *fileName);	/* file containing tuples	*/

int  HelpTable(const char *relName);	/* name of relation		*/

/*
 * Prototypes for QU layer functions
 */

int  Select(const char *srcRelName,	/* source relation name         */
		const char *selAttr,	/* name of selected attribute   */
		int op,			/* comparison operator          */
		int valType,		/* type of comparison value     */
		int valLength,		/* length if type = STRING_TYPE */
		char *value,		/* comparison value             */
		int numProjAttrs,	/* number of attrs to print     */
		char *projAttrs[],	/* names of attrs of print      */
		char *resRelName);       /* result relation name         */

int  Join(REL_ATTR *joinAttr1,		/* join attribute #1            */
		int op,			/* comparison operator          */
		REL_ATTR *joinAttr2,	/* join attribute #2            */
		int numProjAttrs,	/* number of attrs to print     */
		REL_ATTR projAttrs[],	/* names of attrs to print      */
		char *resRelName);	/* result relation name         */

int  Insert(const char *relName,	/* target relation name         */
		int numAttrs,		/* number of attribute values   */
		ATTR_VAL values[]);	/* attribute values             */

int  Delete(const char *relName,	/* target relation name         */
		const char *selAttr,	/* name of selection attribute  */
		int op,			/* comparison operator          */
		int valType,		/* type of comparison value     */
		int valLength,		/* length if type = STRING_TYPE */
		char *value);		/* comparison value             */


void FE_PrintError(const char *errmsg);	/* error message		*/
void FE_Init(void);			/* FE initialization		*/



/*
 * FE layer error codes
 */

#define FE_NERRORS              33

#define FEE_OK			0
#define FEE_ALREADYINDEXED	(-1)
#define FEE_ATTRNAMETOOLONG	(-2)
#define FEE_DUPLATTR		(-3)
#define FEE_INCOMPATJOINTYPES	(-4)
#define FEE_INCORRECTNATTRS	(-5)
#define FEE_INTERNAL		(-6)
#define FEE_INVATTRTYPE		(-7)
#define FEE_INVNBUCKETS		(-8)		/* Ok not to use */
#define FEE_NOTFROMJOINREL	(-9)
#define FEE_NOSUCHATTR		(-10)
#define FEE_NOSUCHDB		(-11)		/* Ok not to use */
#define FEE_NOSUCHREL		(-12)
#define FEE_NOTINDEXED		(-13)
#define FEE_PARTIAL		(-14)		/* Ok not to use */
#define FEE_PRIMARYINDEX	(-15)
#define FEE_RELCAT		(-16)		/* Ok not to use */
#define FEE_RELEXISTS		(-17)
#define FEE_RELNAMETOOLONG	(-18)
#define FEE_SAMEJOINEDREL	(-19)
#define FEE_SELINTOSRC		(-20)
#define FEE_THISATTRTWICE	(-21)
#define FEE_UNIONCOMPAT		(-22)		/* Ok not to use */
#define FEE_WRONGVALTYPE	(-23)
#define FEE_RELNOTSAME          (-24)
#define FEE_NOMEM               (-25)
#define FEE_EOF                 (-26)
#define FEE_CATALOGCHANGE       (-27)
#define FEE_STRTOOLONG          (-28)
#define FEE_SCANTABLEFULL       (-29)
#define FEE_INVALIDSCAN         (-30)
#define FEE_INVALIDSCANDESC     (-31)
#define FEE_INVALIDOP           (-32)

/*************/
#define FEE_UNIX		(-100)
#define FEE_HF			(-101)
#define FEE_AM                  (-102)
#define FEE_PF                  (-103)		/* Ok not to use */


#define INT_SIZE  4
#define REAL_SIZE 4

/*
 * global variables for the catalogs. Must be defined in the FE layer.
 */
extern int relcatFd, attrcatFd;

/*
 * global FE layer error value
 */
extern int FEerrno;

#endif

