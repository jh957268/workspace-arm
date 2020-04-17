#ifndef _CSQLITE_H_
#define _CSQLITE_H_

#include <stdio.h>
#include "sqlite3.h"

class CSqlite
{
    public:
		    CSqlite(char *db);
			~CSqlite() {sqlite3_close(rfid_db)}
			
			void begin_transaction(void);
			void commit(void);
			int  insert_tag(unsigned char *tag, int antid, double rssi);

    private:
			int callback(void *NotUsed, int argc, char **argv, char **azColName);
			
			sqlite3 *rfid_db;
			int select_tag_id;
			char sql_buff[128];
			char *sql;
			char *zErrMsg
};


#endif // _CSQLITE_H_
