#pragma once

#include "common/sys/rc.h"
#include "sql/stmt/stmt.h"
#include "storage/field/field.h"
#include "sql/stmt/filter_stmt.h"
/**
 * @brief 表示join语句
 * @ingroup Statement
 */
class JoinStmt : public Stmt
{
public:
    JoinStmt() = default;
    ~JoinStmt() override;
    StmtType type() const override { return StmtType::JOIN; }
    FilterStmt *filter_stmt() const { return filter_stmt_; }

public:
    static RC create(Db *db, Table *default_table, unordered_map<string, Table *> *table_map, JoinSqlNode &join_sql, JoinStmt *&joinstmt);

private:
    FilterStmt *filter_stmt_ = nullptr;     // join条件的过滤语句
};