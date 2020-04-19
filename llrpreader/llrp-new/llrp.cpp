// llrp.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
//#include <afxwin.h>
#include <iostream>
#include <string>
 #include <sstream>
#include "CWatchDogFeeder.h"
#include "iReaderapi.h"
#include "llrp_MntServer.h"
#include "iAgent_executor.h"
#include "debug_print.h"
#include "muxserial.h"
#include "gpio.h"
#include "CSqlite.h"

using namespace std;

class base {
  public:
    base()
    { cout<<"Constructing base \n"; }
    ~base()
    { cout<<"Destructing base \n"; }
};

class derived: public base {
  public:
    derived()
    { cout<<"Constructing derived \n"; }
    ~derived()
    { cout<<"Destructing derived \n"; }
};

static void printBanner(void);
extern void process_cli_command(string cmd_string);
unsigned char test_char;

Muxserial *Ser1;
Muxserial *Ser2;

//sqlite3 *rfid_db;
//int select_tag_id;
//char sql_buff[128];
CSqlite *Sqlite_db;

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   if (NotUsed)
   {
	   *(int *)NotUsed = atoi(argv[0]);
   }
   printf("\n");
   return 0;
}

int main(int argc, char* argv[])
{
	string input_string = "";
	//WSADATA wsaData;
	char *zErrMsg = 0;
	int rc;

#if 0
	UINT8 datalen[2];
	UINT16 dataLen;

	datalen[0] = 0x12;
	datalen[1] = 0x34;

	dataLen = (datalen[0] << 8) | datalen[1];

	printf("dataLen = %d"NL, dataLen);
#endif

	printBanner();

	 derived *d = new derived();
	 base *b = d;
	 delete d;

	//IReaderApiInitialize();
#if 0
	if (WSAStartup(0x202,&wsaData) != 0) {
		WSACleanup();
		cout << "Bummer, cannot start networking function...Bailout!!!";
		return 1;
	}
#endif

#if 0
	string word = "12384";

	string newWord;
	
	newWord.assign(word, 1, word.size());

	cout << newWord << endl;

	if (std::string::npos != newWord.find_first_not_of("0123456789"))
	{
		cout << "contains not digit" << endl;
	}
	else
	{
		cout << "contains all digit" << endl;
	}

	if (word[0] == '1')
		printf ("True"NL);
	else
		printf("False"NL);

	printf ("string size = %d"NL, word.size());

	typedef ios_base& (*Base)(ios_base&);
	Base  base = dec;

	int value;
	istringstream iss(word);
	iss >> *base >> value;
	printf("The value = %d"NL, value);

	if ( ! (istringstream(word) >> std::dec >> value))
		value = 9999;

	printf("The value = %d"NL, value);

	cout << word << endl;

#endif

	Sqlite_db = new CSqlite("rfidtag.db");

#if 0
	rc = sqlite3_open("rfidtag.db", &rfid_db);

	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(rfid_db));
		return(0);
	} else
	{
	    fprintf(stderr, "Opened database successfully\n");
	}
#endif
	// sqlite3_close(rfid_db);

	/* Create SQL statement */

//	char *sql, *sql1;
#if 0
	sql = "CREATE TABLE TAGINFO("  \
	      "TAGVALUE 	CHAR(24)     NOT NULL," \
	      "ANTID         INT     NOT NULL," \
	      "RSSI          DOUBLE," \
	      "FIRSTSEENTIME DATETIME," \
	      "LASTSEENTIME  DATETIME," \
	      "SEENCOUNT     INT," \
		  "PRIMARY KEY (" \
		  "TAGVALUE," \
		  "ANTID));";

CREATE TABLE TAG_DATA 
( 
    tag_id integer PRIMARY KEY, 
    tag_val text(32) NOT NULL, 
    DATE_TIME_STAMP DATETIME default current_timestamp, 
    tag_antID integer,
	tag_RSSI	   DOUBLE,
    tag_first_seen DATETIME, 
    tag_last_seen  DATETIME, 
    tag_seen_count integer, 
    tag_out_Of_fov integer
);

BEGIN TRANSACTION;
insert into TAG_DATA (tag_id, tag_val, tag_antID, tag_RSSI, tag_first_seen, tag_last_seen, tag_seen_count, 	tag_out_Of_fov)
VALUES
    (1, '1234567890abcdef1234567f', 1, -55.0, datetime('now'), datetime('now'), 1, 1);
	
insert into TAG_DATA (tag_val, tag_antID, tag_RSSI, tag_first_seen, tag_last_seen, tag_seen_count, 	tag_out_Of_fov)
VALUES
    ('1234567890abcdef12345678', 1, -55.0, datetime('now'), datetime('now'), 1, 1),
	('1234567890abcdef1234567a', 2, -56.0, datetime('now'), datetime('now'), 1, 1),
	('1234567890abcdef1234567b', 3, -58.0, datetime('now'), datetime('now'), 1, 1);	
	
select tag_id from TAG_DATA where tag_val='1234567890abcdef1234567a' AND tag_antID=2;

