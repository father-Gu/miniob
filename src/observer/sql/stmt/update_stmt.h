/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#pragma once

#include "common/sys/rc.h"
#include "sql/stmt/stmt.h"
#include "sql/parser/parse_defs.h"
#include "storage/table/table_meta.h"

class Table;

/**
 * @brief 更新语句
 * @ingroup Statement
 */
class UpdateStmt : public Stmt
{
public:
  UpdateStmt() = default;
  UpdateStmt(Table *table, const FieldMeta *field_meta, Value value, const std::vector<ConditionSqlNode> &conditions);
  ~UpdateStmt() override = default;

public:
  static RC create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt);

public:
  Table                               *table() const { return table_; }
  const FieldMeta                     *field_meta() const { return field_meta_; }
  const Value                         &value() const { return value_; }
  const std::vector<ConditionSqlNode> &conditions() const { return conditions_; }

  StmtType type() const override { return StmtType::UPDATE; }

private:
  Table                        *table_      = nullptr;
  const FieldMeta              *field_meta_ = nullptr;
  Value                         value_;
  std::vector<ConditionSqlNode> conditions_;
};
