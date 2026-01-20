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
// Created by WangYunlai on 2021/6/9.
//

#include "sql/operator/insert_physical_operator.h"
#include "sql/stmt/insert_stmt.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

using namespace std;

InsertPhysicalOperator::InsertPhysicalOperator(Table *table, vector<RawTuple> &&tuples)
    : table_(table), tuples_(std::move(tuples))
{}

RC InsertPhysicalOperator::open(Trx *trx)
{
  // Record record;
  // RC     rc = table_->make_record(static_cast<int>(values_.size()), values_.data(), record);
  // if (rc != RC::SUCCESS) {
  //   LOG_WARN("failed to make record. rc=%s", strrc(rc));
  //   return rc;
  // }

  // rc = trx->insert_record(table_, record);
  // if (rc != RC::SUCCESS) {
  //   LOG_WARN("failed to insert record by transaction. rc=%s", strrc(rc));
  // }
  // return rc;
  vector<Record> records; // 记录已插入的记录，方便出错时回滚
  for (const auto &tuple : tuples_) {
    Record record;
    RC     rc = table_->make_record(static_cast<int>(tuple.size()), tuple.data(), record);
    // 没有成功生成record，回滚之前插入的记录
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to make record. rc=%s", strrc(rc));
      for(auto &rec : records) {
        RC del_rc = trx->delete_record(table_, rec);
        if (del_rc != RC::SUCCESS) {
          LOG_ERROR("failed to rollback inserted record. rc=%s", strrc(del_rc));
        }
      }
      return rc;
    }
    rc = trx->insert_record(table_, record);
    // 没插入成功，回滚之前插入的记录
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert record by transaction. rc=%s", strrc(rc));
      for(auto &rec : records) {
        RC del_rc = trx->delete_record(table_, rec);
        if (del_rc != RC::SUCCESS) {
          LOG_ERROR("failed to rollback inserted record. rc=%s", strrc(del_rc));
        }
      }
      return rc;
    }
    records.emplace_back(record);
  }
  return RC::SUCCESS;
}

RC InsertPhysicalOperator::next() { return RC::RECORD_EOF; }

RC InsertPhysicalOperator::close() { return RC::SUCCESS; }
