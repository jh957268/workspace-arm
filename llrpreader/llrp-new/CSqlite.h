#ifndef _CSQLITE_H_
#define _CSQLITE_H_

#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

typedef int (*t_func)(void *NotUsed, int argc, char **argv, char **azColName);

class CSqlite
{
    public:
		    CSqlite(char *db);
			~CSqlite() {sqlite3_close(rfid_db);}
			
			void begin_transaction(void);
			void commit(void);
			int  insert_tag(char *tag, int antid, double rssi);
			int  insert_user_tag(char *tag, int antid);
			int  delete_user_tag(char *tag, int antid);			
			int  select_tag(char *tag, t_func sqcallback, void *param);
			void db_close();
			static int callback(void *NotUsed, int argc, char **argv, char **azColName);
    private:

			sqlite3 *rfid_db;
			int parm[2];
			char sql_buff[256];
			char *sql;
			char *zErrMsg;
};


#endif // _CSQLITE_H_
