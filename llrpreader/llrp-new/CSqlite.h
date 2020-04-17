#ifndef _CSQLITE_H_
#define _CSQLITE_H_

#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

class CSqlite
{
    public:
		    CSqlite(char *db);
			~CSqlite() {sqlite3_close(rfid_db);}
			
			void begin_transaction(void);
			void commit(void);
			int  insert_tag(char *tag, int antid, double rssi);
			int  select_tag(char *tag);
			void db_close();

    private:

			static int callback(void *NotUsed, int argc, char **argv, char **azColName);
			sqlite3 *rfid_db;
			int parm[2];
			char sql_buff[256];
			char *sql;
			char *zErrMsg;
};


#endif // _CSQLITE_H_
