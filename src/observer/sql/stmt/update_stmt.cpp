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

#include "sql/stmt/update_stmt.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

UpdateStmt::UpdateStmt(
    Table *table, const FieldMeta *field_meta, Value value, const std::vector<ConditionSqlNode> &conditions)
    : table_(table), field_meta_(field_meta), value_(value), conditions_(conditions)
{}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt)
{
  // 1. 检查表是否存在
  const char *table_name = update_sql.relation_name.c_str();
  if (nullptr == table_name || common::is_blank(table_name)) {
    LOG_WARN("invalid argument. table name is null.");
    return RC::INVALID_ARGUMENT;
  }

  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // 2. 检查字段是否存在
  const char      *field_name = update_sql.attribute_name.c_str();
  const FieldMeta *field_meta = table->table_meta().field(field_name);
  if (nullptr == field_meta) {
    LOG_WARN("no such field. field=%s.%s.%s", db->name(), table_name, field_name);
    return RC::SCHEMA_FIELD_MISSING;
  }

  // 3. 检查值类型（支持类型转换）
  const Value &value = update_sql.value;
  Value        real_value;

  if (field_meta->type() != value.attr_type()) {
    RC rc = Value::cast_to(value, field_meta->type(), real_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to cast value type. field=%s, field_type=%d, value_type=%d",
               field_meta->name(), field_meta->type(), value.attr_type());
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
  } else {
    real_value = value;
  }

  // 4. 创建 UpdateStmt（直接保存 conditions）
  stmt = new UpdateStmt(table, field_meta, real_value, update_sql.conditions);
  return RC::SUCCESS;
}