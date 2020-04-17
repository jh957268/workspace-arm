#include "CSqlite.h"
#include "stdlib.h"

using namespace std;

CSqlite::CSqlite(char *db)
{
	int rc;
	
	rc = sqlite3_open("rfidtag.db", &rfid_db);

	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(rfid_db));
		return(0);
	} 
	else
	{
	    fprintf(stderr, "Opened database successfully\n");
	}
	
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

	if ( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
	    sqlite3_free(zErrMsg);
	} else {
	    fprintf(stdout, "Table created successfully\n");
	}	   
}

void
CSqlite::begin_transaction(int val)
{
	sqlite3_exec(rfid_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}

void
CSqlite::commit
(
	sqlite3_exec(rfid_db, "COMMIT;", NULL, NULL, NULL);
)


int
CSqlite::insert_tag(unsigned char *tag, int antid, double rssi)
{
	int rc;
	
	sprintf(sql_buff, ""select tag_id from TAG_DATA where tag_val='%s' AND tag_antID=%d;", tag, antid);
	select_tag_id = 0;
	rc = sqlite3_exec(rfid_db, sql, callback, &select_tag_id, &zErrMsg);

	if( rc != SQLITE_OK ){
	    fprintf(stderr, "SQL error: %s\n", zErrMsg);
	    sqlite3_free(zErrMsg);
	} else {
	    fprintf(stdout, "Records created successfully\n");
	}
	if (select_tag_id != 0)
	{
		sprintf(sql_buff, "update TAG_DATA SET tag_seen_count=5 where tag_id=%d;", select_tag_id);
	    rc = sqlite3_exec(rfid_db, sql_buff, callback, 0, &zErrMsg);

	    if( rc != SQLITE_OK ){
	       fprintf(stderr, "SQL error: %s\n", zErrMsg);
	       sqlite3_free(zErrMsg);
	    } else {
	       fprintf(stdout, "Records created successfully\n");
	    }
	}
}

