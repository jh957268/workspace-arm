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
	} 
	else
	{
	    fprintf(stderr, "Opened database successfully\n");
	}
	
	sql = "CREATE TABLE if not exists TAG_DATA" \
	    "(" \
	        "tag_id integer PRIMARY KEY," \
	        "tag_val text(32) NOT NULL," \
	        "DATE_TIME_STAMP DATETIME default current_timestamp," \
	        "tag_antID 		integer," \
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
	    fprintf(stdout, "TAG_DATA Table created successfully\n");
	}

	sql = "CREATE TABLE if not exists TAG_USER_DATA" \
	    "(" \
	        "tag_id integer PRIMARY KEY," \
	        "tag_val text(32) NOT NULL," \
	        "DATE_TIME_STAMP DATETIME default current_timestamp," \
	        "tag_antID 		integer," \
	    	"tag_action	    interger," \ 
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
	    fprintf(stdout, "ASSET_DATA Table created successfully\n");
	}
}

int
CSqlite::callback(void *NotUsed, int argc, char **argv, char **azColName) 
{
   int i;
   int *parm = (int *)NotUsed;
   
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   if (NotUsed)
   {
	   //*(int *)NotUsed = atoi(argv[0]);
		parm[0] = atoi(argv[0]);
		parm[1] = atoi(argv[1]);   
   }
   printf("\n");
   return 0;
}

void
CSqlite::begin_transaction()
{
	sqlite3_exec(rfid_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}

void
CSqlite::commit()
{
	sqlite3_exec(rfid_db, "COMMIT;", NULL, NULL, NULL);
}

void
CSqlite::db_close()
{
	sqlite3_close(rfid_db);
}

int
CSqlite::select_tag(char *tag, t_func sq_callback, void *param, int table)
{
	int rc = SQLITE_OK;
	char *pTable;
	
	if (table == 0)
	{
		pTable = "TAG_DATA";
	}
	else
	{
		pTable = "TAG_USER_DATA";
	}
	

	if (!strcmp(tag, "all"))
	{
		sprintf(sql_buff, "select * from %s;", pTable);
		rc = sqlite3_exec(rfid_db, sql_buff, sq_callback, param, &zErrMsg);

	}
	else
	{
		// tag can be LIMIT 10 OFFSET 10;
		// sql = "select * from TAG_DATA limit xx offset yy;"
		sprintf(sql_buff, "select * from %s %s;", pTable, tag);
		rc = sqlite3_exec(rfid_db, sql_buff, sq_callback, param, &zErrMsg);
	}
	if( rc != SQLITE_OK ){
	    fprintf(stderr, "SQL error: %s\n", zErrMsg);
	    sqlite3_free(zErrMsg);
	} else {
	    fprintf(stdout, "Records selected successfully\n");
	}
}

int
CSqlite::insert_tag(char *tag, int antid, double rssi)
{
	int rc;
	
	sprintf(sql_buff, "select tag_id,tag_seen_count from TAG_DATA where tag_val='%s' AND tag_antID=%d;", tag, antid);
	parm[0] = parm[1] = 0;
	rc = sqlite3_exec(rfid_db, sql_buff, callback, (void*)parm, &zErrMsg);

	if( rc != SQLITE_OK ){
	    fprintf(stderr, "SQL error: %s\n", zErrMsg);
	    sqlite3_free(zErrMsg);
	} else {
	    fprintf(stdout, "Record seleted successfully\n");
	}
	if (parm[0] != 0)	
	{
		int seen_cnt = parm[1] + 1;
		sprintf(sql_buff, "update TAG_DATA SET tag_RSSI=%f,tag_seen_count=%d,tag_last_seen=datetime('now') where tag_id=%d;", rssi, seen_cnt,parm[0] );
	    rc = sqlite3_exec(rfid_db, sql_buff, callback, 0, &zErrMsg);

	    if( rc != SQLITE_OK ){
	       fprintf(stderr, "SQL error: %s\n", zErrMsg);
	       sqlite3_free(zErrMsg);
	    } else {
	       fprintf(stdout, "Record updated successfully\n");
	    }
		return 0;
	}

	sprintf(sql_buff,"insert into TAG_DATA (tag_val, tag_antID, tag_RSSI, tag_first_seen, tag_last_seen, tag_seen_count, tag_out_Of_fov)" \
			 "VALUES" \
	         "('%s', %d, %f, datetime('now'), datetime('now'), 1, 1);",tag, antid, rssi); 
			 
	rc = sqlite3_exec(rfid_db, sql_buff, callback, 0, &zErrMsg);

	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
	    sqlite3_free(zErrMsg);
	} else {
	    fprintf(stdout, "Records inserted successfully\n");
	}
	return 0;
}

int
CSqlite::insert_user_tag(char *tag, int antid, int action)
{
	int rc;
	
	sprintf(sql_buff, "select tag_id from TAG_USER_DATA where tag_val='%s' AND tag_antID=%d;", tag, antid);
	parm[0] = parm[1] = 0;
	rc = sqlite3_exec(rfid_db, sql_buff, callback, (void*)parm, &zErrMsg);

	if( rc != SQLITE_OK ){
	    fprintf(stderr, "SQL error: %s\n", zErrMsg);
	    sqlite3_free(zErrMsg);
	} else {
	    fprintf(stdout, "Record seleted successfully\n");
	}
	if (parm[0] != 0)	
	{
		sprintf(sql_buff, "update TAG_USER_DATA SET tag_action=%d where tag_id=%d;", action, parm[0]);
		rc = sqlite3_exec(rfid_db, sql_buff, callback, 0, &zErrMsg);

	    if( rc != SQLITE_OK ){
	       fprintf(stderr, "SQL error: %s\n", zErrMsg);
	       sqlite3_free(zErrMsg);
	    } else {
	       fprintf(stdout, "Record updated successfully\n");
	    }
		return rc;
	}
	
	sprintf(sql_buff,"insert into TAG_USER_DATA (tag_val, tag_antID, tag_action, tag_create_time)" \
			 "VALUES" \
	         "('%s', %d, %d, datetime('now'));",tag, antid, action); 
			 
	rc = sqlite3_exec(rfid_db, sql_buff, callback, 0, &zErrMsg);

	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
	    sqlite3_free(zErrMsg);
	} else {
	    fprintf(stdout, "Records inserted successfully\n");
	}
	return (rc);
}

int
CSqlite::delete_user_tag(char *tag, int antid)
{
	int rc;
	
	sprintf(sql_buff, "select from TAG_USER_DATA where tag_val='%s' AND tag_antID=%d;", tag, antid);
	parm[0] = parm[1] = 0;
	rc = sqlite3_exec(rfid_db, sql_buff, callback, 0, &zErrMsg);

	if( rc != SQLITE_OK ){
	    fprintf(stderr, "SQL error: %s\n", zErrMsg);
	    sqlite3_free(zErrMsg);
	} else {
	    fprintf(stdout, "Record seleted successfully\n");
	}
	return 0;
}