update TAG_DATA SET tag_seen_count=5 where tag_id=4;

select tag_id,tag_seen_count from TAG_DATA where tag_val='1234567890abcdef1234567a' AND tag_antID=2;

COMMIT;

BEGIN TRANSACTION;
 
UPDATE accounts
   SET balance = balance - 1000
 WHERE account_no = 100;
 
UPDATE accounts
   SET balance = balance + 1000
 WHERE account_no = 200;
 
INSERT INTO account_changes(account_no,flag,amount,changed_at) 
VALUES(100,'-',1000,datetime('now'));
 
INSERT INTO account_changes(account_no,flag,amount,changed_at) 
VALUES(200,'+',1000,datetime('now'));
 
COMMIT;




	
CREATE TABLE prod_mast(
prod_id integer PRIMARY KEY,
prod_name text(20),
prod_rate integer,
prod_qc text(10) DEFAULT 'OK');	

INSERT INTO prod_mast(prod_id, prod_name, prod_rate, prod_qc)
VALUES(1, 'Pancakes', 75, 'OK');

INSERT INTO prod_mast(prod_name, prod_rate, prod_qc)
VALUES('Gulha', 55, 'Problems');
	
#endif
#if 0
	   sql = "CREATE TABLE COMPANY("  \
	      "ID INT PRIMARY KEY     NOT NULL," \
	      "NAME           TEXT    NOT NULL," \
	      "AGE            INT     NOT NULL," \
	      "ADDRESS        CHAR(50)," \
	      "SALARY         REAL );";

	   /* Execute SQL statement */
	   rc = sqlite3_exec(rfid_db, sql, callback, 0, &zErrMsg);

	   if( rc != SQLITE_OK ){
	      fprintf(stderr, "SQL error: %s\n", zErrMsg);
	      sqlite3_free(zErrMsg);
	   } else {
	      fprintf(stdout, "Table created successfully\n");
	   }

	   /* Create SQL statement */
	    sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
	          "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
	          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
	          "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
	          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
	          "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
	          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
	          "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";

	    /* Execute SQL statement */
	    rc = sqlite3_exec(rfid_db, sql, callback, 0, &zErrMsg);



	    if( rc != SQLITE_OK ){
	       fprintf(stderr, "SQL error: %s\n", zErrMsg);
	       sqlite3_free(zErrMsg);
	    } else {
	       fprintf(stdout, "Records created successfully\n");
	    }
#endif

