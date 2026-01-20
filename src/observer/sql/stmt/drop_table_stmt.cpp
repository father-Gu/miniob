#include "sql/stmt/drop_table_stmt.h"
#include "storage/db/db.h"
#include "event/sql_debug.h"

RC DropTableStmt::create(Db *db, const DropTableSqlNode &drop_table, Stmt *&stmt)
{
    // 判断待删除表名是否为空
    if(drop_table.relation_name.empty()) {
        LOG_WARN("invalid drop table sql node. table name is empty");
        return RC::INVALID_ARGUMENT;
    }
    // 判断数据库指针是否为空
    if(db == nullptr) {
        LOG_WARN("invalid argument. db is nullptr");
        return RC::INVALID_ARGUMENT;
    }
    // 检查表是否存在
    Table *table = db->find_table(drop_table.relation_name.c_str());
    if (nullptr == table) {
        LOG_WARN("no such table. db=%s, table_name=%s", db->name(), drop_table.relation_name.c_str());
        return RC::SCHEMA_TABLE_NOT_EXIST;
    }
    // 创建 DropTableStmt 对象
    stmt = new DropTableStmt(drop_table.relation_name);
    if (stmt == nullptr) {
        LOG_ERROR("failed to create DropTableStmt");
        return RC::NOMEM;
    }
    sql_debug("DropTableStmt created for table: %s", drop_table.relation_name.c_str());
    return RC::SUCCESS;
}