#include "sql/stmt/join_stmt.h"

JoinStmt::~JoinStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}
RC JoinStmt::create(Db *db, Table *default_table, unordered_map<string, Table *> *table_map, JoinSqlNode &join_sql, JoinStmt *&joinstmt)
{
    if (nullptr == db) {
        LOG_WARN("invalid argument. db is null");
        return RC::INVALID_ARGUMENT;
    }
    // 检查连接条件
    if (join_sql.conditions.empty()) {
        LOG_WARN("join conditions is empty");
        return RC::INVALID_ARGUMENT;
    }
    // 创建连接条件的过滤语句(当前join语句对应的on里的所有条件都包含在filter_stmt中了)
    FilterStmt *filter_stmt = nullptr;
    RC rc = FilterStmt::create(db, default_table, table_map, join_sql.conditions.data(),
                                static_cast<int>(join_sql.conditions.size()), filter_stmt);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create filter stmt for join conditions. rc=%s", strrc(rc));
        return rc;
    }
    // everything alright
    JoinStmt *join_stmt = new JoinStmt();
    join_stmt->filter_stmt_ = filter_stmt;
    joinstmt = join_stmt;
    
    return RC::SUCCESS;
    //return RC::INVALID_ARGUMENT;
}