#if 0
	   sql = "CREATE TABLE TAG_DATA" \
	    "(" \
	        "tag_id integer PRIMARY KEY," \
	        "tag_val text(32) NOT NULL," \
	        "DATE_TIME_STAMP DATETIME default current_timestamp," \
	        "tag_antID integer," \
	    	"tag_RSSI	    DOUBLE," \
	        "tag_first_seen DATETIME," \
	        "tag_last_seen  DATETIME," \
	        "tag_seen_count integer," \
	        "tag_out_Of_fov integer" \
	   ");";

	   /* Execute SQL statement */
	   rc = sqlite3_exec(rfid_db, sql, callback, 0, &zErrMsg);

	   if( rc != SQLITE_OK ){
	      fprintf(stderr, "SQL error: %s\n", zErrMsg);
	      sqlite3_free(zErrMsg);
	   } else {
	      fprintf(stdout, "Table created successfully\n");
	   }

	   sql = "insert into TAG_DATA (tag_id, tag_val, tag_antID, tag_RSSI, tag_first_seen, tag_last_seen, tag_seen_count, tag_out_Of_fov)" \
			 "VALUES" \
	         "(1, '1234567890abcdef1234567f', 1, -55.0, datetime('now'), datetime('now'), 1, 1);";

	   sql1 = "insert into TAG_DATA (tag_val, tag_antID, tag_RSSI, tag_first_seen, tag_last_seen, tag_seen_count, tag_out_Of_fov)" \
	          "VALUES" \
	          "('1234567890abcdef12345678', 1, -55.0, datetime('now'), datetime('now'), 1, 1)," \
	   	      "('1234567890abcdef1234567a', 2, -56.0, datetime('now'), datetime('now'), 1, 1)," \
	   	      "('1234567890abcdef1234567b', 3, -58.0, datetime('now'), datetime('now'), 1, 1);";


	   // 'db' is the pointer you got from sqlite3_open*
	   sqlite3_exec(rfid_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	   // Any (modifying) SQL commands executed here are not committed until at the you call:

	    /* Execute SQL statement */
	    rc = sqlite3_exec(rfid_db, sql, callback, 0, &zErrMsg);

	    if( rc != SQLITE_OK ){
	       fprintf(stderr, "SQL error: %s\n", zErrMsg);
	       sqlite3_free(zErrMsg);
	    } else {
	       fprintf(stdout, "Records created successfully\n");
	    }

	    rc = sqlite3_exec(rfid_db, sql1, callback, 0, &zErrMsg);

	    if( rc != SQLITE_OK ){
	       fprintf(stderr, "SQL error: %s\n", zErrMsg);
	       sqlite3_free(zErrMsg);
	    } else {
	       fprintf(stdout, "Records created successfully\n");
	    }

	    sql = "select tag_id from TAG_DATA where tag_val='1234567890abcdef1234567a' AND tag_antID=2;";

	    rc = sqlite3_exec(rfid_db, sql, callback, &select_tag_id, &zErrMsg);

	    if( rc != SQLITE_OK ){
	       fprintf(stderr, "SQL error: %s\n", zErrMsg);
	       sqlite3_free(zErrMsg);
	    } else {
	       fprintf(stdout, "Records created successfully\n");
	    }

	    sprintf(sql_buff, "update TAG_DATA SET tag_seen_count=5 where tag_id=%d;", select_tag_id);
	    rc = sqlite3_exec(rfid_db, sql_buff, callback, 0, &zErrMsg);

	    if( rc != SQLITE_OK ){
	       fprintf(stderr, "SQL error: %s\n", zErrMsg);
	       sqlite3_free(zErrMsg);
	    } else {
	       fprintf(stdout, "Records created successfully\n");
	    }

	   sqlite3_exec(rfid_db, "COMMIT;", NULL, NULL, NULL);

	   sqlite3_close(rfid_db);

#endif
	   Sqlite_db->begin_transaction();
	   Sqlite_db->insert_tag("1234567890abcdef1234567f", 1, -55.6);
	   Sqlite_db->insert_tag("1234567890abcdef12345678", 1, -58.6);
	   Sqlite_db->insert_tag("1234567890abcdef1234567a", 2, -56.6);
	   Sqlite_db->insert_tag("1234567890abcdef1234567b", 3, -57.6);

	   Sqlite_db->insert_tag("1234567890abcdef1234567a", 2, -46.6);
	   Sqlite_db->commit();

	   Sqlite_db->select_tag("all", CSqlite::callback, 0);
	   //Sqlite_db->db_close();

//	LLRP_MntServer abc;

//	abc = LLRP_MntServer(1); testing calling the contructor

	if (NULL == IReaderApiInit("0.0.0.0", REGION_USA))
	{
		exit(-1);
	}
	GPIO::getInstance(0, 11);
	GPIO::getInstance(1, 74);

	if (LLRP_MntServer::initRegistry() == -1)
	{
		exit(-1);
	}

	// !!! Make sure IREADER object is created first at IReaderApiInit
	LLRP_MntServer *pLLRP_MntServer = LLRP_MntServer::getInstance(0);
	pLLRP_MntServer->run();

	IAgent_Executor *pIAgent_Executor = IAgent_Executor::getInstance();
	pIAgent_Executor->run();

	// int prio = OwTask::getPriority();
	unsigned int pid = pthread_self();    // cannot use unsigned int, pthread_self returns 64-bit value

	int  my_schedpolicy;
	sched_param  my_schedparam;
	pthread_getschedparam( pthread_self() , &my_schedpolicy, &my_schedparam );

	int prio =  my_schedparam.sched_priority;
	DBG_PRINT(DEBUG_INFO, "Main Tread PID = %d,  Priority = %d"NL, pid, prio);

	// CLI command processing loop
	OwTask::sleep(100);

	Ser1 = new Muxserial("/dev/ttyS1");
#if 0
	Ser2 = new Muxserial("/dev/ttyS2");

    // test the serial port
	Ser1->clearRcv();
	if (Ser1->getChar(&test_char) == 1)
	{
		printf("getChar = %02x\n", test_char);
	}
	else
	{
		printf("getChar fails\n");
	}
	Ser1->sendChar('a');

	if (Ser1->getChar(&test_char) == 1)
	{
		printf("getChar = %02x\n", test_char);
	}
	else
	{
		printf("getChar fails\n");
	}
	Ser1->sendChar('b');
	Ser1->getChar(&test_char);
	printf("Rev = %02x\n", test_char);
	Ser1->sendChar('c');
	Ser1->getChar(&test_char);
	printf("Rev = %02x\n", test_char);
	Ser1->sendChar('d');
	Ser1->getChar(&test_char);
	printf("Rev = %02x\n", test_char);

	Ser1->clearRcv();
#endif

	DBG_PRINT(DEBUG_INFO, NL);
	while (1)
	{
		cout << "llrp> ";
		getline(cin, input_string);
		if (input_string.size() != 0)
		{
			process_cli_command(input_string);
		}
	}
	return 0;
}

static void printBanner(void)
{
	cout << "**************************************************************" << endl;
	cout << "*                                                            *" << endl;
	cout << "*                   RFIDspan Technology                      *" << endl;
	cout << "*                   LLRP iReader-998 Device Server           *" << endl;
	cout << "*                   Build Date : " << __DATE__ "                 *"<< endl;
	cout << "*                   Build Time : " << __TIME__ "                    *"<< endl;
	cout << "*                                                            *" << endl;
	cout << "**************************************************************" << endl;
}

